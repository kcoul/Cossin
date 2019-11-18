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
    @file   OptionCategories.cpp
    @date   03, November 2019
    
    ===============================================================
 */

#include "OptionCategories.h"

#include "PluginEditor.h"
#include "Resources.h"
#include "SharedData.h"
#include "ThemeFolder.h"

namespace
{
inline bool isDefaultTheme(const jaut::ThemeManager::ThemePointer &theme) noexcept
{
    return theme->getThemeMeta()->getName().removeCharacters(" ").equalsIgnoreCase("cossindefault");
}
}
/* ==================================================================================
 * ================================== ThemePreview ==================================
 * ================================================================================== */
#if(1) // ThemePreview
OptionPanelThemes::ThemePanel::ThemePreview::ThemePreview(ThemePanel &panel) noexcept
    : panel(panel)
{
    addAndMakeVisible(buttonWebsiteLink);

    gallery.setViewedComponent(new Component());
    gallery.setScrollBarsShown(false, true);
    addAndMakeVisible(gallery);

    labelNoPreview.setJustificationType(Justification::centred);
    labelNoPreview.setEditable(false);
    addAndMakeVisible(labelNoPreview);
}

//======================================================================================================================
void OptionPanelThemes::ThemePanel::ThemePreview::paint(Graphics &g)
{
    if(theme)
    {
        const LookAndFeel &lf    = getLookAndFeel();
        const Image thumbnail    = theme->getThemeThumbnail();
        const Font &font         = panel.font;
        const Font title_font    = font.withHeight(18.0f).withStyle(Font::bold);
        const Font version_font  = font.withHeight(11.0f);
        const String title       = theme->getThemeMeta()->getName();
        const String version     = theme->getThemeMeta()->getVersion();
        const String license     = theme->getThemeMeta()->getLicense().first.isEmpty() ? "n/a"
                                 : theme->getThemeMeta()->getLicense().first;
        const String authors     = theme->getThemeMeta()->getAuthors().isEmpty() ? ""
                                 : theme->getThemeMeta()->getAuthors().joinIntoString("; ");
        const int version_length = version_font.getStringWidth(version);
        const int max_text_width = getWidth() - 12;

        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
        g.fillAll();

        g.setOrigin(6, 6);
        g.setFont(title_font);
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
        g.drawText(title, 0, 0, getWidth() - version_length, 18, Justification::bottomLeft);
        g.setFont(version_font);
        g.drawText(version, title_font.getStringWidth(title) + 3, 0, version_length, 18, Justification::bottomLeft);

        g.drawImageWithin(thumbnail, 0, 24, 64, 64, RectanglePlacement::stretchToFit);

        g.setFont(font.withHeight(14.0f));
        g.drawFittedText(theme->getThemeMeta()->getDescription(), 70, 24, getWidth() - 70, 50,
                         Justification::topLeft, 10, 1.0f);
        g.drawText("Author: " + theme->getThemeMeta()->getAuthor(), 70, 74, getWidth() - 70, 14,
                   Justification::bottomLeft);
        g.drawText("Website: ", 0, 96, 55, 14, Justification::left);
        g.drawText("License: " + license, 0, 114, max_text_width, 14, Justification::left);
        g.drawText("Other authors:", 0, 132, 100, 14, Justification::left);
        g.setOpacity(0.6f);
        g.drawText(authors, 0, 148, max_text_width, 14, Justification::left);
    }
}

void OptionPanelThemes::ThemePanel::ThemePreview::resized()
{
    buttonWebsiteLink.setBounds(56, 102, getWidth() - 68, 14);
    gallery.setBounds(6, getHeight() - 106, getWidth() - 12, 100);
    labelNoPreview.setBounds(6, getHeight() - 106, getWidth() - 12, 100);
}

