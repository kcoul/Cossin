/**
    ===============================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any internal version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) 2019 ElandaSunshine
    ===============================================================
    
    @author Elanda
    @file   OptionPanel.cpp
    @date   18, October 2019
    
    ===============================================================
 */

#include "OptionPanel.h"

#include <jaut_provider/jaut_provider.h>
#include <jaut_util/general/scopedcursor.h>

#include "Assets.h"
#include "PluginEditor.h"
#include "PluginStyle.h"
#include "Resources.h"
#include "SharedData.h"


#if !JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
#   include "juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h"
#endif

//**********************************************************************************************************************
// region OptionsContainer
//======================================================================================================================
OptionPanel::OptionsContainer::OptionsContainer(OptionPanel &optionPanel)
    : parent(optionPanel)
{
    contentComponent.setTopLeftPosition(0, 0);
    addAndMakeVisible(contentComponent);
}

OptionPanel::OptionsContainer::~OptionsContainer()
{

}

//======================================================================================================================
void OptionPanel::OptionsContainer::resized()
{
    contentComponent.setSize(getWidth(), getHeight() * contentComponent.getNumChildComponents());
    
    for (int i = 0; i < contentComponent.getNumChildComponents(); ++i)
    {
        contentComponent.getChildComponent(i)->setBounds(0, getHeight() * i, getWidth(), getHeight());
    }
}

//======================================================================================================================
void OptionPanel::OptionsContainer::addOptionPanel(OptionCategory &category)
{
    contentComponent.addAndMakeVisible(category);
}

void OptionPanel::OptionsContainer::removeOptionPanel(OptionCategory &category)
{
    if (contentComponent.getIndexOfChildComponent(&category) >= 0)
    {
        contentComponent.removeChildComponent(&category);
        
        for (int i = 0; i < contentComponent.getNumChildComponents(); ++i)
        {
            Component *comp = contentComponent.getChildren().getUnchecked(i);
            comp->setBounds(0, getHeight() * i, getWidth(), getHeight());
        }
    
        repaint();
    }
}

void OptionPanel::OptionsContainer::showPanel(int, int newIndex, bool animate)
{
    const juce::Rectangle<int> bounds_final = contentComponent.getBounds().withY(-getHeight() * newIndex);

    if (animate)
    {
        juce::Desktop::getInstance().getAnimator().animateComponent(&contentComponent, bounds_final, 1.0f, 200,
                                                                    true, 2, 0);
    }
    else
    {
        contentComponent.setTopLeftPosition(bounds_final.getPosition());
    }
}
//======================================================================================================================
// endregion OptionsContainer
//**********************************************************************************************************************
// region OptionPanel
//======================================================================================================================
// region Functors
//======================================================================================================================
struct OptionPanel::NameGetter
{
    template<class ...Types>
    NameArray operator()(CategoryArray &categories)
    {
        return { std::get<Types>(categories.at(CategoryList::indexOf<Types>)).getCategoryName()... };
    }
};

template<class Arg>
struct OptionPanel::DataReloader
{
    template<class Type>
    void operator()(CategoryArray &categories, const Arg &arg)
    {
        using BareType = std::remove_cv_t<std::remove_reference_t<Arg>>;
        static_assert(   std::is_same_v<BareType, jaut::Config>
                      || std::is_same_v<BareType, jaut::Localisation>
                      || std::is_same_v<BareType, jaut::ThemePointer>,
                      "Is not an appropriate type for this functor");
    
        Type &category = std::get<Type>(categories.at(CategoryList::indexOf<Type>));
        
        if constexpr (std::is_same_v<BareType, jaut::Config>)
        {
            category.reloadConfig(arg);
        }
        else if constexpr (std::is_same_v<BareType, jaut::Localisation>)
        {
            category.reloadLocale(arg);
        }
        else if constexpr (std::is_same_v<BareType, jaut::ThemePointer>)
        {
            category.reloadTheme(arg);
        }
    }
};

