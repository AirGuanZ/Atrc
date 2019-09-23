#pragma once

#include <agz/rasterizer/common.h>

AGZ_RASTERIZER_BEGIN

template<typename TInterpolator, typename TVarying>
class TriangleClipper
{
    static bool visible(const Vec4 &pos) noexcept
    {
        return std::abs(pos.x) <= pos.w && std::abs(pos.y) <= pos.w && std::abs(pos.z) <= pos.w;
    }

    enum class ClipPlane
    {
        PosX, NegX,
        PosY, NegY,
        PosZ, NegZ,
        PosW
    };

    template<ClipPlane CLIP_PLANE>
    static bool is_inside(const Vec4 &pos) noexcept
    {
        if constexpr(CLIP_PLANE == ClipPlane::PosX)
            return pos.x <= pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::NegX)
            return pos.x >= -pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::PosY)
            return pos.y <= pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::NegY)
            return pos.y >= -pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::PosZ)
            return pos.z <= pos.w;
        if constexpr(CLIP_PLANE == ClipPlane::NegZ)
            return pos.z >= -pos.w;
        return pos.w >= EPS;
    }

    template<ClipPlane CLIP_PLANE>
    static real inct_ratio(const Vec4 &last, const Vec4 &curr) noexcept
    {
        if(CLIP_PLANE == ClipPlane::PosW)
            return (last.w - EPS) / (last.w - curr.w);
        if(CLIP_PLANE == ClipPlane::PosX)
            return (last.w - last.x) / ((last.w - last.x) - (curr.w - curr.x));
        if(CLIP_PLANE == ClipPlane::NegX)
            return (last.w + last.x) / ((last.w + last.x) - (curr.w + curr.x));
        if(CLIP_PLANE == ClipPlane::PosY)
            return (last.w - last.y) / ((last.w - last.y) - (curr.w - curr.y));
        if(CLIP_PLANE == ClipPlane::NegY)
            return (last.w + last.y) / ((last.w + last.y) - (curr.w + curr.y));
        if(CLIP_PLANE == ClipPlane::PosZ)
            return (last.w - last.z) / ((last.w - last.z) - (curr.w - curr.z));
        return (last.w + last.z) / ((last.w + last.z) - (curr.w + curr.z));
    }

    template<ClipPlane CLIP_PLANE>
    int clip_with_plane(const TVarying *input, int input_count, TVarying *output) const noexcept
    {
        int output_count = 0;

        for(int i = 0; i < input_count; ++i)
        {
            const Varying &last = input[(i - 1 + input_count) % input_count];
            const Varying &curr = input[i];

            bool is_last_inside = is_inside<CLIP_PLANE>(last.agz_position);
            bool is_curr_inside = is_inside<CLIP_PLANE>(curr.agz_position);

            if(is_last_inside != is_curr_inside)
            {
                Varying &dest = output[output_count++];

                real z1 = last.agz_position.z, z2 = curr.agz_position.z;
                real ratio = inct_ratio<CLIP_PLANE>(last.agz_position, curr.agz_position);
                real corr_ratio = z1 * ratio / (z2 + ratio * (z1 - z2));

                dest.agz_position = math::mix(last.agz_position, curr.agz_position, ratio);
                interpolator->process(last, curr, ratio, corr_ratio, &dest);
            }

            if(is_curr_inside)
                output[output_count++] = curr;
        }

        return output_count;
    }

public:

    using Interpolator = TInterpolator;
    using Varying      = TVarying;

    const Interpolator *interpolator;

    static constexpr int CLIPPED_TRIANGLE_MAX_COUNT = 12;

    int clip(const Varying *input, Varying *output) const noexcept
    {
        bool is_v0_visible = visible(input[0].agz_position);
        bool is_v1_visible = visible(input[1].agz_position);
        bool is_v2_visible = visible(input[2].agz_position);
        if(is_v0_visible && is_v1_visible && is_v2_visible)
        {
            output[0] = input[0];
            output[1] = input[1];
            output[2] = input[2];
            return 3;
        }

        int varying_count = 3;
        Varying toutput2[CLIPPED_TRIANGLE_MAX_COUNT];
        varying_count = clip_with_plane<ClipPlane::PosW>(input, varying_count, output);
        varying_count = clip_with_plane<ClipPlane::PosX>(output, varying_count, toutput2);
        varying_count = clip_with_plane<ClipPlane::NegX>(toutput2, varying_count, output);
        varying_count = clip_with_plane<ClipPlane::PosY>(output, varying_count, toutput2);
        varying_count = clip_with_plane<ClipPlane::NegY>(toutput2, varying_count, output);
        varying_count = clip_with_plane<ClipPlane::PosZ>(output, varying_count, toutput2);
        varying_count = clip_with_plane<ClipPlane::NegZ>(toutput2, varying_count, output);

        return varying_count;
    }
};

AGZ_RASTERIZER_END
