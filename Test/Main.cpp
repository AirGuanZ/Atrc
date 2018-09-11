#ifdef ATRC_UNIT_TEST

#include <locale>

#define CATCH_CONFIG_RUNNER
#include "Catch.hpp"

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char* argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)
        | _CRTDBG_LEAK_CHECK_DF);
#endif

    std::locale::global(std::locale(""));

    return Catch::Session().run(argc, argv);
}

#define AGZ_ALL_IMPL

#include <Utils.h>

#endif
