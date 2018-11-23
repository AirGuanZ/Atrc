#include <iostream>

#include "SphericalHarmonics/Projector.h"

using namespace std;

const char *USAGE_MSG =
R"___(Usage:
    shtool project_entity [entity_desc_filename]
    shtool project_light  [light_desc_filename]
    shtool reconstruct    [entity_project_result] [light_project_result])___";

int main()
{
    cout << USAGE_MSG << endl;
}
