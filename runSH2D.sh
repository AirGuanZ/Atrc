./Build/SH2D ps ./Scene/SH2D/Bressant.txt
./Build/SH2D pl ./Scene/SH2D/Light.txt
./Build/SH2D rc -m "./Scene/SH2D/a" \
    '
    workspace = "@";
    scene = "Output/bressant.scene"; 
    light = "Output/StudioSmall4K.light";
    outputFilename = "$Output.png";
    rotation = (RotateZ(Deg(0)));
    '
