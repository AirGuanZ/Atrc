#include <queue>
#include <stack>

#include <Atrc/Lib/Geometry/TriangleBVH.h>

#ifdef AGZ_USE_SSE2

#include <emmintrin.h>

#endif

namespace Atrc
{

struct TriangleBVHCorePrimitive
{
    Vec3 A, B_A, C_A;
};

struct TriangleBVHCorePrimitiveInfo
{
    Vec3 nA, nB_nA, nC_nA;
    Vec2 tA, tB_tA, tC_tA;
    CoordSystem coordSys;
};

namespace
{
    template<typename T>
    struct /*alignas(16)*/ BVHNode
    {
        T low[3], high[3];
        uint32_t start, end;  
        uint32_t rightOffset; // rightOffset为0表示叶节点，否则为内部节点

        bool IsLeaf() const noexcept
        {
            return !rightOffset;
        }

        bool HasIntersect(const T *ori, const T *invDir, T t0, T t1, T *t) const noexcept
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

            *t = t0;

            return t0 <= t1;
        }
    };

#ifdef AGZ_USE_SSE2

    /*
    template<>
    struct alignas(16) BVHNode<float>
    {
        float low[4];
        float high[4];
        uint32_t rightOffset;
        uint32_t start, end;

        BVHNode() { low[3] = high[3] = 0; }

        bool IsLeaf() const noexcept
        {
            return !rightOffset;
        }

        bool HasIntersect(const float *pOri, const float *pInvDir, float t0, float t1, float *t) const noexcept
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

            *t = Max(t0, t0_);

            return _mm_comige_ss(lmax, lmin) && t1_ >= t0 && t1 >= t0_;
        }
    };*/

#endif

    bool HasIntersectionWithTriangle(
        const Ray &r, const Vec3 &A, const Vec3 &B_A, const Vec3 &C_A) noexcept
    {
        Vec3 s1 = Cross(r.d, C_A);
        Real div = Dot(s1, B_A);
        if(!div)
            return false;
        Real invDiv = 1 / div;

        Vec3 o_A = r.o - A;
        Real alpha = Dot(o_A, s1) * invDiv;
        if(alpha < 0 || alpha > 1)
            return false;

        Vec3 s2 = Cross(o_A, B_A);
        Real beta = Dot(r.d, s2) * invDiv;
        if(beta < 0 || alpha + beta > 1)
            return false;

        Real t = Dot(C_A, s2) * invDiv;
        return r.Between(t);
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
        AGZ_ASSERT(record);

        Vec3 s1 = Cross(r.d, C_A);
        Real div = Dot(s1, B_A);
        if(!div)
            return false;
        Real invDiv = 1 / div;

        Vec3 o_A = r.o - A;
        Real alpha = Dot(o_A, s1) * invDiv;
        if(alpha < 0)
            return false;

        Vec3 s2 = Cross(o_A, B_A);
        Real beta = Dot(r.d, s2) * invDiv;
        if(beta < 0 || alpha + beta > 1)
            return false;

        Real t = Dot(C_A, s2) * invDiv;

        if(!r.Between(t))
            return false;

        record->t  = t;
        record->uv = Vec2(alpha, beta);

        return true;
    }

    Real GetTriangleArea(const Vec3 &B_A, const Vec3 &C_A) noexcept
    {
        return Cross(B_A, C_A).Length() / 2;
    }
}

struct /*alignas(16)*/ TriangleBVHCoreNode : BVHNode<Real> { };

static_assert(sizeof(TriangleBVHCoreNode) == sizeof(BVHNode<Real>));

#ifdef AGZ_USE_SSE2

/*static_assert(alignof(TriangleBVHCoreNode) % 16 == 0);*/

#endif

namespace
{
    // 建立BVH时临时使用的链式节点，left == right == nullptr时为leaf，否则为interior
    struct BuildNode
    {
        AABB bounding;
        BuildNode *left, *right;
        uint32_t start, end;
    };

    struct BuildResult
    {
        BuildNode *root;
        uint32_t nodeCount;
    };

    struct BuildTriangle
    {
        const TriangleBVHCore::Vertex *vtx;
        Vec3 centroid;
    };

