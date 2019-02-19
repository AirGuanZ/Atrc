#include <Atrc/Editor/ResourceManagement/MaterialCreator.h>
#include <Atrc/Mgr/Parser.h>

namespace
{
    class GGXDielectricInstance : public MaterialInstance
    {
        TextureSlot rc_;
        TextureSlot roughness_;
        FresnelSlot fresnel_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = GGXDielectric;");
            ExportSubResource("rc", rscMgr, ctx, rc_);
            ExportSubResource("roughness", rscMgr, ctx, roughness_);
            ExportSubResource("fresnel", rscMgr, ctx, fresnel_);
        }

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

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            rc_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["rc"], ctx));
            roughness_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["roughness"], ctx));
            fresnel_.SetInstance(GetResourceInstance<FresnelInstance>(rscMgr, root, params["fresnel"], ctx));
        }
    };

    class GGXMetalInstance : public MaterialInstance
    {
        TextureSlot rc_;
        TextureSlot roughness_;
        FresnelSlot fresnel_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = GGXMetal;");
            ExportSubResource("rc", rscMgr, ctx, rc_);
            ExportSubResource("roughness", rscMgr, ctx, roughness_);
            ExportSubResource("fresnel", rscMgr, ctx, fresnel_);
        }

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

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            rc_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["rc"], ctx));
            roughness_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["roughness"], ctx));
            fresnel_.SetInstance(GetResourceInstance<FresnelInstance>(rscMgr, root, params["fresnel"], ctx));
        }
    };

    class IdealBlackInstance : public MaterialInstance
    {
    protected:

        void Export(const ResourceManager&, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = IdealBlack;");
        }

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager&) override { }
    };

    class IdealDiffuseInstance : public MaterialInstance
    {
        TextureSlot albedo_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = IdealDiffuse;");
            ExportSubResource("albedo", rscMgr, ctx, albedo_);
        }

    public:

        IdealDiffuseInstance(ResourceManager &rscMgr, std::string typeName, std::string name)
            : MaterialInstance(std::move(typeName), std::move(name))
        {
            albedo_.SetInstance(rscMgr.Create<TextureInstance>("Constant", ""));
        }

        void Display(ResourceManager &rscMgr) override
        {
            if(ImGui::TreeNodeEx("albedo", ImGuiTreeNodeFlags_DefaultOpen))
            {
                albedo_.Display(rscMgr);
                ImGui::TreePop();
            }
        }

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            albedo_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["albedo"], ctx));
        }
    };

    class IdealMirrorInstance : public MaterialInstance
    {
        TextureSlot rc_;
        FresnelSlot fresnel_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = IdealMirror;");
            ExportSubResource("rc", rscMgr, ctx, rc_);
            ExportSubResource("fresnel", rscMgr, ctx, fresnel_);
        }

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

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            rc_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["rc"], ctx));
            fresnel_.SetInstance(GetResourceInstance<FresnelInstance>(rscMgr, root, params["fresnel"], ctx));
        }
    };

    class IdealScalerInstance : public MaterialInstance
    {
        TextureSlot scale_;
        MaterialSlot internal_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = IdealScaler;");
            ExportSubResource("scale", rscMgr, ctx, scale_);
            ExportSubResource("internal", rscMgr, ctx, internal_);
        }

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


        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            scale_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["scale"], ctx));
            internal_.SetInstance(GetResourceInstance<MaterialInstance>(rscMgr, root, params["internal"], ctx));
        }
    };

    class IdealSpecularInstance : public MaterialInstance
    {
        TextureSlot rc_;
        FresnelSlot dielectric_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = IdealSpecular;");
            ExportSubResource("rc", rscMgr, ctx, rc_);
            ExportSubResource("fresnel", rscMgr, ctx, dielectric_);
        }

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

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            rc_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["rc"], ctx));
            dielectric_.SetInstance(GetResourceInstance<FresnelInstance>(rscMgr, root, params["fresnel"], ctx));
        }
    };

    class InvisibleSurfaceInstance : public MaterialInstance
    {
    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = Invisible;");
        }

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ResourceManager&) override { }
    };

    class NormalizedDiffusionBSSRDFInstance : public MaterialInstance
    {
        MaterialSlot surface_;
        TextureSlot A_;
        TextureSlot dmfp_;
        float eta_ = 0;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = BSSRDF;");
            ExportSubResource("surface", rscMgr, ctx, surface_);
            ExportSubResource("A", rscMgr, ctx, A_);
            ExportSubResource("dmfp", rscMgr, ctx, dmfp_);
            ctx.AddLine("eta = ", std::to_string(eta_), ";");
        }

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

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            surface_.SetInstance(GetResourceInstance<MaterialInstance>(rscMgr, root, params["surface"], ctx));
            A_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["A"], ctx));
            dmfp_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params, ctx));
            eta_ = params["eta"].Parse<float>();
        }
    };

    class ONMatteInstance : public MaterialInstance
    {
        TextureSlot albedo_;
        TextureSlot sigma_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = ONMatte;");
            ExportSubResource("albedo", rscMgr, ctx, albedo_);
            ExportSubResource("sigma", rscMgr, ctx, sigma_);
        }

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

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            albedo_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["albedo"], ctx));
            sigma_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["sigma"], ctx));
        }
    };

    class TSMetalInstance : public MaterialInstance
    {
        TextureSlot rc_;
        TextureSlot roughness_;
        FresnelSlot fresnel_;

    protected:

        void Export(const ResourceManager &rscMgr, LauncherScriptExportingContext &ctx) const override
        {
            ctx.AddLine("type = TSMetal;");
            ExportSubResource("rc", rscMgr, ctx, rc_);
            ExportSubResource("roughness", rscMgr, ctx, roughness_);
            ExportSubResource("fresnel", rscMgr, ctx, fresnel_);
        }

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

        void Import(ResourceManager &rscMgr, const AGZ::ConfigGroup &root, const AGZ::ConfigGroup &params, const ImportContext &ctx) override
        {
            rc_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["rc"], ctx));
            roughness_.SetInstance(GetResourceInstance<TextureInstance>(rscMgr, root, params["roughness"], ctx));
            fresnel_.SetInstance(GetResourceInstance<FresnelInstance>(rscMgr, root, params["fresnel"], ctx));
        }
    };
}