//======================================================================================================================
void OptionPanelThemes::ThemePanel::ThemePreview::updateContent(const jaut::ThemeManager::ThemePointer &theme) noexcept
{
    if(!theme || this->theme == theme)
    {
        return;
    }

    this->theme = theme;

    resized();

    const int ratio_height = gallery.getHeight() - 12;
    const int ratio_width  = ratio_height / 9 * 16;
    Component &content     = *gallery.getViewedComponent();

    content.removeAllChildren();

    if(!::isDefaultTheme(theme))
    {
        const StringArray screenshot_names = theme->getThemeMeta()->getScreenshots();
        int counter = 0;

        for(int i = 0; i < screenshot_names.size(); ++i)
        {
            if(counter >= 5)
            {
                break;
            }

            const File screenshot_file = theme->getFile(screenshot_names[i]);

            if(screenshot_file.exists())
            {
                const Image screenshot = ImageFileFormat::loadFrom(screenshot_file);

                if(screenshot.isValid())
                {
                    ImageComponent &image_component = screenshots[counter];
                    image_component.setImage(screenshot);
                    image_component.setBounds((ratio_width + 6) * counter++, 2, ratio_width, ratio_height);
                    content.addAndMakeVisible(image_component);
                }
            }
        }

        content.setBounds(0, 0, (ratio_width * counter) + (6 * (counter - 1)), gallery.getHeight());
    }
    else
    {        
        for(int i = 0; i < 3; ++i)
        {
            int image_size;
            const char *image_data = Assets::getNamedResource(String("screenshot00" + String(i) + "_png").toRawUTF8(),
                                                              image_size);
            ImageComponent &image_component = screenshots[i];
            image_component.setImage(ImageCache::getFromMemory(image_data, image_size));
            image_component.setBounds((ratio_width + 6) * i, 2, ratio_width, ratio_height);
            content.addAndMakeVisible(image_component);
        }

        content.setBounds(0, 0, ratio_width * 3 + 12, gallery.getHeight());
    }

    labelNoPreview.setVisible(content.getNumChildComponents() < 1);

    const URL website_url = theme->getThemeMeta()->getWebsite();

    if(!website_url.isEmpty())
    {
        buttonWebsiteLink.setButtonText(website_url.toString(true));
        buttonWebsiteLink.setURL(website_url);
    }
    else
    {
        buttonWebsiteLink.setButtonText("n/a");
        buttonWebsiteLink.setURL(String());
    }
}
#endif // ThemePreview



/* ==================================================================================
 * =================================== ThemePanel ===================================
 * ================================================================================== */
#if(1) // ThemePanel
OptionPanelThemes::ThemePanel::ThemePanel() noexcept
    : selectedTheme(0), selectedRow(0),
      previewBox(*this), themeList("", this)
{
    themeList.setRowHeight(36);
    themeList.setColour(ListBox::outlineColourId, Colours::transparentBlack);
    addAndMakeVisible(themeList);
    addAndMakeVisible(previewBox);

    buttonApply.addListener(this);
    addAndMakeVisible(buttonApply);
}

//======================================================================================================================
void OptionPanelThemes::ThemePanel::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.drawRect(6, 6, 150, getHeight() - 12, 2);
}

void OptionPanelThemes::ThemePanel::resized()
{
    const int panel_width   = getWidth() - 170;
    const int button_height = 30;

    themeList  .setBounds(8, 8, 146, getHeight() - 16);
    previewBox .setBounds(162, 6, panel_width, getHeight() - (12 + button_height));
    buttonApply.setBounds(162, getHeight() - (button_height + 6), panel_width, button_height);
}

//======================================================================================================================
int OptionPanelThemes::ThemePanel::getNumRows()
{
    return themes.size();
}

void OptionPanelThemes::ThemePanel::paintListBoxItem(int row, Graphics &g, int width, int height, bool selected)
{
    const LookAndFeel &lf = getLookAndFeel();
    const auto theme      = themes.at(row);

    if(theme)
    {
        if(selected)
        {
            g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId));
            g.fillRect(0, 0, width, height);
        }

        if(row == selectedTheme)
        {
            g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
            g.fillEllipse(width - 9.0f, 3.0f, 6.0f, 6.0f);
        }

        const jaut::IMetadata *metadata = theme->getThemeMeta();
        const Font current_font         = font.withHeight(12.0f);
        const int text_width            = width - 48;
        const int row_y = row * height;

        g.drawImageWithin(theme->getThemeThumbnail(), 6, 2, 32, 32, RectanglePlacement::fillDestination);

        g.setFont(current_font.withStyle(Font::bold));
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
        g.drawText(metadata->getName(), 42, 2, text_width - 12, 12, Justification::centredLeft);

        g.setFont(current_font);
        g.drawText(metadata->getDescription(), 42, 13, text_width, 12, Justification::centredLeft);

        g.setOpacity(0.5f);
        g.drawText(metadata->getAuthor(), 42, 23, text_width, 12, Justification::centredLeft);
    }
    else
    {
        g.setFont(font);
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
        g.drawText("n/a", 6, 0, width, height, Justification::centredLeft);
    }
}

void OptionPanelThemes::ThemePanel::listBoxItemClicked(int row, const MouseEvent&)
{
    if(selectedRow != row)
    {
        previewBox.updateContent(themes.at(row));
        changeButtonState();
        repaint();
        selectedRow = row;
    }
}

