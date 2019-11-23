/**
    ===============================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) 2019 ElandaSunshine
    ===============================================================
    
    @author Elanda (elanda@elandasunshine.xyz)
    @file   OptionPanel.cpp
    @date   18, October 2019
    
    ===============================================================
 */

#include "OptionPanel.h"
#include "PluginEditor.h"
#include "Resources.h"
#include "SharedData.h"
#include <jaut/appdata.h>
#include <jaut/config.h>
#include <jaut/fontformat.h>
#include <jaut/localisation.h>

#if !JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
#include "juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h"
#endif

namespace
{
inline bool isDefaultTheme(const jaut::ThemeManager::ThemePointer &theme) noexcept
{
    return theme.getName().removeCharacters(" ").equalsIgnoreCase("cossindefault");
}
}



/* ==================================================================================
 * ================================ OptionsContainer ================================
 * ================================================================================== */
#if (1) // OptionsContainer
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
void OptionPanel::OptionsContainer::addOptionPanel(Component &component)
{
    component.setBounds(0, getHeight() * contentComponent.getNumChildComponents(), getWidth(), getHeight());
    contentComponent.addAndMakeVisible(component);
}

void OptionPanel::OptionsContainer::showPanel(int lastIndex, int newIndex, bool animate)
{
    const Rectangle<int> bounds_final = contentComponent.getBounds().withY(-getHeight() * newIndex);

    if(animate)
    {
        Desktop::getInstance().getAnimator().animateComponent(&contentComponent, bounds_final, 1.0f, 200, true, 2, 0);
    }
    else
    {
        contentComponent.setTopLeftPosition(bounds_final.getPosition());
    }
}
#endif // OptionsContainer



/* ==================================================================================
 * ================================== OptionPanel ===================================
 * ================================================================================== */
#if (1) // OptionPanel
OptionPanel::OptionPanel(CossinAudioProcessorEditor &cossin, jaut::Localisation &locale)
    : cossin(cossin), closeCallback(nullptr), lastSelectedRow(0), bttClose("X"),
      optionContainer(*this), optionTabs("OptionTabs", this), locale(locale),
      optionsGeneral(cossin, locale), optionsThemes(cossin)
{   
    cossin.addReloadListener(this);

    categories.add(res::Cfg_General);
    categories.add("themes");
    categories.add(res::Cfg_Optimization);
    categories.add("account");
    categories.add(res::Cfg_Standalone);

    // About resources
    imgCossinAbout   = ImageCache::getFromMemory(Assets::png011_png,         Assets::png011_pngSize);
    imgSocialDiscord = ImageCache::getFromMemory(Assets::social_discord_png, Assets::social_discord_pngSize);
    imgSocialTumblr  = ImageCache::getFromMemory(Assets::social_tumblr_png,  Assets::social_tumblr_pngSize);
    imgSocialTwitter = ImageCache::getFromMemory(Assets::social_twitter_png, Assets::social_twitter_pngSize);
    imgSocialWebsite = ImageCache::getFromMemory(Assets::social_web_png,     Assets::social_web_pngSize);

    // TODO option categories
    optionContainer.addOptionPanel(optionsGeneral);
    optionContainer.addOptionPanel(optionsThemes);
    //optionContainer.addOptionPanel(optionsOptimization);
    //optionContainer.addOptionPanel(optionsAccount);
    //optionContainer.addOptionPanel(optionsStandalone);
    addAndMakeVisible(optionContainer);

    optionTabs.setRowHeight(30);
    optionTabs.setColour(ListBox::outlineColourId, Colours::transparentBlack);
    optionTabs.selectRow(0);
    optionTabs.updateContent();
    addAndMakeVisible(optionTabs);

    bttClose.addListener(this);
    bttClose.setLookAndFeel(this);
    bttClose.getProperties().set("OptionsPanelClose", var());
    addAndMakeVisible(bttClose);

    const Font link_font = Font().withHeight(14.0f);

    linkDiscord.setURL(URL("https://discord.io/ElandaSunshine"));
    linkDiscord.setButtonText("Discord");
    linkDiscord.setFont(link_font, false, Justification::centredLeft);
    addAndMakeVisible(linkDiscord);

    linkTumblr.setURL(URL("https://blog." + String(res::App_Website)));
    linkTumblr.setButtonText("ES Blog");
    linkTumblr.setFont(link_font, false, Justification::centredLeft);
    addAndMakeVisible(linkTumblr);

    linkTwitter.setURL(URL("https://twitter.com/elandaofficial"));
    linkTwitter.setButtonText("Twitter");
    linkTwitter.setFont(link_font, false, Justification::centredLeft);
    addAndMakeVisible(linkTwitter);

    linkWebsite.setURL(URL("https://www." + String(res::App_Website)));
    linkWebsite.setButtonText("Website");
    linkWebsite.setFont(link_font, false, Justification::centredLeft);
    addAndMakeVisible(linkWebsite);
}

OptionPanel::~OptionPanel()
{
    cossin.removeReloadListener(this);
}

