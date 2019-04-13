#include <filesystem>
#include <iostream>

#include <AGZUtils/Utils/Config.h>
#include <AGZUtils/Utils/Exception.h>
#include <Atrc/SH2D/ProjectLight.h>
#include <Atrc/SH2D/ProjectScene.h>
#include <Atrc/SH2D/ReconstructImage.h>

int main(int argc, char *argv[])
{
    try
    {
        static const char USAGE_MSG[] = R"___(
    SH2D ps/project_scene scene_desc_filename output_dir_name
    SH2D pl/project_light light_desc_filename output_filename
    SH2D rc/reconstruct   SH_order(0 to 4) light_coef_filename scene_coef_dir output_filename [rotateDegree]
)___";

        if(argc < 2)
        {
            std::cout << USAGE_MSG << std::endl;
            return 0;
        }

        if(argv[1] == std::string("ps") || argv[1] == std::string("project_scene"))
        {
            if(argc != 4)
            {
                std::cout << USAGE_MSG << std::endl;
                return 0;
            }

            std::string filename = argv[2];
            std::string outputDir = argv[3];

            AGZ::Config config;
            if(!config.LoadFromFile(filename))
            {
                std::cout << "Failed to load scene description file: " << filename << std::endl;
                return -1;
            }

            if(!std::filesystem::exists(outputDir) && !std::filesystem::create_directories(outputDir))
            {
                std::cout << "Failed to create output directory: " << outputDir << std::endl;
                return -1;
            }

            ProjectScene(config, absolute(std::filesystem::path(filename)).parent_path().string(),
                { outputDir, true, true, true });
        }
        else if(argv[1] == std::string("pl") || argv[1] == std::string("project_light"))
        {
            if(argc != 4)
            {
                std::cout << USAGE_MSG << std::endl;
                return 0;
            }

            std::string filename = argv[2];
            std::string outputFilename = argv[3];
            auto outputDir = std::filesystem::path(outputFilename).parent_path();

            AGZ::Config config;
            if(!config.LoadFromFile(filename))
            {
                std::cout << "Failed to load light description file: " << filename << std::endl;
                return -1;
            }

            if(!exists(outputDir) && !create_directories(outputDir))
            {
                std::cout << "Failed to create output directory: " << outputDir << std::endl;
                return -1;
            }

            ProjectLight(config, absolute(std::filesystem::path(filename)).parent_path().string(), outputFilename);
        }
        else if(argv[1] == std::string("rc") || argv[1] == std::string("reconstruct"))
        {
            if(argc != 6 && argc != 7)
            {
                std::cout << USAGE_MSG << std::endl;
                return 0;
            }

            int SHOrder = std::stoi(argv[2]);
            if(SHOrder < 0 || SHOrder > 4)
            {
                std::cout << USAGE_MSG << std::endl;
                return 0;
            }

            int SHC = (SHOrder + 1) * (SHOrder + 1);
            std::string lightFilename = argv[3];

            std::vector<std::string> sceneCoefFilenames(SHC);
            for(int i = 0; i < SHC; ++i)
                sceneCoefFilenames[i] = (std::filesystem::path(argv[4]) / ("coef" + std::to_string(i) + ".sh2d")).string();

            std::string outputFilename = argv[5];

            std::string albedoFilename = (std::filesystem::path(argv[4]) / "albedo.sh2d").string();

            float rotateDeg = 0;
            if(argc == 7)
                rotateDeg = AGZ::Parse<float>(argv[6]);

            ReconstructImage(SHOrder, lightFilename, sceneCoefFilenames.data(), albedoFilename, outputFilename, rotateDeg);
        }
        else
            std::cout << USAGE_MSG << std::endl;
    }
    catch(const std::exception &err)
    {
        std::vector<std::string> errMsgs;
        AGZ::ExtractHierarchyExceptions(err, std::back_inserter(errMsgs));
        for(auto &m : errMsgs)
            std::cout << m << std::endl;
    }
    catch(...)
    {
        std::cout << "An unknown error occurred..." << std::endl;
    }
}