    BuildResult BuildBVH(
        BuildTriangle *triangles, uint32_t triangleCount,
        uint32_t leafSizeThreshold, uint32_t depthThreshold, Arena &arena)
    {
        struct BuildTask
        {
            BuildNode **fillback;
            uint32_t start, end;
            uint32_t depth;
        };

        BuildResult ret = { nullptr, 0 };

        std::queue<BuildTask> tasks;
        tasks.push({ &ret.root, 0, triangleCount, 0 });

        while(!tasks.empty())
        {
            BuildTask task = tasks.front();
            tasks.pop();

            // 计算整体包围盒和中心点包围盒

            AABB allBound, centroidBound;
            for(uint32_t i = task.start; i < task.end; ++i)
            {
                auto &tri = triangles[i];
                allBound.Expand(tri.vtx[0].pos)
                        .Expand(tri.vtx[1].pos)
                        .Expand(tri.vtx[2].pos);
                centroidBound.Expand(tri.centroid);
            }

            AGZ_ASSERT(task.start < task.end);
            uint32_t n = task.end - task.start;

            // n足够小时产生叶节点

            if(n <= leafSizeThreshold)
            {
                ret.nodeCount += 1;

                auto leaf = arena.Create<BuildNode>();
                leaf->bounding = allBound;
                leaf->left     = nullptr;
                leaf->right    = nullptr;
                leaf->start    = task.start;
                leaf->end      = task.end;
                *task.fillback = leaf;

                continue;
            }

            // 根据中心包围盒各维大小选择在哪根坐标轴上划分

            Vec3 centroidDelta = centroidBound.high - centroidBound.low;
            int splitAxis = centroidDelta[0] > centroidDelta[1] ?
                (centroidDelta[0] > centroidDelta[2] ? 0 : 2) :
                (centroidDelta[1] > centroidDelta[2] ? 1 : 2);

            // 深度不是很大时采用位置二分，否则采用数量二分

            uint32_t splitIdx;

            if(task.depth < depthThreshold)
            {
                Real splitPos = (centroidBound.high[splitAxis] + centroidBound.low[splitAxis]) / 2;

                splitIdx = task.start;
                for(uint32_t i = task.start; i < task.end; ++i)
                {
                    if(triangles[i].centroid[splitAxis] < splitPos)
                        std::swap(triangles[i], triangles[splitIdx++]);
                }

                if(splitIdx == task.start || splitIdx == task.end)
                    splitIdx = task.start + n / 2;
            }
            else
            {
                std::sort(
                    triangles + task.start, triangles + task.end,
                    [axis = splitAxis](const BuildTriangle &L, const BuildTriangle &R)
                { return L.centroid[axis] < R.centroid[axis]; });
                splitIdx = task.start + n / 2;
            }

            auto interior = arena.Create<BuildNode>();
            interior->bounding = allBound;
            interior->left     = nullptr;
            interior->right    = nullptr;
            interior->start    = 0;
            interior->end      = 0;

            AGZ_ASSERT(task.fillback);
            *task.fillback = interior;

            ret.nodeCount += 1;

            tasks.push({ &interior->left,  task.start, splitIdx, task.depth + 1 });
            tasks.push({ &interior->right, splitIdx,   task.end, task.depth + 1 });
        }

        return ret;
    }

    Vec3 ComputeEx(
        const Vec3 &B_A, const Vec3 &C_A,
        const Vec2 &b_a, const Vec2 &c_a,
        const Vec3 &ez)
    {
        Real m00 = b_a.u, m01 = b_a.v;
        Real m10 = c_a.u, m11 = c_a.v;
        Real det = m00 * m11 - m01 * m10;
        if(!det)
            return CoordSystem::FromEz(ez).ex;
        Real invDet = 1 / det;
        return (m11 * invDet * B_A - m01 * invDet * C_A).Normalize();
    }

