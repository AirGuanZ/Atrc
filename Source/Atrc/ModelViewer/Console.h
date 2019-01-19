#pragma once

#include <list>
#include <queue>

#include <AGZUtils/Utils/String.h>

struct ConsoleText
{
    enum Type
    {
        Normal,
        Error
    };
    Type type = Normal;
    std::string text;
};

class Console
{
public:

    explicit Console(int inputBufSize = 256, int maxTextCount = 256);

    void Display();

    void SetMaxTextCount(int count);

    void AddText(ConsoleText::Type type, const std::string &text);
    void AddText(ConsoleText::Type type, const AGZ::Str8 &text) { AddText(type, text.ToStdString()); }
    void AddText(ConsoleText::Type type, const char *text)      { AddText(type, std::string(text)); }

    void ClearTexts();
    void ClearInputBox();
    void ClearInputQueue();

    bool FetchInputText(std::string &text);

private:

    void DeleteRedundantTexts();

    int maxTextCount_;
    std::list<ConsoleText> texts_;

    std::vector<char> inputBuf_;
    std::queue<std::string> inputTexts_;

    bool scrollToBottom_;
};
