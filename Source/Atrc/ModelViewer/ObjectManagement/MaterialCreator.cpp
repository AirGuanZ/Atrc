#include <Atrc/ModelViewer/ObjectManagement/MaterialCreator.h>

namespace
{
    class IdealBlackInstance : public MaterialInstance
    {
    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager&) override { }
    };

    class IdealDiffuseInstance : public MaterialInstance
    {
        TextureSlot albedo_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager &objMgr) override
        {
            if(ImGui::TreeNode("albedo"))
            {
                albedo_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };

    class IdealMirrorInstance : public MaterialInstance
    {
        TextureSlot rc_;
        FresnelSlot fresnel_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager &objMgr) override
        {
            if(ImGui::TreeNode("rc"))
            {
                rc_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("fresnel"))
            {
                fresnel_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };

    class IdealScalerInstance : public MaterialInstance
    {
        TextureSlot scale_;
        MaterialSlot internal_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager &objMgr) override
        {
            if(ImGui::TreeNode("scale"))
            {
                scale_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("internal"))
            {
                internal_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };

    class IdealSpecularInstance : public MaterialInstance
    {
        TextureSlot rc_;
        FresnelSlot dielectric_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager &objMgr) override
        {
            if(ImGui::TreeNode("rc"))
            {
                rc_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("dielectric"))
            {
                dielectric_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };

    class InvisibleSurfaceInstance : public MaterialInstance
    {
    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager&) override { }
    };

    class NormalizedDiffusionBSSRDFInstance : public MaterialInstance
    {
        MaterialSlot surface_;
        TextureSlot A_;
        TextureSlot dmfp_;
        float eta_ = 0;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager &objMgr) override
        {
            ImGui::InputFloat("eta", &eta_);
            if(ImGui::TreeNode("surface"))
            {
                surface_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("A"))
            {
                A_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("dmfp"))
            {
                dmfp_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };

    class ONMatteInstance : public MaterialInstance
    {
        TextureSlot albedo_;
        TextureSlot sigma_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager &objMgr) override
        {
            if(ImGui::TreeNode("albedo"))
            {
                albedo_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("sigma"))
            {
                sigma_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };

    class TSMetalInstance : public MaterialInstance
    {
        TextureSlot rc_;
        TextureSlot roughness_;
        FresnelSlot fresnel_;

    public:

        using MaterialInstance::MaterialInstance;

        void Display(ObjectManager &objMgr) override
        {
            if(ImGui::TreeNode("rc"))
            {
                rc_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("roughness"))
            {
                roughness_.Display(objMgr);
                ImGui::TreePop();
            }
            if(ImGui::TreeNode("fresnel"))
            {
                fresnel_.Display(objMgr);
                ImGui::TreePop();
            }
        }
    };
}

void RegisterMaterialCreators(ObjectManager &objMgr)
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
    objMgr.AddCreator(&iIdealBlackCreator);
    objMgr.AddCreator(&iIdealDiffuseCreator);
    objMgr.AddCreator(&iIdealMirrorCreator);
    objMgr.AddCreator(&iIdealScalerCreator);
    objMgr.AddCreator(&iIdealSpecularCreator);
    objMgr.AddCreator(&iInvisibleSurfaceCreator);
    objMgr.AddCreator(&iNormalizedDiffusionBSSRDFCreator);
    objMgr.AddCreator(&iONMatteCreator);
    objMgr.AddCreator(&iTSMetalCreator);
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
