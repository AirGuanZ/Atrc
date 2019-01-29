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

    void AddText(ConsoleText::Type type, std::string text);

    void AddMessage(std::string text) { AddText(ConsoleText::Normal, std::move(text)); }
    void AddError(std::string text)   { AddText(ConsoleText::Error,  std::move(text)); }

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
