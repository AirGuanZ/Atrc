#include <Atrc/Editor/Global.h>
#include <Atrc/Editor/ScreenAxis.h>

namespace
{
    Vec2f Pixel2GL(float pixelX, float pixelY, float screenPixelX, float screenPixelY)
    {
        return Vec2f(2 * pixelX / screenPixelX - 1, 1 - 2 * pixelY / screenPixelY);
    }

} // namespace null

void ScreenAxis::Display(const Mat4f &projViewMat, const GL::Immediate2D &imm)
{
    const float SCREEN_WIDTH = static_cast<float>(Global::GetFramebufferWidth());
    const float SCREEN_HEIGHT = static_cast<float>(Global::GetFramebufferHeight());
    constexpr float SCREEN_AXIS_PIXEL_SIZE = 100;

    constexpr float AXIS_WORLDSPACE_LENGTH = 0.08f;

    Vec3f x = (projViewMat * (AXIS_WORLDSPACE_LENGTH * Vec4f::UNIT_X())).xyz();
    Vec3f y = (projViewMat * (AXIS_WORLDSPACE_LENGTH * Vec4f::UNIT_Y())).xyz();
    Vec3f z = (projViewMat * (AXIS_WORLDSPACE_LENGTH * Vec4f::UNIT_Z())).xyz();

    Vec2f pixelCentre(SCREEN_WIDTH - 30 - SCREEN_AXIS_PIXEL_SIZE / 2, 30 + ImGui::GetFrameHeight() + SCREEN_AXIS_PIXEL_SIZE / 2);
    Vec2f GLCentre = Pixel2GL(pixelCentre.x, pixelCentre.y, SCREEN_WIDTH, SCREEN_HEIGHT);

    Vec2f sx = GLCentre + x.xy();
    Vec2f sy = GLCentre + y.xy();
    Vec2f sz = GLCentre + z.xy();

    imm.DrawLine(GLCentre, sx, AGZ::Math::COLOR::RED);
    imm.DrawLine(GLCentre, sz, AGZ::Math::COLOR::BLUE);
    imm.DrawLine(GLCentre, sy, AGZ::Math::COLOR::GREEN);
}
