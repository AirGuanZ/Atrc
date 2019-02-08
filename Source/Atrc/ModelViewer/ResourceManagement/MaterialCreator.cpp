#include <Atrc/ModelViewer/ResourceManagement/MaterialCreator.h>

namespace
{
    class IdealBlackInstance : public MaterialInstance
    {
    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager&) override { }

        void Export(std::stringstream &sst, const ResourceManager&, ExportingContext &ctx) const override
        {
            sst << AGZ::TFormatter<char>("{}type = IdealBlack;\n").Arg(ctx.Indent());
        }
    };

    class IdealDiffuseInstance : public MaterialInstance
    {
        TextureSlot albedo_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager &rscMgr) override
        {
            if(ImGui::TreeNode("albedo"))
            {
                albedo_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = IdealDiffuse;\n";
            ExportSubResource("albedo", sst, rscMgr, ctx, albedo_);
        }
    };

    class IdealMirrorInstance : public MaterialInstance
    {
        TextureSlot rc_;
        FresnelSlot fresnel_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager &rscMgr) override
        {
            if(ImGui::TreeNode("rc"))
            {
                rc_.Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("fresnel"))
            {
                fresnel_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = IdealMirror;\n";
            ExportSubResource("rc", sst, rscMgr, ctx, rc_);
            ExportSubResource("fresnel", sst, rscMgr, ctx, fresnel_);
        }
    };

    class IdealScalerInstance : public MaterialInstance
    {
        TextureSlot scale_;
        MaterialSlot internal_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager &rscMgr) override
        {
            if(ImGui::TreeNode("scale"))
            {
                scale_.Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("internal"))
            {
                internal_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = IdealScaler;\n";
            ExportSubResource("scale", sst, rscMgr, ctx, scale_);
            ExportSubResource("internal", sst, rscMgr, ctx, internal_);
        }
    };

    class IdealSpecularInstance : public MaterialInstance
    {
        TextureSlot rc_;
        FresnelSlot dielectric_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager &rscMgr) override
        {
            if(ImGui::TreeNode("rc"))
            {
                rc_.Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("dielectric"))
            {
                dielectric_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = IdealSpecular;\n";
            ExportSubResource("rc", sst, rscMgr, ctx, rc_);
            ExportSubResource("fresnel", sst, rscMgr, ctx, dielectric_);
        }
    };

    class InvisibleSurfaceInstance : public MaterialInstance
    {
    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager&) override { }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = Invisible;\n";
        }
    };

    class NormalizedDiffusionBSSRDFInstance : public MaterialInstance
    {
        MaterialSlot surface_;
        TextureSlot A_;
        TextureSlot dmfp_;
        float eta_ = 0;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager &rscMgr) override
        {
            ImGui::InputFloat("eta", &eta_);
            if(ImGui::TreeNode("surface"))
            {
                surface_.Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("A"))
            {
                A_.Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("dmfp"))
            {
                dmfp_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = BSSRDF;\n";
            ExportSubResource("surface", sst, rscMgr, ctx, surface_);
            ExportSubResource("A", sst, rscMgr, ctx, A_);
            ExportSubResource("dmfp", sst, rscMgr, ctx, dmfp_);
            sst << ctx.Indent() << AGZ::TFormatter<char>("eta = {};\n").Arg(eta_);
        }
    };

    class ONMatteInstance : public MaterialInstance
    {
        TextureSlot albedo_;
        TextureSlot sigma_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager &rscMgr) override
        {
            if(ImGui::TreeNode("albedo"))
            {
                albedo_.Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("sigma"))
            {
                sigma_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = ONMatte;\n";
            ExportSubResource("albedo", sst, rscMgr, ctx, albedo_);
            ExportSubResource("sigma", sst, rscMgr, ctx, sigma_);
        }
    };

    class TSMetalInstance : public MaterialInstance
    {
        TextureSlot rc_;
        TextureSlot roughness_;
        FresnelSlot fresnel_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager &rscMgr) override
        {
            if(ImGui::TreeNode("rc"))
            {
                rc_.Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("roughness"))
            {
                roughness_.Display(rscMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("fresnel"))
            {
                fresnel_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Export(std::stringstream &sst, const ResourceManager &rscMgr, ExportingContext &ctx) const override
        {
            sst << ctx.Indent() << "type = TSMetal;\n";
            ExportSubResource("rc", sst, rscMgr, ctx, rc_);
            ExportSubResource("roughness", sst, rscMgr, ctx, roughness_);
            ExportSubResource("fresnel", sst, rscMgr, ctx, fresnel_);
        }
    };
}

void RegisterMaterialCreators(ResourceManager &rscMgr)
{
    static const IdealBlackCreator iIdealBlackCreator;
    static const IdealDiffuseCreator iIdealDiffuseCreator;
    static const IdealMirrorCreator iIdealMirrorCreator;
    static const IdealScalerCreator iIdealScalerCreator;
    static const IdealSpecularCreator iIdealSpecularCreator;
    static const InvisibleSurfaceCreator iInvisibleSurfaceCreator;
    static const NormalizedDiffusionBSSRDFCreator iNormalizedDiffusionBSSRDFCreator;
    static const ONMatteCreator iONMatteCreator;
    static const TSMetalCreator iTSMetalCreator;
    rscMgr.AddCreator(&iIdealBlackCreator);
    rscMgr.AddCreator(&iIdealDiffuseCreator);
    rscMgr.AddCreator(&iIdealMirrorCreator);
    rscMgr.AddCreator(&iIdealScalerCreator);
    rscMgr.AddCreator(&iIdealSpecularCreator);
    rscMgr.AddCreator(&iInvisibleSurfaceCreator);
    rscMgr.AddCreator(&iNormalizedDiffusionBSSRDFCreator);
    rscMgr.AddCreator(&iONMatteCreator);
    rscMgr.AddCreator(&iTSMetalCreator);
}

std::shared_ptr<MaterialInstance> IdealBlackCreator::Create(std::string name) const
{
    return std::make_shared<IdealBlackInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> IdealDiffuseCreator::Create(std::string name) const
{
    return std::make_shared<IdealDiffuseInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> IdealMirrorCreator::Create(std::string name) const
{
    return std::make_shared<IdealMirrorInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> IdealScalerCreator::Create(std::string name) const
{
    return std::make_shared<IdealScalerInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> IdealSpecularCreator::Create(std::string name) const
{
    return std::make_shared<IdealSpecularInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> InvisibleSurfaceCreator::Create(std::string name) const
{
    return std::make_shared<InvisibleSurfaceInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> NormalizedDiffusionBSSRDFCreator::Create(std::string name) const
{
    return std::make_shared<NormalizedDiffusionBSSRDFInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> ONMatteCreator::Create(std::string name) const
{
    return std::make_shared<ONMatteInstance>(std::move(name));
}

std::shared_ptr<MaterialInstance> TSMetalCreator::Create(std::string name) const
{
    return std::make_shared<TSMetalInstance>(std::move(name));
}