    void CompactBVH(
        const BuildNode *buildNode,
        const BuildTriangle *buildTriangle,
        TriangleBVHCoreNode *node,
        TriangleBVHCorePrimitive *primitive,
        TriangleBVHCorePrimitiveInfo *primitiveInfo)
    {
        struct CompactTask
        {
            const BuildNode *tree;
            uint32_t *fillback;
        };

        uint32_t nextNodeIdx = 0, nextPrimIdx = 0;

        std::stack<CompactTask> tasks;
        tasks.push({ buildNode, nullptr });

        while(!tasks.empty())
        {
            CompactTask task = tasks.top();
            const BuildNode *tree = task.tree;
            tasks.pop();

            if(task.fillback)
                *task.fillback = nextNodeIdx;

            if(tree->left && tree->right) // 内部节点
            {
                auto &nNode = node[nextNodeIdx++];
                for(int i = 0; i < 3; ++i)
                {
                    nNode.low[i]  = tree->bounding.low[i];
                    nNode.high[i] = tree->bounding.high[i];
                }
                nNode.start   = 0;
                nNode.end     = 0;

                tasks.push({ tree->right, &nNode.rightOffset });
                tasks.push({ tree->left, nullptr });
            }
            else // 叶节点
            {
                uint32_t start = nextPrimIdx;
                uint32_t end   = nextPrimIdx + (tree->end - tree->start);

                auto &nNode = node[nextNodeIdx++];
                for(int i = 0; i < 3; ++i)
                {
                    nNode.low[i] = tree->bounding.low[i];
                    nNode.high[i] = tree->bounding.high[i];
                }
                nNode.start       = start;
                nNode.end         = end;
                nNode.rightOffset = 0;

                for(uint32_t i = start, j = tree->start; i < end; ++i, ++j)
                {
                    AGZ_ASSERT(j < tree->end);

                    auto &prim     = primitive[i];
                    auto &primInfo = primitiveInfo[i];
                    auto &tri      = buildTriangle[j];

                    Vec3 nA = tri.vtx[0].nor.Normalize();
                    Vec3 nB = tri.vtx[1].nor.Normalize();
                    Vec3 nC = tri.vtx[2].nor.Normalize();

                    prim.A   = tri.vtx[0].pos;
                    prim.B_A = tri.vtx[1].pos - tri.vtx[0].pos;
                    prim.C_A = tri.vtx[2].pos - tri.vtx[0].pos;

                    primInfo.nA    = nA;
                    primInfo.nB_nA = nB - nA;
                    primInfo.nC_nA = nC - nA;

                    primInfo.tA    = tri.vtx[0].uv;
                    primInfo.tB_tA = tri.vtx[1].uv - tri.vtx[0].uv;
                    primInfo.tC_tA = tri.vtx[2].uv - tri.vtx[0].uv;

                    primInfo.coordSys.ez = Cross(prim.B_A, prim.C_A).Normalize();
                    auto meanNor = nA + nB + nC;
                    if(Dot(primInfo.coordSys.ez, meanNor) < 0)
                        primInfo.coordSys.ez = -primInfo.coordSys.ez;

                    primInfo.coordSys.ex = ComputeEx(
                        prim.B_A, prim.C_A,
                        primInfo.tB_tA, primInfo.tC_tA,
                        primInfo.coordSys.ez);
                    primInfo.coordSys.ey = Cross(
                        primInfo.coordSys.ez, primInfo.coordSys.ex).Normalize();
                }

                nextPrimIdx = end;
            }
        }
    }
}

TriangleBVHCore::TriangleBVHCore(const Vertex *vertices, uint32_t triangleCount)
    : nodes_(nullptr), prims_(nullptr), primsInfo_(nullptr)
{
    AGZ_ASSERT(vertices && triangleCount);
    InitBVH(vertices, triangleCount);
}

TriangleBVHCore::TriangleBVHCore(TriangleBVHCore &&moveFrom) noexcept
    : nodes_(moveFrom.nodes_), prims_(moveFrom.prims_), primsInfo_(moveFrom.primsInfo_)
{
    moveFrom.nodes_     = nullptr;
    moveFrom.prims_     = nullptr;
    moveFrom.primsInfo_ = nullptr;
}

TriangleBVHCore::~TriangleBVHCore()
{
    if(nodes_)
        AGZ::CRTAllocator::Free(nodes_); /* FreeAligned(nodes_); */
    if(prims_)
        AGZ::CRTAllocator::Free(prims_);
    if(primsInfo_)
        AGZ::CRTAllocator::Free(primsInfo_);
}

AABB TriangleBVHCore::GetLocalBound() const noexcept
{
    return {
        { nodes_[0].low[0],  nodes_[0].low[1],  nodes_[0].low[2] },
        { nodes_[0].high[0], nodes_[0].high[1], nodes_[0].high[2] }
    };
}

Real TriangleBVHCore::GetSurfaceArea() const noexcept
{
    return areaPrefixSum_.back();
}

