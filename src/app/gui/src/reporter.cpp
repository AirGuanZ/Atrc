#include <functional>

#include <agz/gui/reporter.h>
#include <agz/tracer/utility/logger.h>

GUIProgressReporter::GUIProgressReporter(Clock::duration update_preview_interval)
    : update_preview_interval_(update_preview_interval), last_update_preview_time_(Clock::now())
{

}

bool GUIProgressReporter::need_image_preview() const noexcept
{
    return true;
}

void GUIProgressReporter::progress(double percent, const std::function<agz::texture::texture2d_t<agz::math::tcolor3<float>>()> &get_image_preview)
{
    if(percent > percent_)
    {
        percent_ = percent;
        emit update_pbar(percent_);
    }

    if(get_image_preview && (Clock::now() - last_update_preview_time_ >= update_preview_interval_ || percent_ >= 100))
    {
        last_update_preview_time_ = Clock::now();

        auto img = get_image_preview();

        {
            std::lock_guard lk(preview_image_mutex_);
            preview_image_.swap(img);
        }

        emit update_preview();
    }
}

void GUIProgressReporter::message(const std::string &msg)
{
    AGZ_INFO(msg);
}

void GUIProgressReporter::error(const std::string &err)
{
    AGZ_ERROR(err);
}

void GUIProgressReporter::begin()
{
    total_seconds_ = 0;
}

void GUIProgressReporter::end()
{
    emit update_preview();
}

void GUIProgressReporter::new_stage()
{
    percent_ = 0;
    clock_.restart();

    last_update_preview_time_ = Clock::now();
}

void GUIProgressReporter::end_stage()
{
    last_stage_seconds_ = clock_.ms() / 1000.0;
    total_seconds_ += last_stage_seconds_;
}

double GUIProgressReporter::total_seconds()
{
    return total_seconds_;
}

double GUIProgressReporter::last_stage_seconds()
{
    return last_stage_seconds_;
}

agz::tracer::Image2D<agz::tracer::Spectrum> GUIProgressReporter::get_preview_image()
{
    std::lock_guard lk(preview_image_mutex_);
    return preview_image_;
}
