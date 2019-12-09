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
#include "PluginStyle.h"
#include "Resources.h"
#include "SharedData.h"
#include "ThemeFolder.h"
#include <jaut/appdata.h>
#include <jaut/config.h>
#include <jaut/fontformat.h>
#include <jaut/localisation.h>

namespace
{
inline constexpr int getAuthorPosY(float maxLineWidth, int descriptionLength) noexcept
{
    return 16 * std::min(static_cast<int>(std::ceil(static_cast<float>(descriptionLength) / maxLineWidth)), 3);
}

inline int getLanguageListIndex(const File &languageFile, const std::vector<std::pair<String, String>> &languageList)
{
    const String language_name = languageFile.getFileNameWithoutExtension();

    for(int i = 0; i < languageList.size(); ++i)
    {
        if(languageList.at(i).first.equalsIgnoreCase(language_name))
        {
            return i;
        }
    }

    return -1;
}
}



#if(1) // CategoryGeneral
/* ==================================================================================
 * ================================= PanelDefaults ==================================
 * ================================================================================== */
#if(1) // PanelDefaults
OptionPanelGeneral::PanelDefaults::PanelDefaults(OptionPanelGeneral &panel, jaut::Localisation &locale)
    : panel(panel), locale(locale)
{
    boxPanningLaw.addItem("Linear",     1);
    boxPanningLaw.addItem("Square",     2);
    boxPanningLaw.addItem("Sinusoidal", 3);
    boxPanningLaw.addListener(this);
    addAndMakeVisible(boxPanningLaw);

    // FUTURE (stack, graph)
    boxProcessor.addItem("Solo",  1);
    //boxProcessor.addItem("Stack", 2);
    //boxProcessor.addItem("Graph", 3);
    boxProcessor.addListener(this);
    addAndMakeVisible(boxProcessor);
}

//======================================================================================================================
void OptionPanelGeneral::PanelDefaults::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setFont(panel.font.withHeight(13.0f));
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    jaut::FontFormat::drawSmallCaps(g, locale.translate("options.category.general.default_panning"),
                                    0, 0, getWidth(), 13, Justification::centredLeft);
}

void OptionPanelGeneral::PanelDefaults::resized()
{
    boxPanningLaw.setBounds(0, 17, 200, 30);
}

//======================================================================================================================
void OptionPanelGeneral::PanelDefaults::comboBoxChanged(ComboBox *comboBoxThatHasChanged)
{
    
}
#endif // PanelDefaults



/* ==================================================================================
 * =============================== OptionPanelGeneral ===============================
 * ================================================================================== */
#if(1) // OptionPanelGeneral
OptionPanelGeneral::OptionPanelGeneral(CossinAudioProcessorEditor &editor, jaut::Localisation &locale)
    : editor(editor), locale(locale), currentLanguageIndex(0), lastSelected(0),
      defaultsBox(*this, locale), languageList("", this)

{
    editor.addReloadListener(this);

    addAndMakeVisible(defaultsBox);

    languageList.setColour(ListBox::backgroundColourId, Colours::transparentBlack);
    addAndMakeVisible(languageList);
}

OptionPanelGeneral::~OptionPanelGeneral()
{
    editor.removeReloadListener(this);
}

//======================================================================================================================
void OptionPanelGeneral::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.fillRect(languageList.getBoundsInParent().expanded(0, 1));

    g.setFont(font);
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    jaut::FontFormat::drawSmallCaps(g, locale.translate("options.category.general.defaults_title"),
                                    8, 0, getWidth() - 183, 27, Justification::centred);
    jaut::FontFormat::drawSmallCaps(g, locale.translate("options.category.general.select_language"),
                                    getWidth() - 177, 0, 169, 27, Justification::centred);
}

void OptionPanelGeneral::resized()
{
    const int component_height = getHeight() - 34;
    const int defaults_width   = getWidth() - 183;

    languageList.setBounds(defaults_width + 6, 27, 169, component_height);
    defaultsBox.setBounds(6, 27, defaults_width, component_height);
}

//======================================================================================================================
void OptionPanelGeneral::resetDefaults(const jaut::Config &config)
{
    const int panning_value = config.getProperty("panning", res::Cfg_Defaults).getValue();

    if(jaut::is_in_range(panning_value, 0, 2))
    {
        defaultsBox.boxPanningLaw.setSelectedId(panning_value + 1);
    }
    else
    {
        defaultsBox.boxPanningLaw.setSelectedId(2);
    }
}

