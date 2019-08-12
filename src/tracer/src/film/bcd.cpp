#ifdef USE_BCD

#include <cassert>
#include <fstream>
#include <thread>
#include <vector>

#include <agz/tracer/core/film.h>
#include <agz/tracer/utility/logger.h>
#include <agz/utility/thread.h>

#include <bcd/Denoiser.h>
#include <bcd/MultiscaleDenoiser.h>
#include <bcd/SamplesAccumulator.h>
#include <bcd/SpikeRemovalFilter.h>

AGZ_TRACER_BEGIN

namespace bcd_film
{
    
struct Record
{
    Vec2i image_coord;
    Spectrum value;
};

using RecordBatch = std::vector<Record>;

class Recorder : public misc::uncopyable_t
{
    thread::blocking_queue_t<RecordBatch> queue_;
    bcd::SamplesAccumulator accumulator_;
    Vec2i res_;

    std::thread accumulating_thread_;

    static void accumulating_func(Recorder *recorder)
    {
        assert(recorder);
        for(;;)
        {
            auto opt_rcd_batch = recorder->queue_.pop_or_stop();
            if(!opt_rcd_batch)
                break;
            for(auto &rcd : *opt_rcd_batch)
            {
                if(0 <= rcd.image_coord.x && rcd.image_coord.x < recorder->res_.x &&
                   0 <= rcd.image_coord.y && rcd.image_coord.y < recorder->res_.y)
                {
                    recorder->accumulator_.addSample(rcd.image_coord.y, rcd.image_coord.x,
                                                     rcd.value.r, rcd.value.g, rcd.value.b);
                }
            }
        }
    }

public:

    Recorder(const Vec2i &full_res, const bcd::HistogramParameters &histo_params)
        : accumulator_(full_res.x, full_res.y, histo_params), res_(full_res)
    {
        accumulating_thread_ = std::thread(accumulating_func, this);
    }

    ~Recorder()
    {
        if(accumulating_thread_.joinable())
        {
            queue_.stop();
            accumulating_thread_.join();
        }
    }

    void add_batch(RecordBatch &&batch)
    {
        queue_.push(std::move(batch));
    }

    bcd::SamplesStatisticsImages extract_sample_statistics() &&
    {
        queue_.stop();
        assert(accumulating_thread_.joinable());
        accumulating_thread_.join();
        return accumulator_.extractSamplesStatistics();
    }
};

class BCDFilmGrid : public FilmGrid
{
    RecordBatch batch_;

    int grid_x_beg_;
    int grid_x_end_;
    int grid_y_beg_;
    int grid_y_end_;

public:

    friend class BCDFilm;

    BCDFilmGrid(int x_beg, int x_end, int y_beg, int y_end)
        : grid_x_beg_(x_beg), grid_x_end_(x_end), grid_y_beg_(y_beg), grid_y_end_(y_end)
    {
        
    }

    int sample_x_beg() const noexcept override { return grid_x_beg_; }
    int sample_x_end() const noexcept override { return grid_x_end_; }
    int sample_y_beg() const noexcept override { return grid_y_beg_; }
    int sample_y_end() const noexcept override { return grid_y_end_; }

    void add_sample(const Vec2 &pos, const Spectrum &value, const GBufferPixel&, real w) override
    {
        int x = static_cast<int>(std::floor(pos.x));
        int y = static_cast<int>(std::floor(pos.y));
        batch_.push_back({ { x, y }, value * w });
    }
};

class BCDHistogramParams
{
public:

    int n_bins        = 20;
    real bin_size_exp = real(2.2);
    real max_value    = real(2.5);

    void parse_params(const Config &params)
    {
        n_bins       = params.child_int_or("n_histo_bins", n_bins);
        bin_size_exp = params.child_real_or("histo_bin_size_exp", bin_size_exp);
        max_value    = params.child_real_or("histo_bin_max_value", max_value);
    }
};

class BCDDenoiserParams
{
public:

    real histo_patch_distance_threshold    = 1;
    int patch_radius                       = 1;
    int search_window_radius               = 6;

    real min_eigen                         = real(1e-8);

    bool random_pixel_order                = true;
    
    bool prefilter_spikes                  = true;
    real prefilter_threshold_stdev_factor  = 2;
    
    real marked_pixel_skipping_prob        = 1;
    int n_scales                           = 3;

