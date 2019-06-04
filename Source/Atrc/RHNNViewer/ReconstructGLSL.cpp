const char *RECONSTRUCT_VS_GLSL = R"___(
#version 450 core

in vec2 iScreenPos;
in vec2 iTexCoord;
out vec2 mTexCoord;

void main()
{
    gl_Position = vec4(iScreenPos, 0.5, 1);
    mTexCoord   = iTexCoord;
}
)___";

const char *RECONSTRUCT_FS_GLSL = R"___(
#version 450 core

uniform sampler2D mSceneCoefs[9];
uniform sampler2D mAlbedo;
uniform vec3      mLightCoefs[9];

uniform int mWithAlbedo;

in vec2 mTexCoord;
out vec4 oColor;

void main()
{
    vec3 color = vec3(0, 0, 0);
    for(int i = 0; i < 9; ++i)
    {
        vec3 sceneCoef = texture(mSceneCoefs[i], mTexCoord).rrr;
        color += mLightCoefs[i] * sceneCoef;
    }
    vec3 albedo = mWithAlbedo != 0 ? texture(mAlbedo, mTexCoord).rgb : vec3(1);
    oColor = vec4(clamp(1.4 * albedo * color, 0, 1), 1);
}
)___";