void RegisterMaterialCreators(ResourceManager &rscMgr)
{
    static const GGXDielectricCreator iGGXDielectricCreator;
    static const GGXMetalCreator iGGXMetalCreator;
    static const IdealBlackCreator iIdealBlackCreator;
    static const IdealDiffuseCreator iIdealDiffuseCreator;
    static const IdealMirrorCreator iIdealMirrorCreator;
    static const IdealScalerCreator iIdealScalerCreator;
    static const IdealSpecularCreator iIdealSpecularCreator;
    static const InvisibleSurfaceCreator iInvisibleSurfaceCreator;
    static const NormalizedDiffusionBSSRDFCreator iNormalizedDiffusionBSSRDFCreator;
    static const ONMatteCreator iONMatteCreator;
    static const TSMetalCreator iTSMetalCreator;
    rscMgr.AddCreator(&iGGXDielectricCreator);
    rscMgr.AddCreator(&iGGXMetalCreator);
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

std::shared_ptr<MaterialInstance> GGXDielectricCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<GGXDielectricInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> GGXMetalCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<GGXMetalInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> IdealBlackCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<IdealBlackInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> IdealDiffuseCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<IdealDiffuseInstance>(rscMgr, GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> IdealMirrorCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<IdealMirrorInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> IdealScalerCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<IdealScalerInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> IdealSpecularCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<IdealSpecularInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> InvisibleSurfaceCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<InvisibleSurfaceInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> NormalizedDiffusionBSSRDFCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<NormalizedDiffusionBSSRDFInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> ONMatteCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<ONMatteInstance>(GetName(), std::move(name));
}

std::shared_ptr<MaterialInstance> TSMetalCreator::Create(ResourceManager &rscMgr, std::string name) const
{
    return std::make_shared<TSMetalInstance>(GetName(), std::move(name));
}