    void parse_params(const Config &params)
    {
        histo_patch_distance_threshold = params.child_real_or(
            "histo_patch_distance_threshold", histo_patch_distance_threshold);
        patch_radius = params.child_int_or("patch_radius", patch_radius);
        search_window_radius = params.child_int_or("search_window_radius", search_window_radius);

        min_eigen = params.child_real_or("min_eigen", min_eigen);

        random_pixel_order = params.child_int_or("random_pixel_order", random_pixel_order) != 0;

        prefilter_spikes = params.child_int_or("prefilter_spikes", prefilter_spikes) != 0;
        prefilter_threshold_stdev_factor = params.child_real_or(
            "prefilter_threshold_stdev_factor", prefilter_threshold_stdev_factor);

        marked_pixel_skipping_prob = params.child_real_or(
            "marked_pixel_skipping_prob", marked_pixel_skipping_prob);
        n_scales = params.child_int_or("n_scales", n_scales);
    }
};

class BCDFilm : public Film
{
    BCDDenoiserParams denoiser_params_;

    Vec2i resolution_;
    std::unique_ptr<Recorder> recorder_;

    // image_ & weight_ are invalid before calling end()

    std::unique_ptr<texture::texture2d_t<Spectrum>> image_;
    real weight_ = 1;

    std::string save_statistics_filename_;
    std::string load_statistics_filename_;

    bool denoise_ = true;

    // 这小段代码来自 https://github.com/superboubek/bcd/blob/master/src/cli/main.cpp
    // 虽然很简单，但既然都用了BCD的主体实现了……
    static void clamp_not_finite_values(bcd::DeepImage<float> &io_rImage, bool i_verbose = false)
    {
        using std::isnan;
        using std::isinf;
        int width = io_rImage.getWidth();
        int height = io_rImage.getHeight();
        int depth = io_rImage.getDepth();
        std::vector<float> values(depth);
        for(int line = 0; line < height; line++)
            for(int col = 0; col < width; col++)
            {
                bool hasBadValue = false;
                for(int z = 0; z < depth; ++z)
                {
                    float val = values[z] = io_rImage.get(line, col, z);
                    if(val < 0 || isnan(val) || isinf(val))
                    {
                        io_rImage.set(line, col, z, 0.f);
                        hasBadValue = true;
                    }
                }
                if(hasBadValue && i_verbose)
                {
                    std::stringstream sst;
                    sst << values[0];
                    for(int i = 1; i < depth; ++i)
                        sst << ", " << values[i];
                    AGZ_LOG0("Warning: strange value for pixel (line,column) = (", line, ", ", col, "): (", sst.str(), ")");
                }
            }
    }

    static bool save_deepimf(std::ofstream &out, const bcd::Deepimf &imf)
    {
        int width = imf.getWidth(), height = imf.getHeight(), depth = imf.getDepth();
        out.write(reinterpret_cast<char*>(&width),  sizeof(width));
        out.write(reinterpret_cast<char*>(&height), sizeof(height));
        out.write(reinterpret_cast<char*>(&depth),  sizeof(depth));
        if(!out)
            return false;

        int scalar_count = imf.getSize();
        size_t byte_size = scalar_count * sizeof(float);
        const float *data = imf.getDataPtr();
        out.write(reinterpret_cast<const char*>(data), byte_size);
        if(!out)
            return false;

        return true;
    }

    static bool load_deepimf(std::istream &in, bcd::Deepimf &imf)
    {
        int width, height, depth;
        in.read(reinterpret_cast<char*>(&width),  sizeof(width));
        in.read(reinterpret_cast<char*>(&height), sizeof(height));
        in.read(reinterpret_cast<char*>(&depth),  sizeof(depth));
        if(!in || width <= 0 || height <= 0 || depth <= 0)
            return false;

        imf = bcd::Deepimf(width, height, depth);
        int scalar_count = width * height * depth;
        size_t byte_size = scalar_count * sizeof(float);
        in.read(reinterpret_cast<char*>(imf.getDataPtr()), byte_size);
        if(!in)
            return false;

        return true;
    }