template<>
struct OptionPanel::StateManager<true>
{
    juce::SharedResourcePointer<SharedData> sharedData;
    SharedData::ReadLock lock;
    
    explicit StateManager() noexcept
        : sharedData(SharedData::getInstance()),
          lock(*sharedData)
    {}
    
    template<class Type>
    void operator()(CategoryArray &categories)
    {
        std::get<Type>(categories.at(CategoryList::indexOf<Type>)).loadState(*sharedData);
    }
};

template<>
struct OptionPanel::StateManager<false>
{
    juce::SharedResourcePointer<SharedData> sharedData;
    SharedData::WriteLock lock;
    bool needsReload { false };
    
    explicit StateManager() noexcept
        : sharedData(SharedData::getInstance()),
          lock(*sharedData)
    {}
    
    ~StateManager()
    {
        if (needsReload)
        {
            (void) sharedData->Configuration().save();
            sharedData->sendUpdates();
        }
    }
    
    template<class Type>
    void operator()(CategoryArray &categories)
    {
        if (std::get<Type>(categories.at(CategoryList::indexOf<Type>)).saveState(*sharedData))
        {
            needsReload = true;
        }
    }
};
//======================================================================================================================
// endregion Functors
//======================================================================================================================
OptionPanel::OptionPanel(CossinAudioProcessorEditor &editor)
    : editor(editor), closeCallback([](juce::Button*){}),
      optionsContainer(*this),
      listBoxOptionTabs("OptionTabs", this),
      categories(CategoryList::fillArray(editor))
{
    for (auto &panel : categories)
    {
        std::visit([this](OptionCategory &panel)
        {
            optionsContainer.addOptionPanel(panel);
        }, panel);
    }
    
    // Change tabs for the standalone version
    if (juce::JUCEApplicationBase::isStandaloneApp())
    {
#if !JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
        this->onIntercept = [this](const String &categoryName)
        {
            if (categoryName == res::Cfg_Standalone)
            {
                StandalonePluginHolder::getInstance()->showAudioSettingsDialog();
                return false;
            }
            
            return true;
        };
#endif
    }
    else
    {
        auto &panel = std::get<OptionPanelStandalone>(categories.at(CategoryList::indexOf<OptionPanelStandalone>));
        optionsContainer.removeOptionPanel(panel);
    }
    
    addAndMakeVisible(optionsContainer);
    
    listBoxOptionTabs.setRowHeight(30);
    listBoxOptionTabs.selectRow(0);
    listBoxOptionTabs.updateContent();
    addAndMakeVisible(listBoxOptionTabs);
    
    labelTitleOptions.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(labelTitleOptions);
    
    buttonApply.setLookAndFeel(this);
    buttonApply.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    buttonApply.onClick = [this]()
    {
        jaut::ScopedCursorWait cursor;
        CategoryList::forEach<StateManager<false>>(categories);
    };
    addAndMakeVisible(buttonApply);
    
    buttonCancel.setLookAndFeel(this);
    buttonCancel.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    buttonCancel.getProperties().set("OptionsPanelClose", {});
    buttonCancel.onClick = [this]()
    {
        closeCallback(&buttonCancel);
    };
    addAndMakeVisible(buttonCancel);
    
    buttonOk.setLookAndFeel(this);
    buttonOk.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    buttonOk.onClick = [this]()
    {
        {
            jaut::ScopedCursorWait cursor;
            CategoryList::forEach<StateManager<false>>(categories);
        }
        
        closeCallback(&buttonCancel);
    };
    addAndMakeVisible(buttonOk);
    
    const juce::Font link_font = juce::Font().withHeight(14.0f);

    linkDiscord.setURL(juce::URL::createWithoutParsing("https://discord.io/ElandaSunshine"));
    linkDiscord.setButtonText("Discord");
    linkDiscord.setFont(link_font, false, juce::Justification::centredLeft);
    addAndMakeVisible(linkDiscord);

    linkTumblr.setURL(juce::URL::createWithoutParsing("https://blog." + juce::String(res::App_Website)));
    linkTumblr.setButtonText("ES Blog");
    linkTumblr.setFont(link_font, false, juce::Justification::centredLeft);
    addAndMakeVisible(linkTumblr);

    linkTwitter.setURL(juce::URL::createWithoutParsing("https://twitter.com/elandaofficial"));
    linkTwitter.setButtonText("Twitter");
    linkTwitter.setFont(link_font, false, juce::Justification::centredLeft);
    addAndMakeVisible(linkTwitter);

    linkWebsite.setURL(juce::URL::createWithoutParsing("https://www." + juce::String(res::App_Website)));
    linkWebsite.setButtonText("Website");
    linkWebsite.setFont(link_font, false, juce::Justification::centredLeft);
    addAndMakeVisible(linkWebsite);
    
    // About resources
    imgCossinAbout   = juce::ImageCache::getFromMemory(Assets::png011_png,         Assets::png011_pngSize);
    imgSocialDiscord = juce::ImageCache::getFromMemory(Assets::social_discord_png, Assets::social_discord_pngSize);
    imgSocialTumblr  = juce::ImageCache::getFromMemory(Assets::social_tumblr_png,  Assets::social_tumblr_pngSize);
    imgSocialTwitter = juce::ImageCache::getFromMemory(Assets::social_twitter_png, Assets::social_twitter_pngSize);
    imgSocialWebsite = juce::ImageCache::getFromMemory(Assets::social_web_png,     Assets::social_web_pngSize);
    
    imgCossinAbout = imgCossinAbout.getClippedImage({0, 36, imgCossinAbout.getWidth(), imgCossinAbout.getHeight() - 36});
}