void OptionPanelThemes::ThemePanel::listBoxItemDoubleClicked(int row, const MouseEvent &e)
{
    listBoxItemClicked(row, e);
    buttonClicked(nullptr);
}

//======================================================================================================================
void OptionPanelThemes::ThemePanel::buttonClicked(Button*)
{
    const int selected_row = themeList.getSelectedRow();

    if(selected_row == selectedTheme)
    {
        return;
    }

    MouseCursor::showWaitCursor();

    {
        auto shared_data = SharedData::getInstance();
        auto lock        = shared_data->setWriting();

        const auto new_theme = themes.at(selected_row);

        if(new_theme)
        {
            shared_data->Style().reset(new_theme);
            shared_data->Configuration().getProperty("theme", res::Cfg_General).setValue(new_theme.getName());
            shared_data->sendUpdate();
            selectedTheme = selected_row;
        }

        buttonApply.setEnabled(false);
    }

    MouseCursor::hideWaitCursor();
}

void OptionPanelThemes::ThemePanel::changeButtonState() noexcept
{
    const int selected_row = themeList.getSelectedRow();
    buttonApply.setEnabled(selected_row != selectedTheme);
}
#endif // ThemePanel



/* ==================================================================================
 * =============================== OptionPanelGeneral ===============================
 * ================================================================================== */
#if (1) // OptionPanelGeneral
OptionPanelThemes::OptionPanelThemes(CossinAudioProcessorEditor &editor)
    : editor(editor)
{
    editor.addReloadListener(this);
    addAndMakeVisible(themePanel);
}

OptionPanelThemes::~OptionPanelThemes()
{
    editor.removeReloadListener(this);
}

//======================================================================================================================
void OptionPanelThemes::resized()
{
    themePanel.setBounds(0, 0, getWidth(), getHeight());
}

//======================================================================================================================
void OptionPanelThemes::selectThemeRow(const jaut::ThemeManager::ThemePointer &theme)
{
    const auto iterator = std::find(themePanel.themes.begin(), themePanel.themes.end(), theme);

    if(::isDefaultTheme(theme) || iterator == themePanel.themes.end())
    {
        themePanel.themeList.selectRow(0);
    }
    else
    {
        themePanel.themeList.selectRow(std::distance(themePanel.themes.begin(), iterator));
    }

    themePanel.previewBox.updateContent(theme);
    themePanel.selectedRow = themePanel.themeList.getSelectedRow();
    themePanel.changeButtonState();
}

void OptionPanelThemes::resetThemeList(const jaut::ThemeManager::ThemePointer &defaultTheme,
                                       const std::vector<jaut::ThemeManager::ThemePointer> &themes)
{
    themePanel.themes.clear();
    themePanel.themes.emplace_back(defaultTheme);

    for(auto theme : themes)
    {
        if(!::isDefaultTheme(theme))
        {
            themePanel.themes.emplace_back(theme);
        }
    }

    themePanel.themeList.updateContent();
    repaint();
}

//======================================================================================================================
void OptionPanelThemes::reloadLocale(const jaut::Localisation &locale)
{
    themePanel.buttonApply.setButtonText(locale.translate("options.category.themes.apply"));
    themePanel.previewBox.labelNoPreview.setText(locale.translate("options.category.themes.no_preview"),
                                                 NotificationType::sendNotificationAsync);
}

void OptionPanelThemes::reloadTheme(const jaut::ThemeManager::ThemePointer &theme)
{
    const Colour colour_font = theme->getThemeColour(res::Col_Font);

    themePanel.font = theme->getThemeFont();
    themePanel.themeList.setColour(ListBox::backgroundColourId, theme->getThemeColour(res::Col_Container_Bg));
    themePanel.themeList.setColour(ListBox::textColourId, colour_font);
    themePanel.imageApply = theme->getImage(res::Png_Play);
    themePanel.previewBox.buttonWebsiteLink.setFont(themePanel.font.withHeight(14.0f), false, Justification::left);
    themePanel.previewBox.labelNoPreview.setFont(themePanel.font.withHeight(14.0f).withStyle(Font::bold));
    themePanel.previewBox.labelNoPreview.setColour(Label::outlineColourId, colour_font);
    themePanel.previewBox.labelNoPreview.setColour(Label::textColourId,    colour_font);
    selectThemeRow(theme);

    auto iterator = std::find(themePanel.themes.begin(), themePanel.themes.end(), theme);

    if(iterator != themePanel.themes.end())
    {
        themePanel.selectedTheme = std::distance(themePanel.themes.begin(), iterator);
    }

    repaint();
}

#endif // OptionPanelGeneral