    static void save_samples_statistics(const std::string &filename, const bcd::SamplesStatisticsImages &statistics)
    {
        std::ofstream fout(filename, std::ofstream::binary | std::ofstream::trunc);
        if(!fout ||
           !save_deepimf(fout, statistics.m_meanImage) ||
           !save_deepimf(fout, statistics.m_histoImage) ||
           !save_deepimf(fout, statistics.m_nbOfSamplesImage) ||
           !save_deepimf(fout, statistics.m_covarImage))
        {
            AGZ_LOG0("failed to save sample statistics to ", filename);
        }
    }

    static bcd::SamplesStatisticsImages load_samples_statistics(const std::string &filename)
    {
        std::ifstream fin(filename, std::ifstream::binary);
        bcd::SamplesStatisticsImages ret;
        if(!fin ||
           !load_deepimf(fin, ret.m_meanImage) ||
           !load_deepimf(fin, ret.m_histoImage) ||
           !load_deepimf(fin, ret.m_nbOfSamplesImage) ||
           !load_deepimf(fin, ret.m_covarImage))
        {
            AGZ_LOG0("failed to load sample statistics from ", filename);
            return bcd::SamplesStatisticsImages();
        }
        return ret;
    }

public:

    using Film::Film;

    static std::string description()
    {
        return R"___(
bcd [Film]
    denoise                          [0/1]  (optional; defaultly set to 1) perform denoising or not
    n_histo_bins                     [int]  (optional; defaultly set to 20)   number of histogram bins
    histo_bin_size_exp               [real] (optional; defaultly set to 2.2)  exponent of increasing histogram bin size
    histo_bin_max_value              [real] (optional; defaultly set to 2.5)  maximum contained value of histogram bin
    histo_patch_distance_threshold   [real] (optional; defaultly set to 1)    similarity threshold of histogram patch distance
    patch_radius                     [int]  (optional; defaultly set to 1)    a patch with radius r contains (2r+1)^2 pixels
    search_window_size               [int]  (optional; defaultly set to 6)    limit the range of neighboring patch
    min_eigen                        [real] (optional; defaultly set to 1e-8) eigen value clamping threshold
    random_pixel_order               [0/1]  (optional; defaultly set to 1)    process unmarked pixels in random order
    prefilter_spikes                 [0/1]  (optional; defaultly set to 1)    execute spike prefiltering before denoising
    prefilter_threshold_stdev_factor [real] (optional; defaultly set to 2)    prefilter parameter
    marked_pixel_skipping_prob       [real] (optional; defaultly set to 1)    prob of skipping marked pixels
    n_scales                         [int]  (optional; defaultly set to 3)    denoising scales
        
    height [int] framebuffer height
    width  [int] framebuffer width

    save_statistics_filename [string] (optional; defaultly set to "") save sample statistics to file
    load_statistics_filename [string] (optional; defaultly set to "") use sample statistics loaded from file instead of merged film grids
        
    gbuffer is unsupported
)___";
    }

    void initialize(const Config &params, obj::ObjectInitContext &init_ctx) override
    {
        AGZ_HIERARCHY_TRY

        denoise_ = params.child_int_or("denoise", 1) != 0;

        denoiser_params_.parse_params(params);
        BCDHistogramParams histo_params;
        histo_params.parse_params(params);

        int width  = params.child_int("width");
        int height = params.child_int("height");
        if(width <= 0 || height <= 0)
            throw ObjectConstructionException("invalid film resolution");
        resolution_ = { width, height };

        bcd::HistogramParameters histo_params2;
        histo_params2.m_nbOfBins = histo_params.n_bins;
        histo_params2.m_gamma    = histo_params.bin_size_exp;
        histo_params2.m_maxValue = histo_params.max_value;
        recorder_ = std::make_unique<Recorder>(resolution_, histo_params2);

        if(auto node = params.find_child("save_statistics_filename"))
            save_statistics_filename_ = init_ctx.path_mgr->get(node->as_value().as_str());
        if(auto node = params.find_child("load_statistics_filename"))
            load_statistics_filename_ = init_ctx.path_mgr->get(node->as_value().as_str());

        AGZ_HIERARCHY_WRAP("in initializing bcd film object")
    }

    void merge_grid(FilmGrid &&grid) override
    {
        assert(recorder_);
        auto &tgrid = dynamic_cast<BCDFilmGrid&>(grid);
        recorder_->add_batch(std::move(tgrid.batch_));
    }

