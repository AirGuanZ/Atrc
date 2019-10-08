#pragma once

#include <mutex>

#include <agz/tracer/core/gbuffer.h>
#include <agz/common/math.h>
#include <agz/utility/texture.h>

AGZ_TRACER_BEGIN

using ImageBuffer = texture::texture2d_t<Spectrum>;

/**
 * @brief 一块完整film上的一块矩形区域，和原film的存储是分离的
 */
class FilmGrid : public misc::uncopyable_t
{
public:

    virtual ~FilmGrid() = default;

    /** @brief 有效采样区域的x坐标最小值，以像素为单位 */
    virtual int sample_x_beg() const noexcept = 0;

    /** @brief 有效采样区域的x坐标最大值+1 */
    virtual int sample_x_end() const noexcept = 0;

    /** @brief 有效采样区域的y坐标最小值 */
    virtual int sample_y_beg() const noexcept = 0;

    /** @brief 有效采样区域的y坐标最大值+1 */
    virtual int sample_y_end() const noexcept = 0;

    /** @brief 添加一个采样点，更新内部存储的像素和权重数据 */
    virtual void add_sample(const Vec2 &pos, const Spectrum &value, const GBufferPixel &gpixel, real w = 1) = 0;
};

class Film
{
public:

    virtual ~Film() = default;

    /**
     * @brief 线程安全当且仅当并行合并的grid间不重合
     */
    virtual void merge_grid(const FilmGrid &grid) = 0;

    /**
     * @brief 生成一块新grid
     * 
     * 参数单位为像素，需满足：
     * - 0 <= x_beg <= x_end <= w_
     * - 0 <= y_beg <= y_end <= h_
     * 
     * 线程安全
     */
    virtual std::unique_ptr<FilmGrid> new_grid(int x_beg, int x_end, int y_beg, int y_end) const = 0;

    /**
     * @brief 基于一块已有的grid生成一块新的
     * 
     * 尽可能重用原来的内存空间
     * 
     * 线程安全
     */
    virtual std::unique_ptr<FilmGrid> renew_grid(int x_beg, int x_end, int y_beg, int y_end, std::unique_ptr<FilmGrid> &&old_grid) const
    {
        return new_grid(x_beg, x_end, y_beg, y_end);
    }

    /**
     * @brief 返回最终图像
     * 
     * 线程不安全
     */
    virtual ImageBuffer image() const = 0;

    /**
     * @brief 返回GBuffer，其中的成员可以为空，表示该film不支持记录此项信息
     * 
     * 线程不安全
     */
    virtual GBuffer gbuffer() const = 0;

    /**
     * @brief 图像分辨率
     * 
     * 线程安全
     */
    virtual Vec2i resolution() const noexcept = 0;
};

AGZ_TRACER_END
