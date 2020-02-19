#pragma once

#include <mutex>

#include <QObject>

#include <agz/tracer/tracer.h>
#include <agz/utility/time.h>

class GUIProgressReporter : public QObject, public agz::tracer::ProgressReporter
{
    Q_OBJECT

public:

    using Clock = std::chrono::steady_clock;

    explicit GUIProgressReporter(Clock::duration update_preview_interval);

    bool need_image_preview() const noexcept override;

    void progress(double percent, const std::function<agz::texture::texture2d_t<agz::math::tcolor3<float>>()> &get_image_preview) override;

    void message(const std::string &msg) override;

    void error(const std::string &err) override;

    void begin() override;

    void end() override;

    void new_stage() override;

    void end_stage() override;

    double total_seconds() override;

    double last_stage_seconds() override;

    agz::tracer::Image2D<agz::tracer::Spectrum> get_preview_image();

private:

    double percent_ = 0;

    agz::time::clock_t clock_;
    double total_seconds_ = 0;
    double last_stage_seconds_ = 0;

    Clock::duration update_preview_interval_;
    Clock::time_point last_update_preview_time_;

    std::mutex preview_image_mutex_;
    agz::tracer::Image2D<agz::tracer::Spectrum> preview_image_;

signals:

    void update_preview();

    void update_pbar(double percent);
};