    std::unique_ptr<FilmGrid> new_grid(int x_beg, int x_end, int y_beg, int y_end) const override
    {
        return std::make_unique<BCDFilmGrid>(x_beg, x_end, y_beg, y_end);
    }

    texture::texture2d_t<Spectrum> image() const override
    {
        assert(image_);
        return *image_;
    }

    GBuffer gbuffer() const override
    {
        return GBuffer();
    }

    Vec2i resolution() const noexcept override
    {
        return resolution_;
    }

    void map_spectrum(std::function<Spectrum(const Spectrum&)> func) override
    {
        assert(image_);
        image_->get_data() = image_->get_data().map(func);
    }

    void map_weight(std::function<real(real)> func) override
    {
        weight_ = func(weight_);
    }

    void end() override
    {
        assert(!image_);

        bcd::SamplesStatisticsImages raw = std::move(*recorder_).extract_sample_statistics();
        recorder_ = nullptr;

        if(!save_statistics_filename_.empty())
        {
            AGZ_LOG0("saving bcd sample statistics to ", save_statistics_filename_);
            save_samples_statistics(save_statistics_filename_, raw);
        }

        if(!load_statistics_filename_.empty())
        {
            AGZ_LOG0("loading bcd sample statistics from ", load_statistics_filename_);
            auto new_raw = load_samples_statistics(load_statistics_filename_);
            if(!raw.m_meanImage.isEmpty())
                raw = std::move(new_raw);
        }

        bcd::Deepimf output_img(raw.m_meanImage);

        if(denoise_)
        {
            bcd::DenoiserInputs inputs;
            bcd::DenoiserOutputs outputs;
            bcd::DenoiserParameters params;

            if(denoiser_params_.prefilter_spikes)
            {
                AGZ_LOG1("prefiltering spikes");
                bcd::SpikeRemovalFilter::filter(
                    raw.m_meanImage,
                    raw.m_nbOfSamplesImage,
                    raw.m_histoImage,
                    raw.m_covarImage,
                    denoiser_params_.prefilter_threshold_stdev_factor);
            }

            inputs.m_pColors = &raw.m_meanImage;
            inputs.m_pNbOfSamples = &raw.m_nbOfSamplesImage;
            inputs.m_pHistograms = &raw.m_histoImage;
            inputs.m_pSampleCovariances = &raw.m_covarImage;

            outputs.m_pDenoisedColors = &output_img;

            params.m_histogramDistanceThreshold = denoiser_params_.histo_patch_distance_threshold;
            params.m_patchRadius = denoiser_params_.patch_radius;
            params.m_searchWindowRadius = denoiser_params_.search_window_radius;
            params.m_minEigenValue = denoiser_params_.min_eigen;
            params.m_useRandomPixelOrder = denoiser_params_.random_pixel_order;
            params.m_markedPixelsSkippingProbability = denoiser_params_.marked_pixel_skipping_prob;
            params.m_nbOfCores = 0;
            params.m_useCuda = false;

            std::unique_ptr<bcd::IDenoiser> denoiser;
            if(denoiser_params_.n_scales > 1)
                denoiser = std::make_unique<bcd::MultiscaleDenoiser>(denoiser_params_.n_scales);
            else
                denoiser = std::make_unique<bcd::Denoiser>();

            denoiser->setInputs(inputs);
            denoiser->setOutputs(outputs);
            denoiser->setParameters(params);

            AGZ_LOG1("denoising");

            denoiser->denoise();

            clamp_not_finite_values(output_img);
        }
        else
            output_img = std::move(raw.m_meanImage);

        assert(output_img.getDepth() == 3);
        image_ = std::make_unique<texture::texture2d_t<Spectrum>>(resolution_.y, resolution_.x);
        for(int y = 0; y < image_->height(); ++y)
        {
            for(int x = 0; x < image_->width(); ++x)
            {
                auto &pixel = (*image_)(y, x);
                pixel.r = output_img.get(y, x, 0);
                pixel.g = output_img.get(y, x, 1);
                pixel.b = output_img.get(y, x, 2);
            }
        }
        weight_ = 1;
    }
};

} // namespace bcd_film

using BCDFilm = bcd_film::BCDFilm;

AGZT_IMPLEMENTATION(Film, BCDFilm, "bcd")

AGZ_TRACER_END

#endif // #ifdef USE_BCD