void OptionPanelGeneral::selectLangRow(const File &languageFile)
{
    if(languageFile.getFullPathName().isEmpty())
    {
        languageList.selectRow(0);
        currentLanguageIndex = 0;
        lastSelected         = 0;

        return;
    }

    const int index      = ::getLanguageListIndex(languageFile, languages);
    currentLanguageIndex = index;
    lastSelected         = index;
    languageList.selectRow(index);
}

void OptionPanelGeneral::resetLangList(const File &langDirectory)
{
    DirectoryIterator iterator(langDirectory, false, "*.lang");

    languages.clear();
    languages.emplace_back("default", "Default");

    while(iterator.next())
    {
        const File language_file = iterator.getFile();
        const String file_name   = language_file.getFileNameWithoutExtension();
        auto lang_data           = jaut::Localisation::getLanguageFileData(language_file);
        languages.emplace_back(file_name, lang_data.first + " - " + lang_data.second.joinIntoString(" "));
    }

    Logger::getCurrentLogger()->writeToLog(langDirectory.getFullPathName());

    languageList.updateContent();
}

//======================================================================================================================
int OptionPanelGeneral::getDefaultPanningMode() const noexcept
{
    return defaultsBox.boxPanningLaw.getSelectedId() - 1;
}

int OptionPanelGeneral::getDefaultProcessor() const noexcept
{
    return defaultsBox.boxProcessor.getSelectedId() - 1;
}

Rectangle<int> OptionPanelGeneral::getDefaultWindowSize() const noexcept
{
    return {800, 500};
}

String OptionPanelGeneral::getSelectedLanguage() const noexcept
{
    return currentLanguageIndex == 0 ? "default" : languages.at(currentLanguageIndex).first;
}

//======================================================================================================================
void OptionPanelGeneral::reloadLocale(const jaut::Localisation &locale)
{
    selectLangRow(locale.getLanguageFile());
}

void OptionPanelGeneral::reloadTheme(const jaut::ThemePointer &theme)
{
    font = theme->getThemeFont();
}

void OptionPanelGeneral::reloadConfig(const jaut::Config &config)
{

}

//======================================================================================================================
int OptionPanelGeneral::getNumRows()
{
    return languages.size();
}

