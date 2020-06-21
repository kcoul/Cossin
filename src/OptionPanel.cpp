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
#include <jaut_util/jaut_util.h>

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

//======================================================================================================================
void OptionPanel::OptionsContainer::resized()
{
    contentComponent.setSize(getWidth(), getHeight() * parent.getNumRows());

    for(int i = 0; i < contentComponent.getNumChildComponents(); ++i)
    {
        Component *comp = contentComponent.getChildren().getUnchecked(i);
        comp->setBounds(0, getHeight() * i, getWidth(), getHeight());
    }
}

//======================================================================================================================
void OptionPanel::OptionsContainer::addOptionPanel(OptionCategory &category)
{
    category.setBounds(0, getHeight() * contentComponent.getNumChildComponents(), getWidth(), getHeight());
    categories.emplace_back(&category);
    contentComponent.addAndMakeVisible(category);
}

void OptionPanel::OptionsContainer::showPanel(int, int newIndex, bool animate)
{
    const juce::Rectangle<int> bounds_final = contentComponent.getBounds().withY(-getHeight() * newIndex);

    if(animate)
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
const std::vector<OptionCategory*>& OptionPanel::OptionsContainer::getCategories() const noexcept
{
    return categories;
}
//======================================================================================================================
// endregion OptionsContainer
//**********************************************************************************************************************
// region OptionPanel
//======================================================================================================================
OptionPanel::OptionPanel(CossinAudioProcessorEditor &editor, jaut::Localisation &locale)
    : editor(editor), closeCallback(nullptr), lastSelectedRow(0), bttClose("X"),
      optionContainer(*this), optionTabs("OptionTabs", this), locale(locale),
      optionsGeneral(editor, locale), optionsThemes(editor, locale),
      optionsPerformance(editor, locale)
{
    // About resources
    imgCossinAbout   = juce::ImageCache::getFromMemory(Assets::png011_png,         Assets::png011_pngSize);
    imgSocialDiscord = juce::ImageCache::getFromMemory(Assets::social_discord_png, Assets::social_discord_pngSize);
    imgSocialTumblr  = juce::ImageCache::getFromMemory(Assets::social_tumblr_png,  Assets::social_tumblr_pngSize);
    imgSocialTwitter = juce::ImageCache::getFromMemory(Assets::social_twitter_png, Assets::social_twitter_pngSize);
    imgSocialWebsite = juce::ImageCache::getFromMemory(Assets::social_web_png,     Assets::social_web_pngSize);

    // TODO option categories
    addOptionCategory(res::Cfg_General,      optionsGeneral);
    addOptionCategory("themes",              optionsThemes);
    addOptionCategory(res::Cfg_Optimization, optionsPerformance);
    //addOptionCategory(res::Cfg_Account,      optionsAccount);

    if(juce::JUCEApplicationBase::isStandaloneApp())
    {
        optionsStandalone = std::make_unique<OptionPanelStandalone>(editor, locale);
        addOptionCategory(res::Cfg_Standalone, *optionsStandalone);

#if !JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
        this->onIntercept = [this](const String &categoryName)
        {
            if(categoryName == res::Cfg_Standalone)
            {
                StandalonePluginHolder::getInstance()->showAudioSettingsDialog();
                return false;
            }
            
            return true;
        };
#endif
    }
    
    addAndMakeVisible(optionContainer);

    for(auto *category : optionContainer.getCategories())
    {
        editor.addReloadListener(category);
    }

    optionTabs.setRowHeight(30);
    optionTabs.selectRow(0);
    optionTabs.updateContent();
    addAndMakeVisible(optionTabs);

    bttClose.addListener(this);
    bttClose.setLookAndFeel(this);
    bttClose.getProperties().set("OptionsPanelClose", {});
    addAndMakeVisible(bttClose);

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
}

OptionPanel::~OptionPanel()
{
    optionsStandalone.reset();
    
    for(auto *category : optionContainer.getCategories())
    {
        editor.removeReloadListener(category);
    }
}

//======================================================================================================================
void OptionPanel::paint(juce::Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillAll();
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.drawRect(0, 30, getWidth(), 2);

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    g.setFont(font);
    jaut::FontFormat::drawSmallCaps(g, locale.translate("options.title"), 0, 0, getWidth(), 30,
                                    juce::Justification::centred);

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.setOrigin(getWidth() - 202, 32);
    g.fillRect(0, 0, 202, getHeight() - 32);

    const int distance = 10;
    const int start    = 12;
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId).contrasting());
    g.drawImageAt(imgCossinAbout, 0, 0, true);
    g.drawImageAt(imgSocialDiscord, start, start,                     true);
    g.drawImageAt(imgSocialTumblr,  start, start + 32 + distance,     true);
    g.drawImageAt(imgSocialTwitter, start, start + 64 + distance * 2, true);
    g.drawImageAt(imgSocialWebsite, start, start + 96 + distance * 3, true);
}

