#include <filesystem>
#include <iostream>
#include <string>

#include <AGZUtils/Utils/Exception.h>
#include <Lib/cxxopts/cxxopts.hpp>

/*
    1. 将.npy格式的环境光球谐系数转换为以(theta, phi)直接映射到uv的环境贴图
    2. 将图像格式的环境光转换为.npy格式的SH系数
*/

void NpySHToNormal(const std::string &npyFilename, int width, int height, const std::string &outputFilename);

void NormalToNpySH(const std::string &imgFilename, int SHOrder, const std::string &outputFilename);

void HDRToNpySH(const std::string &imgFilename, int SHOrder, const std::string &outputFilename);

void Run(int argc, char *argv[])
{
    cxxopts::Options opt("Atrc::EnvLight");
    opt.add_options()
        ("i,input", "input filename", cxxopts::value<std::string>())
        ("o,output", "output filename", cxxopts::value<std::string>())
        ("w,width", "output image width", cxxopts::value<int>())
        ("h,height", "output image height", cxxopts::value<int>())
        ("s,shorder", "SH order (0 to 4)", cxxopts::value<int>())
        ("help", "print help");
    auto optRt = opt.parse(argc, argv);

    if(optRt.count("help"))
    {
        std::cout << opt.help({ "" }) << std::endl;
        return;
    }

    if(!optRt.count("input"))
    {
        std::cout << "input filename is unspecified" << std::endl;
        return;
    }

    if(!optRt.count("output"))
    {
        std::cout << "output filename is unspecified" << std::endl;
        return;
    }

    auto inputFilename  = optRt["input"].as<std::string>();
    auto outputFilename = optRt["output"].as<std::string>();

    auto iExt = std::filesystem::path(inputFilename).extension();
    auto oExt = std::filesystem::path(outputFilename).extension();

    if(iExt == ".npy" && oExt == ".png")
        NpySHToNormal(inputFilename, optRt["width"].as<int>(), optRt["height"].as<int>(), outputFilename);
    else if((iExt == ".jpg" || iExt == ".png") && oExt == ".npy")
        NormalToNpySH(inputFilename, optRt["shorder"].as<int>(), outputFilename);
    else if(iExt == ".hdr" && oExt == ".npy")
        HDRToNpySH(inputFilename, optRt["shorder"].as<int>(), outputFilename);
    else
        std::cout << "Unknown convertion type (" << iExt.string() << " -> " << oExt.string() << std::endl;
}

int main(int argc, char *argv[])
{
    try
    {
        Run(argc, argv);
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