void OptionPanelGeneral::paintListBoxItem(int row, Graphics &g, int width, int height, bool selected)
{
    const LookAndFeel &lf       = getLookAndFeel();
    const bool is_selected_lang = row == currentLanguageIndex;

    if(selected)
    {
        Colour selected_background_colour = lf.findColour(is_selected_lang
                                                          ? CossinAudioProcessorEditor::ColourComponentForegroundId
                                                          : CossinAudioProcessorEditor::ColourComponentBackgroundId);
        g.setColour(selected_background_colour);
        g.fillRect(g.getClipBounds().reduced(2, 1));

        g.setColour(is_selected_lang ? selected_background_colour.contrasting()
                                     : lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    }
    else
    {
        if(is_selected_lang)
        {
            g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
            g.fillRect(g.getClipBounds().reduced(2, 1));
        }

        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    }

    g.setFont(font);
    g.drawText(languages.at(row).second, 0, 0, width, height, Justification::centred);
}

void OptionPanelGeneral::listBoxItemClicked(int row, const MouseEvent&)
{
    lastSelected = row;
}

void OptionPanelGeneral::listBoxItemDoubleClicked(int row, const MouseEvent&)
{
    if(row == currentLanguageIndex)
    {
        return;
    }

    jaut::Localisation new_locale(locale);
    const String lang_name = languages.at(row).first;

    if(row == 0 || lang_name == "default")
    {
        new_locale.setCurrentLanguage(SharedData::getInstance()->getDefaultLocale());
        editor.reloadLocale(new_locale);
        editor.repaint();
        currentLanguageIndex = 0;
    }
    else if(new_locale.setCurrentLanguage(languages.at(row).first))
    {
        editor.reloadLocale(new_locale);
        editor.repaint();
        currentLanguageIndex = row;
    }
    else
    {
        resetLangList(locale.getRootDirectory());
        languageList.selectRow(lastSelected);
    }

    lastSelected = row;
}
#endif // OptionPanelGeneral
#endif // CategoryGeneral



#if(1) // CategoryThemes
/* ==================================================================================
 * ================================== ThemePreview ==================================
 * ================================================================================== */
#if(1) // ThemePreview
OptionPanelThemes::ThemePanel::ThemePreview::ThemePreview(ThemePanel &panel)
    : panel(panel)
{
    addAndMakeVisible(buttonWebsiteLink);
    addAndMakeVisible(buttonLicenseLink);

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
    if(theme && theme->isValid())
    {
        const LookAndFeel &lf    = getLookAndFeel();
        const Image thumbnail    = theme->getThemeThumbnail();
        const Font &font         = panel.font;
        const Font title_font    = font.withHeight(18.0f).withStyle(Font::bold);
        const Font version_font  = font.withHeight(11.0f);
        const Font content_font  = font.withHeight(14.0f);
        const String title       = theme->getThemeMeta()->getName();
        const String version     = theme->getThemeMeta()->getVersion();
        const String description = theme->getThemeMeta()->getDescription();
        const String author      = theme->getThemeMeta()->getAuthor();
        const String website     = buttonWebsiteLink.getURL().isEmpty() ? buttonWebsiteLink.getButtonText() : "";
        const String license     = buttonLicenseLink.getURL().isEmpty() ? buttonLicenseLink.getButtonText() : "";
        const String authors     = theme->getThemeMeta()->getAuthors().isEmpty() ? ""
                                 : theme->getThemeMeta()->getAuthors().joinIntoString("; ");
        const int version_length = version_font.getStringWidth(version);
        const int max_text_width = getWidth() - 12;
        const int description_w  = max_text_width - 72;
        const int author_pos_y   = ::getAuthorPosY(description_w, content_font.getStringWidthFloat(description));

        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
        g.fillAll();

        // Title
        g.setOrigin(6, 6);
        g.setFont(title_font);
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
        g.drawText(title, 0, 0, getWidth() - version_length, 18, Justification::bottomLeft);
        g.setFont(version_font);
        g.drawText(version, title_font.getStringWidth(title) + 3, 0, version_length, 18, Justification::bottomLeft);

        // Header
        g.drawImageWithin(thumbnail, 0, 24, 64, 64, RectanglePlacement::stretchToFit);
        g.setFont(content_font);
        g.drawFittedText(description, 70, 24, description_w, 50, Justification::topLeft, 3, 1.0f);
        g.setOpacity(0.5f);
        g.drawText(author, 70, 24 + author_pos_y, getWidth() - 70, 14, Justification::bottomLeft);
        g.setOpacity(1.0f);
        
        // Body
        g.drawText("Website: " + website, 0, 96,  max_text_width, 14, Justification::left);
        g.drawText("License: " + license, 0, 114, max_text_width, 14, Justification::left);
        g.drawText("Authors: ", 0, 132, 100, 14, Justification::left);
        g.setOpacity(0.5f);
        g.drawText(authors, 0, 148, max_text_width, 14, Justification::left);
    }
}

void OptionPanelThemes::ThemePanel::ThemePreview::resized()
{
    buttonWebsiteLink.setBounds(56, 102, getWidth() - 62, 14);
    buttonLicenseLink.setBounds(56, 120, getWidth() - 62, 14);
    gallery.setBounds(6, getHeight() - 106, getWidth() - 12, 100);
    labelNoPreview.setBounds(6, getHeight() - 106, getWidth() - 12, 100);
}

//======================================================================================================================
void OptionPanelThemes::ThemePanel::ThemePreview::updateContent(const jaut::ThemePointer &theme)
{
    if(!theme || !theme->isValid() || this->theme == theme)
    {
        return;
    }

    this->theme = theme;

    resized();

    const int ratio_height = gallery.getHeight() - 12;
    const int ratio_width  = ratio_height / 9 * 16;
    Component &content     = *gallery.getViewedComponent();

    content.removeAllChildren();

    if(theme.getId() != "default")
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

    const String website_url  = theme->getThemeMeta()->getWebsite();
    const String license_url  = theme->getThemeMeta()->getLicense().second;
    const String license_text = theme->getThemeMeta()->getLicense().first;

    if(URL::isProbablyAWebsiteURL(website_url))
    {
        buttonWebsiteLink.setMouseCursor(MouseCursor::PointingHandCursor);
        buttonWebsiteLink.setButtonText(website_url);
        buttonWebsiteLink.setURL(website_url);
        buttonWebsiteLink.setVisible(true);
    }
    else
    {
        buttonWebsiteLink.setMouseCursor(MouseCursor::NormalCursor);
        buttonWebsiteLink.setButtonText("N/A");
        buttonWebsiteLink.setURL(String());
        buttonWebsiteLink.setVisible(false);
    }

    if(URL::isProbablyAWebsiteURL(license_url))
    {
        buttonLicenseLink.setMouseCursor(MouseCursor::PointingHandCursor);
        buttonLicenseLink.setButtonText(license_text.isEmpty() ? "Link" : license_text);
        buttonLicenseLink.setURL(license_url);
        buttonLicenseLink.setVisible(true);
    }
    else
    {
        buttonLicenseLink.setMouseCursor(MouseCursor::NormalCursor);
        buttonLicenseLink.setButtonText(license_text.isEmpty() ? "N/A" : license_text);
        buttonLicenseLink.setURL(String());
        buttonLicenseLink.setVisible(false);
    }

    repaint();
}
#endif // ThemePreview



/* ==================================================================================
 * =================================== ThemePanel ===================================
 * ================================================================================== */
#if(1) // ThemePanel
OptionPanelThemes::ThemePanel::ThemePanel(CossinAudioProcessorEditor &editor)
    : editor(editor), selectedTheme(0), selectedRow(0), previewBox(*this),
      themeList("", this)
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

    if(theme && theme->isValid())
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
        g.drawText("N/A", 6, 0, width, height, Justification::centredLeft);
    }
}

