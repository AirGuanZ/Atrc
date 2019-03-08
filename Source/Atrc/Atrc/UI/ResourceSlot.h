#pragma once

#include <type_traits>
#include <QComboBox>
#include <QVBoxLayout>

#include <Atrc/Atrc/ResourceInterface/ResourceInstance.h>

class ResourceSlotWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ResourceSlotWidget(const std::vector<std::string> &names)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        creatorSelector_ = MakeUniqueQ<QComboBox>();
        creatorSelector_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        for(size_t i = 0; i < names.size(); ++i)
            creatorSelector_->addItem(QString::fromStdString(names[i]), i);

        vLayout_ = MakeUniqueQ<QVBoxLayout>(this);
        hLayout_ = MakeUniqueQ<QHBoxLayout>();

        vLayout_->addLayout(hLayout_.get());
        hLayout_->addWidget(creatorSelector_.get());

        hWidget_ = MakeUniqueQ<QWidget>(this);
        hwLayout_ = MakeUniqueQ<QHBoxLayout>();
        hWidget_->setLayout(hwLayout_.get());
        
        hLayout_->addWidget(hWidget_.get());

        connect(creatorSelector_.get(), &QComboBox::currentTextChanged, this, [=](const QString &text)
        {
            onSelectedTextChanged_(text.toStdString());
        });
    }

    std::string GetSelectedCreatorName() const
    {
        return creatorSelector_->currentText().toStdString();
    }

    QLayout *GetLayout() noexcept
    {
        return vLayout_.get();
    }

    QWidget *GetWidget() noexcept
    {
        if(hwLayout_->count() > 0)
            return hwLayout_->itemAt(0)->widget();
        if(vLayout_->count() > 1)
            return vLayout_->itemAt(1)->widget();
        return nullptr;
    }

    void SetWidget(QWidget *widget, bool multiline)
    {
        auto oldWidget = GetWidget();
        if(hwLayout_->count() > 0)
            hwLayout_->removeWidget(hwLayout_->itemAt(0)->widget());
        if(vLayout_->count() > 1)
            vLayout_->removeWidget(vLayout_->itemAt(1)->widget());
        if(oldWidget)
        {
            AGZ_ASSERT(oldWidget->parentWidget() == vLayout_->parentWidget() || oldWidget->parentWidget() == hWidget_.get());
            oldWidget->setParent(nullptr);
        }
        if(!widget)
            return;

        if(multiline)
            vLayout_->addWidget(widget);
        else
            hwLayout_->addWidget(widget);
    }

    void SetCallbackOnSelectedTextChanged(std::function<void(const std::string&)> func)
    {
        onSelectedTextChanged_ = std::move(func);
    }

private:

    std::function<void(const std::string&)> onSelectedTextChanged_;

    UniqueQPtr<QComboBox> creatorSelector_;
    UniqueQPtr<QVBoxLayout> vLayout_;
    UniqueQPtr<QHBoxLayout> hLayout_;

    UniqueQPtr<QWidget> hWidget_;
    UniqueQPtr<QHBoxLayout> hwLayout_;
};

template<typename TResourceInstance>
class ResourceSlot
{
    const ResourceInstanceCreatorManager<TResourceInstance> &creatorMgr_;
    UniqueQPtr<ResourceSlotWidget> widget_;
    std::unique_ptr<TResourceInstance> curRsc_;

public:

    explicit ResourceSlot(const ResourceInstanceCreatorManager<TResourceInstance> &creatorMgr)
        : creatorMgr_(creatorMgr)
    {
        std::vector<std::string> names;
        for(auto &it : creatorMgr)
            names.push_back(it.first);
        widget_ = MakeUniqueQ<ResourceSlotWidget>(names);

        auto curName = widget_->GetSelectedCreatorName();
        curRsc_.reset(creatorMgr_[curName]->Create());
        widget_->SetWidget(curRsc_->GetWidget(), curRsc_->IsMultiline());

        widget_->SetCallbackOnSelectedTextChanged([=](const std::string &name)
        {
            std::unique_ptr<TResourceInstance> oldRsc = std::move(curRsc_);
            curRsc_.reset(creatorMgr[name]->Create());
            widget_->SetWidget(curRsc_->GetWidget(), curRsc_->IsMultiline());
        });
    }

    ResourceSlotWidget *GetWidget()
    {
        return widget_.get();
    }
};