void OptionPanel::resized()
{   
    const int content_pos_x = getHeight() - 32;

    optionContainer.setBounds(100, 32, getWidth() - 300, content_pos_x);
    optionTabs     .setBounds(0, 32, 100, content_pos_x);
    bttClose       .setBounds(getWidth() - 30, 0, 30, 30);
    
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
}

//======================================================================================================================
void OptionPanel::addOptionCategory(const juce::String &name, OptionCategory &category)
{
    const juce::String category_id = name.removeCharacters(" ").toLowerCase();

    category.setName(category_id);
    categories.add(category_id);
    optionContainer.addOptionPanel(category);
}

//======================================================================================================================
void OptionPanel::show()
{
    const jaut::ScopedCursorWait wait;
    auto shared_data = SharedData::getInstance();
    const SharedData::ReadLock lock(*shared_data);

    for(auto *category : optionContainer.getCategories())
    {
        category->loadState(*shared_data);
    }
    
    setVisible(true);
}

void OptionPanel::hide()
{
    const jaut::ScopedCursorWait wait;
    auto shared_data = SharedData::getInstance();
    const SharedData::WriteLock lock(*shared_data);
    
    bool needs_full_reload = false;

    for(auto *category : optionContainer.getCategories())
    {
        if(!category->saveState(*shared_data))
        {
            needs_full_reload = true;
        }
    }

    (void) shared_data->Configuration().save();
    shared_data->sendChangeToAllInstancesExcept(needs_full_reload ? nullptr : &editor);
    setVisible(false);
}

//======================================================================================================================
void OptionPanel::setCloseButtonCallback(std::function<void(juce::Button*)> callback) noexcept
{
    closeCallback = std::move(callback);
}

//======================================================================================================================
void OptionPanel::buttonClicked(juce::Button *button)
{
    if (closeCallback && button == &bttClose)
    {
        closeCallback(button);
    }
}

//======================================================================================================================
void OptionPanel::drawButtonText(juce::Graphics &g, juce::TextButton &button, bool highlighted, bool)
{
    const LookAndFeel &lf = getLookAndFeel();
    g.setFont(font);
    g.setColour(highlighted ? lf.findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                            : lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.drawText(button.getName(), 0, 0, button.getWidth(), button.getHeight(), juce::Justification::centred);
}

//======================================================================================================================
int OptionPanel::getNumRows()
{
    return categories.size();
}

void OptionPanel::paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
    const LookAndFeel  &lf            = getLookAndFeel();
    const juce::String &category_name = categories.getReference(rowNumber);

    if(rowIsSelected)
    {
        const juce::DropShadow shadow(juce::Colours::black, 10, {101, 0});
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
        g.fillRect(0, 0, width, height);
        shadow.drawForRectangle(g, {0, 0, width, height});
    }
    
    g.setFont(font);
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    g.drawText(locale.translate("options.category." + category_name), 10, 0, width - 10, height,
               juce::Justification::centredLeft);
}

void OptionPanel::listBoxItemClicked(int row, const juce::MouseEvent &e)
{
    if(row == lastSelectedRow)
    {
        return;
    }

    const juce::String &category_name = categories.getReference(row);
    
    if(onIntercept && !onIntercept(category_name))
    {
        optionTabs.selectRow(lastSelectedRow);
        return;
    }

    optionContainer.showPanel(lastSelectedRow, row, editor.getOption(Flag_AnimationComponents));
    lastSelectedRow = row;
}

//======================================================================================================================
void OptionPanel::reloadTheme(const jaut::ThemePointer &theme)
{
    const juce::Colour colour_background_contrasting = theme->getThemeColour(res::Col_ContainerBg).contrasting();

    linkDiscord.setColour(juce::HyperlinkButton::textColourId, colour_background_contrasting);
    linkTumblr .setColour(juce::HyperlinkButton::textColourId, colour_background_contrasting);
    linkTwitter.setColour(juce::HyperlinkButton::textColourId, colour_background_contrasting);
    linkWebsite.setColour(juce::HyperlinkButton::textColourId, colour_background_contrasting);

    font = theme->getThemeFont();
}
//======================================================================================================================
// endregion OptionPanel
//**********************************************************************************************************************