void OptionPanelThemes::ThemePanel::listBoxItemClicked(int row, const MouseEvent&)
{
    if(selectedRow != row)
    {
        previewBox.updateContent(themes.at(row));
        changeButtonState();
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

    if(themes.size() > selected_row)
    {
        MouseCursor::showWaitCursor();

        editor.reloadTheme(themes.at(selected_row));
        selectedTheme = selected_row;
        buttonApply.setEnabled(false);
        editor.repaint();

        MouseCursor::hideWaitCursor();
    }
}

void OptionPanelThemes::ThemePanel::changeButtonState()
{
    const int selected_row = themeList.getSelectedRow();
    const bool is_selected = themeList.getSelectedRow() == selectedTheme;
    buttonApply.setEnabled(!is_selected);
    buttonApply.setMouseCursor(!is_selected ? MouseCursor::PointingHandCursor : MouseCursor::NormalCursor);
}
#endif // ThemePanel



/* ==================================================================================
 * ================================ OptionPanelThemes ===============================
 * ================================================================================== */
#if (1) // OptionPanelThemes
OptionPanelThemes::OptionPanelThemes(CossinAudioProcessorEditor &editor)
    : editor(editor), themePanel(editor)
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
void OptionPanelThemes::selectThemeRow(const jaut::ThemePointer &theme)
{
    const auto iterator = std::find(themePanel.themes.begin(), themePanel.themes.end(), theme);

    if(theme.getId() == "default" || iterator == themePanel.themes.end())
    {
        themePanel.themeList.selectRow(0);
    }
    else
    {
        themePanel.themeList.selectRow(std::distance(themePanel.themes.begin(), iterator));
    }

    const int selected_row = themePanel.themeList.getSelectedRow();

    themePanel.previewBox.updateContent(theme);
    themePanel.selectedRow   = selected_row;
    themePanel.selectedTheme = selected_row;
    themePanel.changeButtonState();
}

void OptionPanelThemes::resetThemeList(const std::vector<jaut::ThemePointer> &themes)
{
    themePanel.themes = themes;
    themePanel.themeList.updateContent();
    repaint();
}

//======================================================================================================================
const jaut::ThemePointer &OptionPanelThemes::getSelectedTheme() const noexcept
{
    return themePanel.themes.at(themePanel.selectedTheme);
}

//======================================================================================================================
void OptionPanelThemes::reloadLocale(const jaut::Localisation &locale)
{
    themePanel.buttonApply.setButtonText(locale.translate("options.category.themes.apply"));
    themePanel.previewBox.labelNoPreview.setText(locale.translate("options.category.themes.no_preview"),
                                                 NotificationType::sendNotificationAsync);
}

void OptionPanelThemes::reloadTheme(const jaut::ThemePointer &theme)
{
    const Colour colour_font = theme->getThemeColour(res::Col_Font);

    themePanel.font = theme->getThemeFont();
    themePanel.themeList.setColour(ListBox::backgroundColourId, theme->getThemeColour(res::Col_Container_Bg));
    themePanel.themeList.setColour(ListBox::textColourId, colour_font);
    themePanel.imageApply = theme->getImage(res::Png_Play);
    themePanel.previewBox.buttonWebsiteLink.setFont(themePanel.font.withHeight(14.0f), false, Justification::left);
    themePanel.previewBox.buttonLicenseLink.setFont(themePanel.font.withHeight(14.0f), false, Justification::left);
    themePanel.previewBox.labelNoPreview.setFont(themePanel.font.withHeight(14.0f).withStyle(Font::bold));
    themePanel.previewBox.labelNoPreview.setColour(Label::outlineColourId, colour_font);
    themePanel.previewBox.labelNoPreview.setColour(Label::textColourId,    colour_font);
    selectThemeRow(theme);

    auto iterator = std::find(themePanel.themes.begin(), themePanel.themes.end(), theme);

    if(iterator != themePanel.themes.end())
    {
        themePanel.selectedTheme = std::distance(themePanel.themes.begin(), iterator);
    }
}

#endif // OptionPanelThemes
#endif // CategoryThemes
