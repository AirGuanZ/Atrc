#include <AGZUtils/Container/SharedPtrPool.h>
#include <Atrc/Editor/Texture/HDR.h>
#include <Atrc/Editor/Global.h>

bool HDR::SetGLTextureFilename(const std::filesystem::path &filename)
{
    struct V2K
    {
        const std::filesystem::path &operator()(const GLTextureWithFilename &tex) noexcept
        {
            return tex.filename;
        }
    };
    static AGZ::SharedPtrPool<std::filesystem::path, GLTextureWithFilename, V2K> pool;

    glTex_ = pool.GetOrAdd(filename, [&]() -> GLTextureWithFilename*
    {
        try
        {
            auto data = AGZ::TextureFile::LoadRGBFromHDR(filename.string()).Map(
                [](const AGZ::Math::Color3f &c)
            {
                return c.Map([](float f)
                { return static_cast<uint8_t>(AGZ::Math::Clamp(f, 0.0f, 1.0f) * 255); });
            });
            auto ret = new GLTextureWithFilename;
            ret->filename = filename;
            ret->tex.InitializeHandle();
            ret->tex.InitializeFormatAndData(
                1, data.GetWidth(), data.GetHeight(),
                GL_RGB8, data.RawData());
            ret->WOverH = static_cast<float>(data.GetWidth()) / data.GetHeight();
            return ret;
        }
        catch(...)
        {
            return nullptr;
        }
    });

    return glTex_ != nullptr;
}

std::string HDR::Save(const std::filesystem::path &relPath) const
{
    AGZ_HIERARCHY_TRY

    if(!glTex_)
        throw std::runtime_error("empty hdr object");

    const AGZ::Fmt fmt(R"___(
        type = {};
        filename = {};
    )___");
    return Wrap(fmt.Arg(GetType(), RelPath(fileSelector_.GetFilename(), relPath).string()));

    AGZ_HIERARCHY_WRAP("in saving hdr texture")
}

void HDR::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    auto filename = relative(relPath / params["filename"].AsValue());
    if(SetGLTextureFilename(filename))
        fileSelector_.SetFilename(filename);
    else
    {
        fileSelector_.SetFilename("");
        throw std::runtime_error("failed to set gl texture filename to " + filename.string());
    }

    AGZ_HIERARCHY_WRAP("in loading hdr texture with " + params.ToString())
}

std::string HDR::Export(const std::filesystem::path &relPath) const
{
    return Save(relPath);
}

void HDR::Display()
{
    if(fileSelector_.Display())
    {
        auto filename = fileSelector_.GetFilename();
        if(!SetGLTextureFilename(filename))
        {
            Global::ShowNormalMessage("failed to load gl texture from " + filename.string());
            fileSelector_.SetFilename("");
        }
    }

    if(ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", fileSelector_.GetFilename().c_str());
        ImGui::Image(ImTextureID(size_t(glTex_->tex.GetHandle())), ImVec2(200 * glTex_->WOverH, 200));
        ImGui::EndTooltip();
    }
}

bool HDR::IsMultiline() const noexcept
{
    return false;
}
