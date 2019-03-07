#pragma once

#include <memory>

class UniqueQDeleter final
{
public:

    template<typename T>
    void operator()(T *w) const
    {
        if(!w->parentWidget())
            delete w;
    }
};

template<typename TQObj>
using UniqueQPtr = std::unique_ptr<TQObj, UniqueQDeleter>;

template<typename TQObj, typename...Args>
UniqueQPtr<TQObj> MakeUniqueQ(Args&&...args)
{
    return std::unique_ptr<TQObj, UniqueQDeleter>(new TQObj(std::forward<Args>(args)...));
}