OptionPanel::~OptionPanel() = default;

//======================================================================================================================
void OptionPanel::paint(juce::Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillAll();
    
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.drawRect(0, 30, getWidth(), 2);
    
    {
        juce::Graphics::ScopedSaveState sss(g);
        
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
        g.setOrigin(getWidth() - 202, 32);
        g.fillRect(0, 0, 202, getHeight() - 68);
        
        const int distance = 10;
        const int start = 12;
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId).contrasting());
        g.drawImageAt(imgCossinAbout,   0,     0);
        g.drawImageAt(imgSocialDiscord, start, start);
        g.drawImageAt(imgSocialTumblr,  start, start + 32 + distance);
        g.drawImageAt(imgSocialTwitter, start, start + 64 + distance * 2);
        g.drawImageAt(imgSocialWebsite, start, start + 96 + distance * 3);
    }
    
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.fillRect(0, getHeight() - 36, getWidth(), 2);
}

void OptionPanel::resized()
{   
    const int content_pos_x = getHeight() - 68;
    const int button_y      = getHeight() - 34;
    
    optionsContainer .setBounds(100,              32,       getWidth() - 300, content_pos_x);
    listBoxOptionTabs.setBounds(0,                32,       100,              content_pos_x);
    buttonOk         .setBounds(getWidth() - 303, button_y, 101,              34);
    buttonCancel     .setBounds(getWidth() - 202, button_y, 101,              34);
    buttonApply      .setBounds(getWidth() - 101, button_y, 101,              34);
    
    const int distance = 10;
    const int start    = 12;
    const int x        = getWidth() - 160;
    const int y        = 31 + start;
    const int w        = 160;
    const int h        = 32;

    linkDiscord.setBounds(x + distance, y,                     w, h);
    linkTumblr .setBounds(x + distance, y + 32 + distance,     w, h);
    linkTwitter.setBounds(x + distance, y + 64 + distance * 2, w, h);
    linkWebsite.setBounds(x + distance, y + 96 + distance * 3, w, h);
    
    labelTitleOptions.setBounds(0, 0, getWidth(), 30);
}

