#include <functional>

#include <agz/gui/reporter.h>
#include <agz/tracer/tracer.h>

GUIProgressReporter::GUIProgressReporter(Clock::duration update_preview_interval)
    : update_preview_interval_(update_preview_interval),
      last_update_preview_time_(Clock::now())
{

}

bool GUIProgressReporter::need_image_preview() const noexcept
{
    return true;
}

void GUIProgressReporter::progress(
    double percent, const PreviewFunc &get_image_preview)
{
    if(percent > percent_)
    {
        percent_ = percent;
        emit update_pbar(percent_);
    }

    const auto delta_t = Clock::now() - last_update_preview_time_;
    const bool its_update_time = delta_t >= update_preview_interval_;
    if(get_image_preview && (its_update_time || percent_ >= 100))
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

void GUIProgressReporter::begin()
{

}

void GUIProgressReporter::end()
{
    emit update_preview();
}

void GUIProgressReporter::new_stage()
{
    percent_ = 0;

    last_update_preview_time_ = Clock::now();
}

void GUIProgressReporter::end_stage()
{

}

agz::tracer::Image2D<agz::tracer::Spectrum> GUIProgressReporter::get_preview_image()
{
    std::lock_guard lk(preview_image_mutex_);
    return preview_image_;
}
