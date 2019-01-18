#include "Console.h"
#include "GL.h"

Console::Console(int inputBufSize, int maxTextCount)
{
    AGZ_ASSERT(maxTextCount >= 0 && inputBufSize > 1);

    maxTextCount_ = maxTextCount;
    inputBuf_.resize(inputBufSize);
    inputBuf_[0] = '\0';

    scrollToBottom_ = true;
}

void Console::Display()
{
    ImGui::SetNextWindowPos(ImVec2(400, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);

    if(!ImGui::Begin("Console##ModelViewer"))
    {
        ImGui::End();
        return;
    }

    //留一个separator+一行text的空间给输入槽
    const float reservedFooterHeight = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -reservedFooterHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

    for(auto &t : texts_)
    {
        static const ImVec4 TEXT_COLOR[] =
        {
            { 1.0f, 1.0f, 1.0f, 1.0f }, //Normal
            { 1.0f, 0.4f, 0.4f, 1.0f }, //Error
        };
        ImGui::PushStyleColor(ImGuiCol_Text, TEXT_COLOR[static_cast<int>(t.type)]);
        ImGui::TextWrapped("%s", ((t.type == ConsoleText::Error ? "[err] " : "[out] ") + t.text).c_str());
        ImGui::PopStyleColor();
    }

    if(scrollToBottom_)
    {
        ImGui::SetScrollHere();
        scrollToBottom_ = false;
    }

    ImGui::EndChild();

    ImGui::Separator();

    bool inputTextEntered = ImGui::InputText("Input", inputBuf_.data(), inputBuf_.size(),
        ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SameLine();
    inputTextEntered |= ImGui::Button("Enter");

    if(inputTextEntered && inputBuf_[0] != '\0')
    {
        inputTexts_.push(inputBuf_.data());
        inputBuf_[0] = '\0';
    }

    ImGui::SameLine();
    if(ImGui::Button("Clear"))
        texts_.clear();

    ImGui::End();
}

void Console::SetMaxTextCount(int count)
{
    assert(count >= 0);
    maxTextCount_ = count;
    DeleteRedundantTexts();
}

void Console::AddText(ConsoleText::Type type, const std::string &text)
{
    texts_.push_back({ type, text });
    DeleteRedundantTexts();
    scrollToBottom_ = true;
}

void Console::ClearTexts()
{
    texts_.clear();
}

void Console::ClearInputBox()
{
    inputBuf_[0] = '\0';
}

void Console::ClearInputQueue()
{
    while(!inputTexts_.empty())
        inputTexts_.pop();
}

bool Console::FetchInputText(std::string &text)
{
    if(!inputTexts_.empty())
    {
        text = std::move(inputTexts_.front());
        inputTexts_.pop();
        return true;
    }
    return false;
}

void Console::DeleteRedundantTexts()
{
    while(texts_.size() > static_cast<size_t>(maxTextCount_))
        texts_.pop_front();
}