namespace
{
    constexpr int TVL_STK_SIZE = 128;
    thread_local uint32_t tasks[TVL_STK_SIZE];
}

bool TriangleBVHCore::HasIntersection(Ray r) const noexcept
{
    AGZ_ASSERT(nodes_ && prims_ && primsInfo_ && areaPrefixSum_.size());

    alignas(16) Real ori[4]    = { r.o.x,     r.o.y,     r.o.z,     1 };
    alignas(16) Real invDir[4] = { 1 / r.d.x, 1 / r.d.y, 1 / r.d.z, 1 };

    int top = 0;
    tasks[top++] = 0;

    while(top)
    {
        uint32_t taskNodeIdx = tasks[--top];
        const TriangleBVHCoreNode &node = nodes_[taskNodeIdx];

        if(node.rightOffset) // 内部节点
        {
            Real t;
            if(node.HasIntersect(ori, invDir, r.t0, r.t1, &t))
            {
                AGZ_ASSERT(top + 2 <= TVL_STK_SIZE);
                tasks[top++] = taskNodeIdx + 1;
                tasks[top++] = node.rightOffset;
            }
        }
        else
        {
            for(uint32_t i = node.start; i < node.end; ++i)
            {
                auto &prim = prims_[i];
                if(HasIntersectionWithTriangle(r, prim.A, prim.B_A, prim.C_A))
                    return true;
            }
        }
    }

    return false;
}

bool TriangleBVHCore::FindIntersection(Ray r, GeometryIntersection *inct) const noexcept
{
    AGZ_ASSERT(nodes_ && prims_ && primsInfo_ && areaPrefixSum_.size());

    alignas(16) Real ori[4] = { r.o.x,     r.o.y,     r.o.z,     1 };
    alignas(16) Real invDir[4] = { 1 / r.d.x, 1 / r.d.y, 1 / r.d.z, 1 };

    int top = 0; Real t;
    if(nodes_[0].HasIntersect(ori, invDir, r.t0, r.t1, &t))
        tasks[top++] = 0;
    else
        return false;

    TriangleIntersectionRecoed rc, trc;
    rc.t = RealT::Infinity();

    uint32_t flag0 = 0;

    while(top)
    {
        uint32_t taskNodeIdx = tasks[--top];
        const TriangleBVHCoreNode &node = nodes_[taskNodeIdx];

        if(node.rightOffset)
        {
            Real tLeft, tRight;

            bool addLeft  = nodes_[taskNodeIdx + 1] .HasIntersect(ori, invDir, r.t0, r.t1, &tLeft);
            bool addRight = nodes_[node.rightOffset].HasIntersect(ori, invDir, r.t0, r.t1, &tRight);

            AGZ_ASSERT(top + 2 <= TVL_STK_SIZE);

            if(addLeft && addRight)
            {
                if(tLeft < tRight)
                {
                    tasks[top++] = node.rightOffset;
                    tasks[top++] = taskNodeIdx + 1;
                }
                else
                {
                    tasks[top++] = taskNodeIdx + 1;
                    tasks[top++] = node.rightOffset;
                }
            }
            else if(addLeft)
                tasks[top++] = taskNodeIdx + 1;
            else if(addRight)
                tasks[top++] = node.rightOffset;
        }
        else
        {
            for(uint32_t i = node.start; i < node.end; ++i)
            {
                auto &prim = prims_[i];
                if(FindIntersectionWithTriangle(r, prim.A, prim.B_A, prim.C_A, &trc) && trc.t < rc.t)
                {
                    rc = trc;
                    r.t1 = trc.t;
                    flag0 = i;
                }
            }
        }
    }

    if(RealT(rc.t).IsInfinity())
        return false;

    auto &primInfo = primsInfo_[flag0];

    inct->t        = rc.t;
    inct->pos      = r.At(rc.t);
    inct->wr       = -r.d;
    inct->uv       = rc.uv;
    inct->coordSys = primInfo.coordSys;
    inct->flag0    = flag0;

    // TODO: inct->usr
    inct->usr.uv       = primInfo.tA + rc.uv.u * primInfo.tB_tA + rc.uv.v * primInfo.tC_tA;
    inct->usr.coordSys = inct->coordSys;

    return true;
}

