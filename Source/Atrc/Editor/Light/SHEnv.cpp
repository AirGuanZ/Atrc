#include <Atrc/Core/Utility/ConfigConvert.h>
#include <Atrc/Editor/Light/SHEnv.h>
#include "Atrc/Editor/Global.h"

namespace Atrc::Editor
{
    
std::string SHEnv::Save(const std::filesystem::path &relPath) const
{
    static const AGZ::Fmt fmt(
        "type = {};"
        "SHOrder = {};"
        "coefs = ({});"
    );
    std::vector<std::string> coefsStr;
    for(auto &s : coefs_)
        coefsStr.push_back(Vec3fToCS(s));
    return Wrap(fmt.Arg(
        GetType(), SHOrder_,
        AGZ::Join(",", coefsStr.begin(), coefsStr.end())));
}

void SHEnv::Load(const AGZ::ConfigGroup &params, const std::filesystem::path &relPath)
{
    AGZ_HIERARCHY_TRY

    SHOrder_ = params["SHOrder"].Parse<int>();
    if(SHOrder_ < 1 || SHOrder_ > 5)
        throw Exception("invalid SHOrder value: " + std::to_string(SHOrder_));

    int SHC = SHOrder_ * SHOrder_;
    coefs_.resize(SHC);
    auto &coefArr = params["coefs"].AsArray();
    for(int i = 0; i < SHC; ++i)
        coefs_[i] = Node2Vec3f(*coefArr.At(i));

    AGZ_HIERARCHY_WRAP("in loading sh env with " + params.ToString())
}

std::string SHEnv::Export(const std::filesystem::path &path) const
{
    return Save(path);
}

void SHEnv::Display()
{
    if(ImGui::Button("load from file"))
    {
        fileBrowser_.SetTitle("coefs file");
        fileBrowser_.SetPwd();
        fileBrowser_.ClearSelected();
        fileBrowser_.Open();
    }

    fileBrowser_.Display();

    try
    {
        if(fileBrowser_.HasSelected())
        {
            auto filename = fileBrowser_.GetSelected();
            fileBrowser_.ClearSelected();

            std::ifstream fin(filename);
            if(!fin)
                throw Exception("failed to open file " + filename.string());

            int SHOrder;
            fin >> SHOrder;
            ++SHOrder;

            if(!fin || SHOrder_ < 1 || SHOrder_ > 5)
                throw Exception("invalid SHOrder value: " + std::to_string(SHOrder));

            std::vector<Spectrum> coefs;
            int SHC = SHOrder * SHOrder;
            for(int i = 0; i < SHC; ++i)
            {
                Spectrum s;
                fin >> s.r;
                fin >> s.g;
                fin >> s.b;
                coefs.push_back(s);
            }

            if(!fin)
                throw Exception("failed to load sh coefs from file");

            SHOrder_ = SHOrder;
            coefs_ = std::move(coefs);
        }
    }
    catch(const std::exception &err)
    {
        Global::ShowErrorMessage(err.what());
        SHOrder_ = 1;
        coefs_ = { Spectrum() };
    }

    if(ImGui::SliderInt("SHOrder", &SHOrder_, 1, 5))
        coefs_.resize(SHOrder_ * SHOrder_);
    int SHC = SHOrder_ * SHOrder_;
    for(int i = 0; i < SHC; ++i)
    {
        ImGui::PushID(i);
        ImGui::InputFloat3("", &coefs_[i][0]);
        ImGui::PopID();
    }
}

bool SHEnv::IsMultiline() const noexcept
{
    return true;
}

} // namespace Atrc::Editor
