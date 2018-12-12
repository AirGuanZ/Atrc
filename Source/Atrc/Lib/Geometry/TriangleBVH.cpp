#include <Atrc/Lib/Geometry/TriangleBVH.h>

#ifdef AGZ_USE_SSE2

#include <emmintrin.h>

#endif

namespace Atrc
{

namespace
{

    template<typename T>
    struct BVHNode
    {
        T low[3], high[3];
        uint32_t begin, end;  
        uint32_t rightOffset; 

        bool HasIntersect(const T *ori, const T *invDir, T t0, T t1) const noexcept
        {
            T nx = invDir[0] * (low[0]  - ori[0]);
            T ny = invDir[1] * (low[1]  - ori[1]);
            T nz = invDir[2] * (low[2]  - ori[2]);

            T fx = invDir[0] * (high[0] - ori[0]);
            T fy = invDir[1] * (high[1] - ori[1]);
            T fz = invDir[2] * (high[2] - ori[2]);

            t0 = Max(t0, Min(nx, fx));
            t0 = Max(t0, Min(ny, fy));
            t0 = Max(t0, Min(nz, fz));

            t1 = Min(t1, Max(nx, fx));
            t1 = Min(t1, Max(ny, fy));
            t1 = Min(t1, Max(nz, fz));

            return t0 <= t1;
        }
    };

#ifdef AGZ_USE_SSE2

    template<>
    struct alignas(16) BVHNode<float>
    {
        float low[3];
        uint32_t rightOffset;
        float high[3];
        uint32_t begin, end;

        bool HasIntersect(const float *pOri, const float *pInvDir, float t0, float t1) const noexcept
        {
            AGZ_ASSERT(size_t(pOri) % 16 == 0 && size_t(pInvDir) % 16 == 0);

            // 参见 http://www.flipcode.com/archives/SSE_RayBox_Intersection_Test.shtml
            alignas(16) static const float psCstPlusInf[4] =
            {
                -logf(0), -logf(0), -logf(0), -logf(0)
            };
            alignas(16) static const float psCstMinusInf[4] =
            {
                logf(0), logf(0), logf(0), logf(0)
            };

            AGZ_ASSERT(size_t(psCstPlusInf) % 16 == 0 && size_t(psCstMinusInf) % 16 == 0);

            const __m128 plusInf  = _mm_load_ps(psCstPlusInf);
            const __m128 minusInf = _mm_load_ps(psCstMinusInf);

            const __m128 boxMin = _mm_load_ps(low);
            const __m128 boxMax = _mm_load_ps(high);
            const __m128 ori    = _mm_load_ps(pOri);
            const __m128 invDir = _mm_load_ps(pInvDir);

            const __m128 l1 = _mm_mul_ps(_mm_sub_ps(boxMin, ori), invDir);
            const __m128 l2 = _mm_mul_ps(_mm_sub_ps(boxMax, ori), invDir);

            const __m128 filteredL1a = _mm_min_ps(l1, plusInf);
            const __m128 filteredL2a = _mm_min_ps(l2, plusInf);

            const __m128 filteredL1b = _mm_max_ps(l1, minusInf);
            const __m128 filteredL2b = _mm_max_ps(l2, minusInf);

            __m128 lmax = _mm_max_ps(filteredL1a, filteredL2a);
            __m128 lmin = _mm_min_ps(filteredL1b, filteredL2b);

            const __m128 lmax0 = _mm_shuffle_ps(lmax, lmax, 0x39);
            const __m128 lmin0 = _mm_shuffle_ps(lmin, lmin, 0x39);

            lmax = _mm_min_ss(lmax, lmax0);
            lmin = _mm_max_ss(lmin, lmin0);

            const __m128 lmax1 = _mm_movehl_ps(lmax, lmax);
            const __m128 lmin1 = _mm_movehl_ps(lmin, lmin);

            lmax = _mm_min_ss(lmax, lmax1);
            lmin = _mm_max_ss(lmin, lmin1);

            float t0_, t1_;
            _mm_store_ss(&t0_, lmin);
            _mm_store_ss(&t1_, lmax);

            return t1_ >= t0 && t1 >= t0_;
        }
    };

#endif

    bool HasIntersectionWithTriangle(
        const Ray &r, const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A) noexcept
    {
        // TODO
        return false;
    }

    struct TriangleIntersectionRecoed
    {
        Real t;
        Vec2 uv;
    };

    bool FindIntersectionWithTriangle(
        const Ray &r, const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A,
        TriangleIntersectionRecoed *record) noexcept
    {
        // TODO
        return false;
    }
}

struct alignas(16) TriangleBVHNode : public BVHNode<Real> { };

static_assert(sizeof(TriangleBVHNode) == sizeof(BVHNode<Real>));

#ifdef AGZ_USE_SSE2

static_assert(alignof(TriangleBVHNode) % 16 == 0);

#endif

} // namespace Atrc
