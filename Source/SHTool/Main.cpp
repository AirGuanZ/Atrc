#include <iostream>

#include "SphericalHarmonics/Projector.h"

using namespace AGZ;
using namespace std;

const char *USAGE_MSG =
R"___(Usage:
    shtool project_entity [entity_desc_filename] (-o [output_filenmae])
    shtool project_light  [light_desc_filename]  (-o [output_filename])
    shtool render_entity  [entity_project_result] [light_project_result] (-o [output_filename])
    shtool render_light   [light_project_result] (-o [output_filename]))___";

void ProjectEntity(const Str8 &descFilename);

int main()
{
    cout << USAGE_MSG << endl;
}

void ProjectEntity(const Str8 &descFilename)
{
    Config configFile;
    if(!configFile.LoadFromFile(descFilename))
    {
        cout << "Failed to load configuration from: " << descFilename.ToStdString() << endl;
        return;
    }


}