//======================================================================================================================
void OptionPanel::load()
{
    CategoryList::forEach<StateManager<true>>(categories);
}

//======================================================================================================================
void OptionPanel::setCloseButtonCallback(std::function<void(juce::Button*)> callback) noexcept
{
    closeCallback = std::move(callback);
}

//======================================================================================================================
void OptionPanel::drawButtonText(juce::Graphics &g, juce::TextButton &button, bool highlighted, bool)
{
    const LookAndFeel &lf = getLookAndFeel();
    
    g.setFont(font);
    g.setColour(highlighted ? lf.findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId)
                            : lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillAll();
    
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.drawRect(0, 0, 2, button.getHeight(), 2);
    
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    g.drawText(button.getButtonText(), 0, 0, button.getWidth(), button.getHeight(), juce::Justification::centred);
}

//======================================================================================================================
int OptionPanel::getNumRows()
{
    return categories.size();
}

void OptionPanel::paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
    const LookAndFeel &lf = getLookAndFeel();

    if (rowIsSelected)
    {
        const juce::DropShadow shadow(juce::Colours::black, 10, {101, 0});
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
        g.fillRect(0, 0, width, height);
        shadow.drawForRectangle(g, {0, 0, width, height});
    }
    
    g.setFont(font);
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    
    const juce::String &category_name = categoryNames[static_cast<NameArray::size_type>(rowNumber)];
    g.drawText(category_name, 10, 0, width - 10, height, juce::Justification::centredLeft);
}

void OptionPanel::listBoxItemClicked(int row, const juce::MouseEvent&)
{
    if (row == lastSelectedRow)
    {
        return;
    }
    
    if (onIntercept && !onIntercept(categoryNames.at(static_cast<NameArray::size_type>(row))))
    {
        listBoxOptionTabs.selectRow(lastSelectedRow);
        return;
    }

    optionsContainer.showPanel(lastSelectedRow, row,
                               editor.getOption(CossinAudioProcessorEditor::FlagAnimationComponents));
    lastSelectedRow = row;
}

//======================================================================================================================
void OptionPanel::reloadTheme(const jaut::ThemePointer &theme)
{
    const juce::Colour colour_background_contrasting = theme->getThemeColour(res::Col_ContainerBg).contrasting();

    linkDiscord .setColour(juce::HyperlinkButton::textColourId, colour_background_contrasting);
    linkTumblr  .setColour(juce::HyperlinkButton::textColourId, colour_background_contrasting);
    linkTwitter .setColour(juce::HyperlinkButton::textColourId, colour_background_contrasting);
    linkWebsite .setColour(juce::HyperlinkButton::textColourId, colour_background_contrasting);
    
    font = theme->getThemeFont();
    labelTitleOptions.setFont(font);
    
    CategoryList::forEach<DataReloader<jaut::ThemePointer>>(categories, theme);
}

void OptionPanel::reloadLocale(const jaut::Localisation &locale)
{
    labelTitleOptions.setText(locale.translate("options.title"), juce::dontSendNotification);
    
    buttonApply .setButtonText(locale.translate("general.button.apply"));
    buttonCancel.setButtonText(locale.translate("general.button.cancel"));
    buttonOk    .setButtonText(locale.translate("general.button.ok"));
    
    for (int i = 0; i < static_cast<int>(categoryNames.size()); ++i)
    {
        std::visit([&](auto &&var)
        {
            juce::String name = locale.translate("options.category." + juce::String(var.getCategoryName()));
            std::swap(categoryNames[static_cast<NameArray::size_type>(i)], name);
        },
        categories.at(static_cast<NameArray::size_type>(i)));
    }
    
    CategoryList::forEach<DataReloader<jaut::Localisation>>(categories, locale);
}

void OptionPanel::reloadConfig(const jaut::Config &config)
{
    CategoryList::forEach<DataReloader<jaut::Config>>(categories, config);
}
//======================================================================================================================
// endregion OptionPanel
//**********************************************************************************************************************
