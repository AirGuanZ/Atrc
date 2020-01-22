#pragma once

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief 用于展示渲染进度的reporter接口
 * 
 * 实现不必管线程安全
 * 
 * 使用姿势：
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
     * @brief 汇报进度数据
     * 
     * @param percent 百分比，范围[0, 100]
     * 
     * 在多线程环境下汇报的数据不一定保证递增
     */
    virtual void progress(double percent, const std::function<Image2D<Spectrum>()> &get_image_preview) = 0;

    /**
     * @brief 输出一条消息
     */
    virtual void message(const std::string &msg) = 0;

    /**
     * @brief 输出一条错误信息
     */
    virtual void error(const std::string &err) = 0;

    /**
     * @brief 开始渲染的回调函数
     */
    virtual void begin() = 0;

    /**
     * @brief 结束渲染的回调函数
     */
    virtual void end() = 0;

    /**
     * @brief 开始一个新渲染pass，调用后progress给的percent会重新累计
     */
    virtual void new_stage() = 0;

    /**
     * @brief 结束一个渲染pass
     */
    virtual void end_stage() = 0;

    /**
     * @brief 到目前为止所有已完成的pass用了多少秒
     */
    virtual double total_seconds() = 0;

    /**
     * @brief 上一个完成的pass用了多少秒
     */
    virtual double last_stage_seconds() = 0;
};

AGZ_TRACER_END
