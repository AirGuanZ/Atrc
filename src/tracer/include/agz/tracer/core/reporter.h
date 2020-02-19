#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief progress reporter interface
 * 
 * all methods are not thread-safe
 * 
 * typical usage:
 *  reporter.begin();
 *  { reporter.new_stage(); reporter.end_stage(); }*
 *  reporter.end();
 */
class ProgressReporter
{
public:

    virtual ~ProgressReporter() = default;

    virtual bool need_image_preview() const noexcept = 0;

    /**
     * @brief report the progress data
     * 
     * @param percent range: [0, 100]
     * 
     * reported values are not guaranteed to ↗ in multi-thread rendering
     */
    virtual void progress(double percent, const std::function<Image2D<Spectrum>()> &get_image_preview) = 0;

    /**
     * @brief output a ordinary message
     */
    virtual void message(const std::string &msg) = 0;

    /**
     * @brief output an error message
     */
    virtual void error(const std::string &err) = 0;

    /**
     * @brief start rendering
     */
    virtual void begin() = 0;

    /**
     * @brief complete rendering
     */
    virtual void end() = 0;

    /**
     * @brief start a new rendering stage. progress percentage will be reset to 0
     */
    virtual void new_stage() = 0;

    /**
     * @brief complete the rendering stage
     */
    virtual void end_stage() = 0;

    /**
     * @brief totally used seconds
     */
    virtual double total_seconds() = 0;

    /**
     * @brief how many seconds does last rendering stage use
     */
    virtual double last_stage_seconds() = 0;
};

AGZ_TRACER_END