//======================================================================================================================
void OptionPanel::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillAll();
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.drawRect(0, 30, getWidth(), 2);

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    g.setFont(font);
    jaut::FontFormat::drawSmallCaps(g, locale.translate("options.title"), 0, 0, getWidth(), 30, Justification::centred);

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
void OptionPanel::show()
{
    {
        auto shared_data = SharedData::getInstance();
        auto lock        = shared_data->setReading();

        // General tab
        optionsGeneral.resetLangList(shared_data->App().getDir("Lang").toFile());
        optionsGeneral.selectLangRow(shared_data->Locale().getLanguageFile());

        // Themes tab
        optionsThemes.resetThemeList(shared_data->getDefaultTheme(), shared_data->Themes().getAllThemePacks());
        optionsThemes.selectThemeRow(shared_data->Style().getTheme());
    }

    setVisible(true);
}

void OptionPanel::hide()
{
    MouseCursor::showWaitCursor();

    {
        auto shared_data = SharedData::getInstance();
        auto lock        = shared_data->setWriting();

        shared_data->Configuration().save();
        shared_data->sendUpdate();
    }

    MouseCursor::hideWaitCursor();
    setVisible(false);
}

//======================================================================================================================
void OptionPanel::setCloseButtonCallback(std::function<void(Button*)> callback) noexcept
{
    closeCallback = callback;
}

//======================================================================================================================
void OptionPanel::buttonClicked(Button *button)
{
    if(closeCallback && button == &bttClose)
    {
        closeCallback(button);
    }
}

//======================================================================================================================
void OptionPanel::drawButtonText(Graphics &g, TextButton &button, bool highlighted, bool)
{
    const LookAndFeel &lf = getLookAndFeel();
    g.setFont(font);
    g.setColour(highlighted ? lf.findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                            : lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.drawText(button.getName(), 0, 0, button.getWidth(), button.getHeight(), Justification::centred);
}

//======================================================================================================================
int OptionPanel::getNumRows()
{
    return categories.size();
}

void OptionPanel::paintListBoxItem(int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    const LookAndFeel &lf       = getLookAndFeel();
    const String &category_name = categories.getReference(rowNumber);

    if(rowIsSelected)
    {
        DropShadow shadow(Colours::black, 10, {101, 0});
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
        g.fillRect(0, 0, width, height);
        shadow.drawForRectangle(g, {0, 0, width, height});
    }

    g.setFont(font);
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));

    JT_IS_STANDALONE({})
    JT_STANDALONE_ELSE
    (
        if(category_name.equalsIgnoreCase("standalone") && !JUCEApplicationBase::isStandaloneApp())
        {
            g.setOpacity(0.3f);
        }
    )

    if(category_name.equalsIgnoreCase("account"))
    {
        g.setOpacity(0.3f);
    }

    g.drawText(locale.translate("options.category." + category_name), 10, 0, width - 10, height,
               Justification::centredLeft);
}

void OptionPanel::listBoxItemClicked(int row, const MouseEvent &e)
{
    if(row == lastSelectedRow)
    {
        return;
    }

    const String &category_name = categories.getReference(row);

    JT_IS_STANDALONE({})
    JT_STANDALONE_ELSE
    (
        if(category_name.equalsIgnoreCase("standalone"))
        {
#if !JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
            if(JUCEApplicationBase::isStandaloneApp())
            {
                StandalonePluginHolder::getInstance()->showAudioSettingsDialog();
            }
#endif
            optionTabs.selectRow(lastSelectedRow);
            return;
        }
    )

    if(category_name.equalsIgnoreCase("account"))
    {
        optionTabs.selectRow(lastSelectedRow);
        return;
    }

    optionContainer.showPanel(lastSelectedRow, row, cossin.getOption(Flag_AnimationComponents));
    lastSelectedRow = row;
}

//======================================================================================================================
void OptionPanel::reloadConfig(const jaut::Config &config)
{
    
}

void OptionPanel::reloadTheme(const jaut::ThemeManager::ThemePointer &theme)
{
    const LookAndFeel &lf    = getLookAndFeel();
    const Colour colour_container_background   = theme->getThemeColour(res::Col_Container_Bg);
    const Colour colour_background_contrasting = colour_container_background.contrasting();

    optionTabs.setColour(ListBox::backgroundColourId, colour_container_background);
    optionTabs.setColour(ListBox::textColourId,       theme->getThemeColour(res::Col_Font));

    linkDiscord.setColour(HyperlinkButton::textColourId, colour_background_contrasting);
    linkTumblr .setColour(HyperlinkButton::textColourId, colour_background_contrasting);
    linkTwitter.setColour(HyperlinkButton::textColourId, colour_background_contrasting);
    linkWebsite.setColour(HyperlinkButton::textColourId, colour_background_contrasting);

    font = theme->getThemeFont();
}

void OptionPanel::reloadLocale(const jaut::Localisation &locale)
{
    
}
#endif // OptionPanel
