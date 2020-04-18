#pragma once

#include <mutex>

#include <QObject>

#include <agz/tracer/tracer.h>

class GUIProgressReporter : public QObject, public agz::tracer::RendererInteractor
{
    Q_OBJECT

public:

    using Clock = std::chrono::steady_clock;

    explicit GUIProgressReporter(Clock::duration update_preview_interval);

    bool need_image_preview() const noexcept override;

    void progress(double percent, const PreviewFunc &get_image_preview) override;

    void message(const std::string &msg) override;

    void begin() override;

    void end() override;

    void new_stage() override;

    void end_stage() override;

    agz::tracer::Image2D<agz::tracer::Spectrum> get_preview_image();

private:

    double percent_ = 0;

    Clock::duration update_preview_interval_;
    Clock::time_point last_update_preview_time_;

    std::mutex preview_image_mutex_;
    agz::tracer::Image2D<agz::tracer::Spectrum> preview_image_;

signals:

    void update_preview();

    void update_pbar(double percent);
};
