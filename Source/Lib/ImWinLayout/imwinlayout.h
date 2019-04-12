#pragma once

#ifndef IMGUI_VERSION
#   error "include imgui.h before this header"
#endif

#include <functional>
#include <memory>
#include <variant>
#include <vector>

enum ImWindowLayoutSizePolicy_
{
    ImWindowLayoutSizePolicy_Ranged,
    ImWindowLayoutSizePolicy_Fixed,
    ImWindowLayoutSizePolicy_Auto,
    ImWindowLayoutSizePolicy_Depending
};

enum ImWindowLayoutPositionPolicy_
{
    ImWindowLayoutPositionPolicy_Left,
    ImWindowLayoutPositionPolicy_Right,
    ImWindowLayoutPositionPolicy_Center,
};

namespace ImGui
{
    class HBoxLayout;
    class VBoxLayout;
    class WindowElem;

    namespace WL
    {
        class Uncopyable
        {
        public:

            Uncopyable() = default;
            Uncopyable(const Uncopyable&) = delete;
            Uncopyable &operator=(const Uncopyable&) = delete;
        };

        struct SP_Auto
        {

        };

        struct SP_Ranged
        {
            int minPixel, maxPixel;
        };

        struct SP_Fixed
        {
            int pixel;
        };

        struct SP_Depending
        {
            bool useAbsIndex;
            int index;
            std::function<int(int)> depFunc;
        };

        using SizePolicy = std::variant<SP_Auto, SP_Ranged, SP_Fixed, SP_Depending>;
        using LayoutItemContent = std::variant<HBoxLayout, VBoxLayout, WindowElem>;

        struct LayoutItem
        {
            SizePolicy sizePolicy;
            LayoutItemContent content;
        };
    }

    class BoxLayout : public WL::Uncopyable
    {
    public:

        virtual ~BoxLayout() = default;

        virtual float SizeInParentX() const = 0;
        virtual float SizeInParentY() const = 0;

        virtual HBoxLayout *AddHBox_Low()  = 0;
        virtual HBoxLayout *AddHBox_High() = 0;
        virtual VBoxLayout *AddVBox_Low()  = 0;
        virtual VBoxLayout *AddVBox_High() = 0;
    };

    template<bool THori>
    class BoxLayoutImpl : public BoxLayout
    {
        std::vector<std::unique_ptr<WL::LayoutItem>> lowElems_;
        std::vector<std::unique_ptr<WL::LayoutItem>> highElems_;

    public:

        float SizeInParentX() const override;
        float SizeInParentY() const override;

        HBoxLayout *AddHBox_Low()  override;
        HBoxLayout *AddHBox_High() override;
        VBoxLayout *AddVBox_Low()  override;
        VBoxLayout *AddVBox_High() override;
    };
}