Geometry::SampleResult TriangleBVHCore::Sample(const Vec3 &sample) const
{
    AGZ_ASSERT(nodes_ && prims_ && primsInfo_ && areaPrefixSum_.size());

    // 按面积选择一个三角形

    Real u = sample.x * areaPrefixSum_.back();
    auto upper = std::lower_bound(areaPrefixSum_.begin(), areaPrefixSum_.end(), u);
    size_t primIdx = upper == areaPrefixSum_.end() ? (areaPrefixSum_.size() - 1)
                                                   : (upper - areaPrefixSum_.begin());
    auto &prim = prims_[primIdx];

    // 在三角形上采样uv

    auto uv = AGZ::Math::DistributionTransform::UniformOnTriangle<Real>::Transform(sample.yz());

    Geometry::SampleResult ret;
    ret.pos = prim.A + uv.u * prim.B_A + uv.v * prim.C_A;
    ret.nor = primsInfo_[primIdx].coordSys.ez;
    ret.pdf = 1 / areaPrefixSum_.back();

    return ret;
}

void TriangleBVHCore::InitBVH(const Vertex *vtx, uint32_t triangleCount)
{
    AGZ_ASSERT(vtx && triangleCount);
    AGZ_ASSERT(!nodes_ && !prims_ && !primsInfo_ && !areaPrefixSum_.size());

    std::vector<BuildTriangle> triangles(triangleCount);
    for(uint32_t i = 0, j = 0; i < triangleCount; ++i, j += 3)
    {
        triangles[i].vtx      = &vtx[j];
        triangles[i].centroid = (vtx[j].pos + vtx[j+1].pos + vtx[j+2].pos) / 3;
    }

    Arena arena;
    auto [root, nodeCount] = BuildBVH(triangles.data(), triangleCount, 5, TVL_STK_SIZE / 2, arena);

    nodes_ = static_cast<TriangleBVHCoreNode*>(AGZ::CRTAllocator::Malloc(
        sizeof(TriangleBVHCoreNode) * nodeCount/*, alignof(TriangleBVHCoreNode)*/));
    prims_ = static_cast<TriangleBVHCorePrimitive*>(AGZ::CRTAllocator::Malloc(
        sizeof(TriangleBVHCorePrimitive) * triangleCount));
    primsInfo_ = static_cast<TriangleBVHCorePrimitiveInfo*>(AGZ::CRTAllocator::Malloc(
        sizeof(TriangleBVHCorePrimitiveInfo) * triangleCount));

    CompactBVH(root, triangles.data(), nodes_, prims_, primsInfo_);

    areaPrefixSum_.resize(triangleCount);
    for(uint32_t i = 0; i < triangleCount; ++i)
        areaPrefixSum_[i] = GetTriangleArea(prims_[i].B_A, prims_[i].C_A);
}

TriangleBVH::TriangleBVH(const Transform &local2World, const TriangleBVHCore &core) noexcept
    : Geometry(local2World), core_(core)
{
    
}

bool TriangleBVH::HasIntersection(const Ray &r) const noexcept
{
    return core_.HasIntersection(local2World_.ApplyInverseToRay(r));
}

bool TriangleBVH::FindIntersection(const Ray &r, GeometryIntersection *inct) const noexcept
{
    if(!core_.FindIntersection(local2World_.ApplyInverseToRay(r), inct))
        return false;
    inct->pos          = local2World_.ApplyToPoint(inct->pos);
    inct->wr           = -r.d;
    inct->coordSys     = local2World_.ApplyToCoordSystem(inct->coordSys);
    inct->usr.coordSys = local2World_.ApplyToCoordSystem(inct->usr.coordSys);
    return true;
}

AABB TriangleBVH::GetLocalBound() const noexcept
{
    return core_.GetLocalBound();
}

Real TriangleBVH::GetSurfaceArea() const noexcept
{
    return local2World_.ScaleFactor() * local2World_.ScaleFactor() * core_.GetSurfaceArea();
}

Geometry::SampleResult TriangleBVH::Sample(const Vec3 &sample) const noexcept
{
    auto sam = core_.Sample(sample);
    sam.pos = local2World_.ApplyToPoint(sam.pos);
    sam.nor = local2World_.ApplyToVector(sam.nor).Normalize();
    sam.pdf = 1 / GetSurfaceArea();
    return sam;
}

Real TriangleBVH::SamplePDF(const Vec3 &pos) const noexcept
{
    return 1 / GetSurfaceArea();
}

} // namespace Atrc
