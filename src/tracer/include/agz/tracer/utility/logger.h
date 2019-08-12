#pragma once

#include <cassert>
#include <iostream>
#include <sstream>
#include <string_view>

#include <agz/tracer/common.h>

AGZ_TRACER_BEGIN

/**
 * @brief 简单的日志设施
 * 
 * ProgressReporter用于展示渲染进度和渲染过程中的关键消息，Logger则主要用于记录建立场景数据结构等过程中的细节
 * 
 * 日志信息分为三个级别，由低到高愈加详细：
 * 
 * 0. 纲要类，指出渲染器正在进行的大阶段，以及非正常输入产生的警告和错误信息
 * 1. 细节类，输出一些数据结构相关的重要参数和小阶段转换
 * 2. DEBUG类，输出一些只可能对debug有用的详细信息
 * 
 * 可以通过定义宏AGZ_LOG_LEVEL为0/1/2来指定至多输出哪一级别的日志，若未定义，默认不记录日志
 * 
 * agz中除了渲染部分外基本不存在并行，故Logger的实现不要求线程安全
 */
class Logger : public misc::uncopyable_t
{
    template<typename Arg>
    static void log_aux(std::stringstream &sst, const Arg &arg)
    {
        sst << arg;
    }

    template<typename Arg, typename Arg1, typename...Args>
    static void log_aux(std::stringstream &sst, const Arg &arg, const Arg1 &arg1, const Args&...args)
    {
        log_aux(sst, arg);
        log_aux(sst, arg1, args...);
    }

public:

    virtual ~Logger() = default;

    virtual void msg(std::string_view src_file, int src_line, std::string_view msg) = 0;

    template<typename...Args>
    void log(std::string_view src_file, int src_line, const Args&...args)
    {
        std::stringstream sst;
        log_aux(sst, args...);
        msg(src_file, src_line, sst.str());
    }
};

namespace logger_impl
{

// 不直接用inline variable是因为辣鸡MSVC的bug
inline std::unique_ptr<Logger> &global_logger()
{
    static std::unique_ptr<Logger> logger;
    return logger;
}

} // namespace logger_impl
    
inline void set_global_logger(std::unique_ptr<Logger> &&logger)
{
    logger_impl::global_logger() = std::move(logger);
}

inline Logger *get_global_logger()
{
    auto ret = logger_impl::global_logger().get();
    assert(ret);
    return ret;
}

#ifndef AGZ_LOG_LEVEL
#   define AGZ_LOG0(...) do { } while(false)
#   define AGZ_LOG1(...) do { } while(false)
#   define AGZ_LOG2(...) do { } while(false)
#elif (AGZ_LOG_LEVEL == 0)
#   define AGZ_LOG0(...) do { (::agz::tracer::get_global_logger())->log(__FILE__, __LINE__, "[LOG0] ", __VA_ARGS__); } while(false)
#   define AGZ_LOG1(...) do { } while(false)
#   define AGZ_LOG2(...) do { } while(false)
#elif (AGZ_LOG_LEVEL == 1)
#   define AGZ_LOG0(...) do { (::agz::tracer::get_global_logger())->log(__FILE__, __LINE__, "[LOG0] ", __VA_ARGS__); } while(false)
#   define AGZ_LOG1(...) do { (::agz::tracer::get_global_logger())->log(__FILE__, __LINE__, "[LOG1] ", __VA_ARGS__); } while(false)
#   define AGZ_LOG2(...) do { } while(false)
#elif (AGZ_LOG_LEVEL == 2)
#   define AGZ_LOG0(...) do { (::agz::tracer::get_global_logger())->log(__FILE__, __LINE__, "[LOG0] ", __VA_ARGS__); } while(false)
#   define AGZ_LOG1(...) do { (::agz::tracer::get_global_logger())->log(__FILE__, __LINE__, "[LOG1] ", __VA_ARGS__); } while(false)
#   define AGZ_LOG2(...) do { (::agz::tracer::get_global_logger())->log(__FILE__, __LINE__, "[LOG2] ", __VA_ARGS__); } while(false)
#else
#   error "invalid AGZ_LOG_LEVEL macro value"
#endif

class StdOutLogger : public Logger
{
public:

    void msg(std::string_view, int, std::string_view msg) override
    {
        std::cout << msg << std::endl;
    }
};

AGZ_TRACER_END
