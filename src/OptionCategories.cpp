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

#include "CossinMain.h"
#include "PluginEditor.h"
#include "PluginStyle.h"
#include "SharedData.h"
#include "ThemeFolder.h"
#include <jaut/appdata.h>
#include <jaut/config.h>
#include <jaut/fontformat.h>
#include <jaut/localisation.h>

#include <numeric>

#pragma region Namespace
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

inline Rectangle<int> getMaxUseableArea() noexcept
{
    int window_max_area       = 0;
    int window_max_width      = 0;
    int window_max_height = 0;

    for (auto display : Desktop::getInstance().getDisplays().displays)
    {
        const Rectangle<int> user_area = display.userArea;
        const int area                 = user_area.getWidth() * user_area.getHeight();

        if (area >= window_max_area)
        {
            window_max_area   = area;
            window_max_width  = user_area.getWidth();
            window_max_height = user_area.getHeight();
        }
    }

    return {window_max_width, window_max_height};
}

struct Resolution
{
    int width;
    int height;
    int ratioWidth;
    int ratioHeight;
    String displayName;

    //==================================================================================================================
    static Resolution getResolutionFromName(const String &name) noexcept
    {
        Resolution resolution;

        if(name == "720p")
        {
            resolution = {1280, 720, 16, 9, name};
        }
        else if(name == "1080p")
        {
            resolution = {1920, 1080, 16, 9, name};
        }
        else if(name == "4k")
        {
            resolution = {3840, 2160, 16, 9, name};
        }
        else if(name == "8k")
        {
            resolution = {7680, 4320, 16, 9, name};
        }
        else
        {
            resolution = {0, 0, 1, 1, "custom"};
        }

        return resolution;
    }

    static Resolution getResolutionFromSize(int width, int height) noexcept
    {
        const int ratio_gcd = std::gcd(width, height);
        int ratio_width  = 16;
        int ratio_height = 9;
        String name;

        if(width == 1280 && height == 720)
        {
            name = "720p";
        }
        else if(width == 1920 && height == 1080)
        {
            name = "1080p";
        }
        else if(width == 3840 && height == 2160)
        {
            name = "4k";
        }
        else if(width == 7680 && height == 4320)
        {
            name = "8k";
        }
        else
        {
            name = "custom";
            ratio_width  = width  / ratio_gcd;
            ratio_height = height / ratio_gcd;
        }
        
        return {width, height, ratio_width, ratio_height, name};
    }

    //==================================================================================================================
    inline bool canFit(const String &name) const noexcept
    {
        const Resolution resolution = getResolutionFromName(name);
        return width >= resolution.width && height >= resolution.height;
    }

    inline String ratioToString() const noexcept
    {
        return String(ratioWidth) + ":" + String(ratioHeight);
    }

    inline int getNewWidth(int height) const noexcept
    {
        return height / ratioHeight * ratioWidth;
    }

    inline int getNewHeight(int width) const noexcept
    {
        return width / ratioWidth * ratioHeight;
    }

    void setNewWidth(int width) noexcept
    {
        if(this->width != width)
        {
            this->width = width;
            height = getNewHeight(width);
        }
    }

    void setNewHeight(int height) noexcept
    {
        if(this->height != height)
        {
            this->height = height;
            width = getNewWidth(height);
        }
    }
};

inline void addResolutionIfApplicable(ComboBox &resolutionBox, const Resolution &targetResolution,
                                  const String &resolutionName) noexcept
{
    if(targetResolution.canFit(resolutionName))
    {
        const int last_id = resolutionBox.getNumItems() + 1;
        resolutionBox.addItem(resolutionName, last_id);
    }
}
}
#pragma endregion Namespace



/** Category General Region Start */
#pragma region CategoryGeneral
#pragma region PanelDefaults
/* ==================================================================================
 * ================================= PanelDefaults ==================================
 * ================================================================================== */
OptionPanelGeneral::PanelDefaults::PanelDefaults(OptionPanelGeneral &panel)
    : panel(panel)
{
    for(int i = 0; i < res::Pan_Modes_Num; ++i)
    {
        boxPanningLaw.addItem(res::Pan_Modes[i], i + 1);
    }
    addAndMakeVisible(boxPanningLaw);

    for(int i = 0; i < res::Proc_Types_Num; ++i)
    {
        boxProcessor.addItem(res::Proc_Types[i], i + 1);
    }
    addAndMakeVisible(boxProcessor);

    const Rectangle max_area      = ::getMaxUseableArea();
    const ::Resolution resolution = ::Resolution::getResolutionFromSize(max_area.getWidth(), max_area.getHeight());

    boxSize.addItem("Minimum", 1);
    ::addResolutionIfApplicable(boxSize, resolution, "720p");
    ::addResolutionIfApplicable(boxSize, resolution, "1080p");
    ::addResolutionIfApplicable(boxSize, resolution, "4k");
    ::addResolutionIfApplicable(boxSize, resolution, "8k");

    int id_count = boxSize.getNumItems() + 1;
    boxSize.addItem("Maximum", id_count++);
    boxSize.addItem("Custom", id_count);
    boxSize.addListener(this);
    addAndMakeVisible(boxSize);

    boxWindowWidth.addListener(this);
    boxWindowWidth.setInputFilter(this, false);
    addAndMakeVisible(boxWindowWidth);

    boxWindowHeight.addListener(this);
    boxWindowHeight.setInputFilter(this, false);
    addAndMakeVisible(boxWindowHeight);

    boxRatio.addListener(this);
    boxRatio.setInputFilter(this, false);
    addAndMakeVisible(boxRatio);
}

//======================================================================================================================
void OptionPanelGeneral::PanelDefaults::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setFont(panel.font.withHeight(13.0f));
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    jaut::FontFormat::drawSmallCaps(g, panel.locale.translate("options.category.general.default_panning"),
                                    0, 0, getWidth(), 13, Justification::centredLeft);
    jaut::FontFormat::drawSmallCaps(g, panel.locale.translate("options.category.general.default_unit"),
                                    0, 60, getWidth(), 13, Justification::centredLeft);
    jaut::FontFormat::drawSmallCaps(g, panel.locale.translate("options.category.general.default_size"),
                                    0, 120, getWidth(), 13, Justification::centredLeft);
}

void OptionPanelGeneral::PanelDefaults::paintOverChildren(Graphics &g)
{
    if(boxSize.getSelectedItemIndex() != boxSize.getNumItems() - 1)
    {
        const LookAndFeel &lf = getLookAndFeel();

        g.setColour(Colours::black);
        g.setOpacity(0.4f);
        g.fillRect(0, 172, 256, 25);
    }
}

void OptionPanelGeneral::PanelDefaults::resized()
{
    boxPanningLaw.setBounds(0, 17, 200, 30);
    boxProcessor .setBounds(0, 77, 200, 30);
    boxSize      .setBounds(0, 137, 200, 30);
    boxWindowWidth .setBounds(0, 172, 97, 25);
    boxWindowHeight.setBounds(103, 172, 97, 25);
    boxRatio       .setBounds(206, 172, 50, 25);
}

//======================================================================================================================
void OptionPanelGeneral::PanelDefaults::mouseWheelMove(const juce::MouseEvent &e, const juce::MouseWheelDetails &wheel)
{}

//======================================================================================================================
String OptionPanelGeneral::PanelDefaults::filterNewText(TextEditor &editor, const String &newInput)
{
    if(&editor == &boxRatio)
    {
        if(newInput.containsChar(':') && editor.getText().containsChar(':'))
        {
            return "";
        }

        return newInput.retainCharacters("0123456789:");
    }
    else
    {
        if(newInput[0] == '0' && editor.getCaretPosition() == 0)
        {
            return "";
        }

        return newInput.retainCharacters("0123456789")
                       .substring(0, 5 - (editor.getTotalNumChars() - editor.getHighlightedRegion().getLength()));
    }

}

//======================================================================================================================
void OptionPanelGeneral::PanelDefaults::comboBoxChanged(ComboBox *box)
{
    const bool is_custom = box->getSelectedItemIndex() == box->getNumItems() - 1;
    ::Resolution resolution;

    if(box->getSelectedId() == 1)
    {
        resolution = ::Resolution::getResolutionFromSize(Const_WindowDefaultWidth, Const_WindowDefaultHeight);
    }
    else if(box->getSelectedItemIndex() == box->getNumItems() - 2)
    {
        const Rectangle max_area = ::getMaxUseableArea();
        resolution = ::Resolution::getResolutionFromSize(max_area.getWidth(), max_area.getHeight());
    }
    else if(box->getSelectedItemIndex() != box->getNumItems() - 1)
    {
        resolution = ::Resolution::getResolutionFromName(box->getText());
    }
    else
    {
        resolution = ::Resolution::getResolutionFromSize(boxWindowWidth .getText().getIntValue(),
                                                         boxWindowHeight.getText().getIntValue());
    }

    boxWindowWidth.setText(String(resolution.width));
    boxWindowWidth.setEnabled(is_custom);

    boxWindowHeight.setText(String(resolution.height));
    boxWindowHeight.setEnabled(is_custom);

    boxRatio.setText(resolution.ratioToString());
    boxRatio.setEnabled(is_custom);

    repaint(0, 172, 246, 25);
}

void OptionPanelGeneral::PanelDefaults::textEditorTextChanged(TextEditor &editor)
{
    if(&editor == &boxRatio)
    {
        const String text = editor.getText();

        if(text.startsWithChar('0'))
        {
            editor.setText(text.trimCharactersAtStart("0"));
        }
        else if(text.matchesWildcard("*:0*", true))
        {
            editor.setText(text.replace(":0", ":"));
        }

        if(text.endsWithChar(':'))
        {
            editor.setText(text + "1");
        }
        else if(text.startsWithChar(':'))
        {
            editor.setText("1" + text);
        }
        else if(!text.isEmpty() && !text.containsChar(':'))
        {
            editor.setText("");
        }
    }
    else if(&editor == &boxWindowWidth && editor.hasKeyboardFocus(true) && !boxRatio.isEmpty())
    {
        StringArray ratio_tokens;
        ratio_tokens.addTokens(boxRatio.getText(), ":", "\"");

        const int width = editor.getText().getIntValue();
        const int new_height = width / ratio_tokens[0].getIntValue() * ratio_tokens[1].getIntValue();

        boxWindowHeight.setText(String(new_height));
    }
    else if(&editor == &boxWindowHeight && editor.hasKeyboardFocus(true) && !boxRatio.isEmpty())
    {
        StringArray ratio_tokens;
        ratio_tokens.addTokens(boxRatio.getText(), ":", "\"");

        const int height = editor.getText().getIntValue();
        const int new_width = height / ratio_tokens[1].getIntValue() * ratio_tokens[0].getIntValue();

        boxWindowWidth.setText(String(new_width));
    }
}

void OptionPanelGeneral::PanelDefaults::textEditorFocusLost(TextEditor &editor)
{
    
}
#pragma endregion PanelDefaults
#pragma region OptionPanelGeneral
/* ==================================================================================
 * =============================== OptionPanelGeneral ===============================
 * ================================================================================== */
OptionPanelGeneral::OptionPanelGeneral(CossinAudioProcessorEditor &editor, jaut::Localisation &locale)
    : OptionCategory(editor, locale), currentLanguageIndex(0), lastSelected(0),
      defaultsBox(*this), languageList("", this)
{
    addAndMakeVisible(defaultsBox);
    addAndMakeVisible(languageList);
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
bool OptionPanelGeneral::saveState(SharedData &sharedData) const
{
    const jaut::Config &config = sharedData.Configuration();

    config.getProperty("panning",   res::Cfg_Defaults).setValue(defaultsBox.boxPanningLaw.getSelectedId() - 1);
    config.getProperty("processor", res::Cfg_Defaults).setValue(defaultsBox.boxProcessor.getSelectedId()  - 1);

    auto property_size = config.getProperty("size", res::Cfg_Defaults);
    property_size.getProperty("width") .setValue(defaultsBox.boxWindowWidth .getText().getIntValue());
    property_size.getProperty("height").setValue(defaultsBox.boxWindowHeight.getText().getIntValue());

    const String selected_language = currentLanguageIndex <= 0 || currentLanguageIndex >= languages.size()
                                     ? "default" : languages.at(currentLanguageIndex).first;
    config.getProperty("language").setValue(selected_language);
    sharedData.Localisation().setCurrentLanguage(locale);

    return true;
}

void OptionPanelGeneral::loadState(const SharedData &sharedData)
{
    reloadConfig(sharedData.Configuration());
    reloadLocale(sharedData.Localisation());
}

//======================================================================================================================
void OptionPanelGeneral::reloadLocale(const jaut::Localisation &locale)
{
    DirectoryIterator iterator(locale.getRootDirectory(), false, "*.lang");

    languages.clear();
    languages.emplace_back("default", "Default");

    while(iterator.next())
    {
        const File language_file = iterator.getFile();
        const String file_name   = language_file.getFileNameWithoutExtension();
        auto lang_data           = jaut::Localisation::getLanguageFileData(language_file);
        languages.emplace_back(file_name, lang_data.first + " - " + lang_data.second.joinIntoString(" "));
    }

    languageList.updateContent();
    selectLangRow(locale.getLanguageFile());

    languageList.setTooltip(locale.translate("tooltip.option.select_language"));
    defaultsBox.boxPanningLaw  .setTooltip(locale.translate("tooltip.option.default_panning"));
    defaultsBox.boxProcessor   .setTooltip(locale.translate("tooltip.option.default_processor"));
    defaultsBox.boxSize        .setTooltip(locale.translate("tooltip.option.default_size"));
    defaultsBox.boxWindowWidth .setTooltip(locale.translate("tooltip.option.default_size.width"));
    defaultsBox.boxWindowHeight.setTooltip(locale.translate("tooltip.option.default_size.height"));
    defaultsBox.boxRatio       .setTooltip(locale.translate("tooltip.option.default_size.ratio"));
}

void OptionPanelGeneral::reloadTheme(const jaut::ThemePointer &theme)
{
    font = theme->getThemeFont();

    const Font text_editor_font = font.withHeight(16.0f);

    defaultsBox.boxWindowWidth .setFont(text_editor_font);
    defaultsBox.boxWindowHeight.setFont(text_editor_font);
    defaultsBox.boxRatio       .setFont(text_editor_font);
}

void OptionPanelGeneral::reloadConfig(const jaut::Config &config)
{
    const int panning_value     = config.getProperty("panning",   res::Cfg_Defaults).getValue();
    const int processor_value   = config.getProperty("processor", res::Cfg_Defaults).getValue();
    const int max_panning_modes = defaultsBox.boxPanningLaw.getNumItems();
    const int max_processors    = defaultsBox.boxProcessor.getNumItems();
    ComboBox &box_pan           = defaultsBox.boxPanningLaw;
    ComboBox &box_proc          = defaultsBox.boxProcessor;

    box_pan .setSelectedId(jaut::fit_s(panning_value, 0, max_panning_modes) ? panning_value   + 1 : 2);
    box_proc.setSelectedId(jaut::fit_s(processor_value, 0, max_processors)  ? processor_value + 1 : 1);

    // size box
    const auto property_size = config.getProperty("size", res::Cfg_Defaults);
    const int window_width   = std::max<int>(property_size.getProperty("width") .getValue(), Const_WindowDefaultWidth);
    const int window_height  = std::max<int>(property_size.getProperty("height").getValue(), Const_WindowDefaultHeight);
    const auto resolution    = ::Resolution::getResolutionFromSize(window_width, window_height);
    ComboBox &box_size       = defaultsBox.boxSize;

    if(resolution.displayName != "custom")
    {
        for(int i = 1; i < box_size.getNumItems() - 2; ++i)
        {
            if(box_size.getItemText(i).equalsIgnoreCase(resolution.displayName))
            {
                box_size.setSelectedItemIndex(i);
                break;
            }
        }
    }
    else if(window_width == Const_WindowDefaultWidth && window_height == Const_WindowDefaultHeight)
    {
        box_size.setSelectedId(1);
    }
    else
    {
        const Rectangle max_area = ::getMaxUseableArea();

        if(window_width == max_area.getWidth() && window_height == max_area.getHeight())
        {
            box_size.setSelectedId(box_size.getNumItems() - 1);
        }
        else
        {
            box_size.setSelectedId(box_size.getNumItems());
        }
    }

    defaultsBox.previousSize = {window_width, window_height};
    defaultsBox.boxWindowWidth .setText(String(window_width));
    defaultsBox.boxWindowHeight.setText(String(window_height));
    defaultsBox.boxRatio       .setText(resolution.ratioToString());
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
        reloadLocale(locale);
    }

    lastSelected = row;
}

//======================================================================================================================
void OptionPanelGeneral::selectLangRow(const File &langFile)
{
    if(langFile.getFullPathName().isEmpty())
    {
        languageList.selectRow(0);
        currentLanguageIndex = 0;
        lastSelected         = 0;

        return;
    }

    const int index = ::getLanguageListIndex(langFile, languages);

    currentLanguageIndex = index;
    lastSelected         = index;
    languageList.selectRow(index);
}
#pragma endregion OptionPanelGeneral
#pragma endregion CategoryGeneral



/** Category Themes Region Start */
#pragma region CategoryThemes
#pragma region ThemePreview
/* ==================================================================================
 * ================================== ThemePreview ==================================
 * ================================================================================== */
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
        const Font &font         = panel.panel.font;
        const Font title_font    = font.withHeight(18.0f).withStyle(Font::bold);
        const Font version_font  = font.withHeight(11.0f);
        const Font content_font  = font.withHeight(14.0f);
        const String title       = theme->getThemeMeta()->getName();
        const String version     = theme->getThemeMeta()->getVersion();
        const String description = theme->getThemeMeta()->getDescription();
        const String author      = theme->getThemeMeta()->getAuthor();
        const String website     = buttonWebsiteLink.getURL().isEmpty() ? buttonWebsiteLink.getButtonText() : "";
        const String license     = buttonLicenseLink.getURL().isEmpty() ? buttonLicenseLink.getButtonText() : "";
        const String authors     = theme->getThemeMeta()->getAuthors().isEmpty() ? "-"
                                   : theme->getThemeMeta()->getAuthors().joinIntoString(", ");
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
        const jaut::Localisation locale = panel.panel.locale;

        g.drawText("Website: " + website, 0, 96,  max_text_width, 14, Justification::left);
        g.drawText(locale.translate("options.category.themes.license") + ": " + license, 0, 114, max_text_width, 14,
                   Justification::left);
        g.drawText(locale.translate("options.category.themes.authors") + ": ", 0, 132, 100, 14, Justification::left);
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
                    screenshot.getProperties()->set("name", screenshot_file.getFileName());

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
            const String screenshot_name = "screenshot00" + String(i);
            const char *image_data = Assets::getNamedResource(String(screenshot_name + "_png").toRawUTF8(),
                                                              image_size);
            Image image = ImageCache::getFromMemory(image_data, image_size);
            image.getProperties()->set("name", screenshot_name);

            ImageComponent &image_component = screenshots[i];
            image_component.setImage(image);
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
#pragma endregion ThemePreview
#pragma region ThemePanel
/* ==================================================================================
 * =================================== ThemePanel ===================================
 * ================================================================================== */
OptionPanelThemes::ThemePanel::ThemePanel(OptionPanelThemes &panel)
    : panel(panel), selectedTheme(0), selectedRow(0), previewBox(*this),
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
        const Font current_font         = panel.font.withHeight(12.0f);
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
        g.setFont(panel.font);
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

        panel.editor.reloadTheme(themes.at(selected_row));
        selectedTheme = selected_row;
        buttonApply.setEnabled(false);
        panel.editor.repaint();

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
#pragma endregion ThemePanel
#pragma region OptionPanelThemes
/* ==================================================================================
 * ================================ OptionPanelThemes ===============================
 * ================================================================================== */
OptionPanelThemes::OptionPanelThemes(CossinAudioProcessorEditor &editor, jaut::Localisation &locale)
    : OptionCategory(editor, locale), themePanel(*this)
{
    addAndMakeVisible(themePanel);
}

//======================================================================================================================
void OptionPanelThemes::resized()
{
    themePanel.setBounds(0, 0, getWidth(), getHeight());
}

//======================================================================================================================
bool OptionPanelThemes::saveState(SharedData &sharedData) const
{
    const int selected_theme       = themePanel.selectedTheme;
    const String selected_theme_id = selected_theme <= 0 || selected_theme >= themePanel.themes.size()
                                     ? "default" : themePanel.themes.at(themePanel.selectedTheme).getId();
            
    if(sharedData.ThemeManager().setCurrentTheme(selected_theme_id))
    {
        sharedData.Configuration().getProperty("theme").setValue(selected_theme_id);
    }
    else
    {
        return false;
    }

    return true;
}

void OptionPanelThemes::loadState(const SharedData &sharedData)
{
    if(themePanel.themes.empty())
    {
        themePanel.themes = sharedData.ThemeManager().getAllThemes();
        themePanel.themeList.updateContent();
    }

    selectThemeRow(sharedData.ThemeManager().getCurrentTheme());
}

//======================================================================================================================
void OptionPanelThemes::reloadLocale(const jaut::Localisation &locale)
{
    themePanel.buttonApply.setButtonText(locale.translate("options.category.themes.apply"));
    themePanel.previewBox.labelNoPreview.setText(locale.translate("options.category.themes.no_preview"),
                                                 NotificationType::sendNotificationAsync);

    themePanel.themeList.setTooltip(locale.translate("tooltip.option.select_theme"));
}

void OptionPanelThemes::reloadTheme(const jaut::ThemePointer &theme)
{
    font = theme->getThemeFont();
    themePanel.previewBox.buttonWebsiteLink.setFont(font, false, Justification::left);
    themePanel.previewBox.buttonLicenseLink.setFont(font, false, Justification::left);
    themePanel.previewBox.labelNoPreview   .setFont(font.withStyle(Font::bold));
    selectThemeRow(theme);

    auto iterator = std::find(themePanel.themes.begin(), themePanel.themes.end(), theme);

    if(iterator != themePanel.themes.end())
    {
        themePanel.selectedTheme = std::distance(themePanel.themes.begin(), iterator);
    }
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
#pragma endregion OptionPanelThemes
#pragma endregion CategoryThemes



/** Category Performance Region Start */
#pragma region CategoryPerformance::OptionPanelPerformance
/* ==================================================================================
 * ============================= OptionPanelPerformance =============================
 * ================================================================================== */
OptionPanelPerformance::OptionPanelPerformance(CossinAudioProcessorEditor &editor, jaut::Localisation &locale)
    : OptionCategory(editor, locale)
{
    boxAnimationMode.addItem(locale.translate("options.category.optimization.animation.none"), 1);
    boxAnimationMode.addItem(locale.translate("options.category.optimization.animation.user"), 2);
    boxAnimationMode.addItem(locale.translate("options.category.optimization.animation.some"), 3);
    boxAnimationMode.addItem(locale.translate("options.category.optimization.animation.all"),  4);
    boxAnimationMode.onChange = [this, &editor]()
    {
        const int id = boxAnimationMode.getSelectedId();

        editor.setOption(Flag_AnimationComponents, id == 4 || (id == 2 && tickControls.getToggleState()));
        editor.setOption(Flag_AnimationEffects,    id  > 2 || (id == 2 && tickEffects .getToggleState()));
    };
    addAndMakeVisible(boxAnimationMode);

    tickControls.addListener(this);
    tickControls.setButtonText(locale.translate("options.category.optimization.animate_ctrl"));
    addAndMakeVisible(tickControls);

    tickEffects.addListener(this);
    tickEffects.setButtonText(locale.translate("options.category.optimization.animate_fx"));
    addAndMakeVisible(tickEffects);

#if COSSIN_USE_OPENGL
    tickHardwareAcceleration.setButtonText(locale.translate("options.category.optimization.use_hardware"));
    tickHardwareAcceleration.setEnabled(editor.isOpenGLSupported());
    addAndMakeVisible(tickHardwareAcceleration);

    tickMultisampling.setButtonText(locale.translate("options.category.optimization.multisampling"));
    tickMultisampling.setEnabled(editor.isOpenGLSupported());
    addAndMakeVisible(tickMultisampling);

    tickSmoothing.setButtonText(locale.translate("options.category.optimization.filter"));
    tickSmoothing.setEnabled(editor.isOpenGLSupported());
    addAndMakeVisible(tickSmoothing);
#endif
}

OptionPanelPerformance::~OptionPanelPerformance() {}

//======================================================================================================================
void OptionPanelPerformance::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

#if COSSIN_USE_OPENGL
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.drawRect(0, 145, getWidth(), 2);
#endif

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    g.setFont(font);

    const String title_animations = locale.translate("options.category.optimization.animation_title");
    jaut::FontFormat::drawSmallCaps(g, title_animations, 6, 0, getWidth(), 27, Justification::centred);

#if COSSIN_USE_OPENGL
    const String title_quality = locale.translate("options.category.optimization.quality_title");
    jaut::FontFormat::drawSmallCaps(g, title_quality, 6, 147, getWidth(), 27, Justification::centred);
#endif

    g.setFont(font.withHeight(13.0f));
    jaut::FontFormat::drawSmallCaps(g, locale.translate("options.category.optimization.animation_mode"),
                                    6, 27, getWidth(), 13, Justification::centredLeft);
}

void OptionPanelPerformance::resized()
{
    const int x_start = 6;
    const int y_start = 44;

    boxAnimationMode.setBounds(x_start, y_start, 200, 30);
    tickControls    .setBounds(x_start, y_start + 40, 200, 16);
    tickEffects     .setBounds(x_start, y_start + 67, 200, 16);

#if COSSIN_USE_OPENGL
    const int hy_start = 169;

    tickHardwareAcceleration.setBounds(x_start, hy_start,      200, 16);
    tickMultisampling       .setBounds(x_start, hy_start + 27, 200, 16);
    tickSmoothing           .setBounds(x_start, hy_start + 54, 200, 16);
#endif
}

//======================================================================================================================
bool OptionPanelPerformance::saveState(SharedData &sharedData) const
{
    jaut::Config &config = sharedData.Configuration();

    auto property_animations = config.getProperty("animations", res::Cfg_Optimization);
    auto property_customs    = property_animations.getProperty("custom");

    property_animations.getProperty("mode")      .setValue(boxAnimationMode.getSelectedId() - 1);
    property_customs   .getProperty("components").setValue(tickControls.getToggleState());
    property_customs   .getProperty("effects")   .setValue(tickEffects .getToggleState());

#if COSSIN_USE_OPENGL
    config.getProperty("hardwareAcceleration", res::Cfg_Optimization)
          .setValue(tickHardwareAcceleration.getToggleState());
    config.getProperty("useMultisampling", res::Cfg_Optimization).setValue(tickMultisampling.getToggleState());
    config.getProperty("textureSmoothing", res::Cfg_Optimization).setValue(tickSmoothing.getToggleState());
#endif

    return true;
}

void OptionPanelPerformance::loadState(const SharedData &sharedData)
{
    reloadConfig(sharedData.Configuration());
}

//======================================================================================================================
void OptionPanelPerformance::reloadTheme(const jaut::ThemePointer &theme)
{
    font = theme->getThemeFont();
}

void OptionPanelPerformance::reloadConfig(const jaut::Config &config)
{
    const auto property_animations = config.getProperty("animations", res::Cfg_Optimization);
    const auto property_custom     = property_animations.getProperty("custom");
    const int  animation_mode      = property_animations.getProperty("mode").getValue();

    boxAnimationMode.setSelectedId(jaut::fit_a(animation_mode, 0, 3) ? animation_mode + 1 : 4);

    tickControls.setToggleState(property_custom.getProperty("components").getValue(),
                                NotificationType::sendNotification);
    tickEffects.setToggleState(property_custom.getProperty("effects").getValue(),
                               NotificationType::sendNotification);

#if COSSIN_USE_OPENGL
    tickHardwareAcceleration.setToggleState(config.getProperty("hardwareAcceleration",
                                                               res::Cfg_Optimization).getValue(),
                                            NotificationType::dontSendNotification);
    tickMultisampling.setToggleState(config.getProperty("useMultisampling", res::Cfg_Optimization).getValue(),
                                     NotificationType::dontSendNotification);
    tickSmoothing.setToggleState(config.getProperty("textureSmoothing", res::Cfg_Optimization).getValue(),
                                 NotificationType::dontSendNotification);
#endif
}

void OptionPanelPerformance::reloadLocale(const jaut::Localisation &locale)
{
    boxAnimationMode.changeItemText(1, locale.translate("options.category.optimization.animation.none"));
    boxAnimationMode.changeItemText(2, locale.translate("options.category.optimization.animation.user"));
    boxAnimationMode.changeItemText(3, locale.translate("options.category.optimization.animation.some"));
    boxAnimationMode.changeItemText(4, locale.translate("options.category.optimization.animation.all"));

    tickControls.setButtonText(locale.translate("options.category.optimization.animate_ctrl"));
    tickEffects .setButtonText(locale.translate("options.category.optimization.animate_fx"));

    boxAnimationMode.setTooltip(locale.translate("tooltip.option.animations"));
    tickControls    .setTooltip(locale.translate("tooltip.option.animation.controls"));
    tickEffects     .setTooltip(locale.translate("tooltip.option.animation.effects"));

#if COSSIN_USE_OPENGL
    tickHardwareAcceleration.setButtonText(locale.translate("options.category.optimization.use_hardware"));
    tickMultisampling       .setButtonText(locale.translate("options.category.optimization.multisampling"));
    tickSmoothing           .setButtonText(locale.translate("options.category.optimization.filter"));

    const String require         = "\n\n&r" + locale.translate("tooltip.option.requires") + "\n";
    const String require_restart = locale.translate("tooltip.option.requires_restart");
    const String requires_all    = require + locale.translate("tooltip.option.requires_ha") + "\n" + require_restart;
    const String requires_one    = require + require_restart;

    tickHardwareAcceleration.setTooltip(locale.translate("tooltip.option.hardware_acceleration")    + requires_one);
    tickMultisampling       .setTooltip(locale.translate("tooltip.option.multisampling")            + requires_all);
    tickSmoothing           .setTooltip(locale.translate("tooltip.option.filtering")                + requires_all);
#endif
}

//======================================================================================================================
void OptionPanelPerformance::buttonStateChanged(Button *button)
{
    if(boxAnimationMode.getSelectedId() != 2)
    {
        return;
    }

    const int flag_index = button == &tickControls ? Flag_AnimationComponents : Flag_AnimationEffects;
    editor.setOption(flag_index, button->getToggleState());
}
#pragma endregion CategoryPerformance::OptionPanelPerformance



/** Category Standalone Region Start */
#pragma region CategoryStandalone
#pragma region DevicePanel
/* ==================================================================================
 * =================================== DevicePanel ==================================
 * ================================================================================== */
OptionPanelStandalone::DevicePanel::DevicePanel(OptionPanelStandalone &panel)
    : panel(panel), deviceManager(panel.plugin.deviceManager)
{
    deviceManager.addChangeListener(this);

    buttonControlPanel.setButtonText(panel.locale.translate("options.category.standalone.show_ct_panel"));
    addAndMakeVisible(buttonControlPanel);

    addAndMakeVisible(boxBufferSize);
    
    auto &types = deviceManager.getAvailableDeviceTypes();

    if(types.size() > 0)
    {
        for (int i = 0; i < types.size(); ++i)
        {
            boxDevice.addItem(types.getUnchecked(i)->getTypeName(), i + 1);
        }
    }

    boxDevice.onChange = [this]()
    {
        if (AudioIODeviceType *type = deviceManager.getAvailableDeviceTypes()[boxDevice.getSelectedId() - 1])
        {
            deviceManager.setCurrentAudioDeviceType(type->getTypeName(), true);
            auto shared_data = SharedData::getInstance();
            this->panel.updateAllData();
        }
    };
    addAndMakeVisible(boxDevice);

    addAndMakeVisible(boxInput);
    addAndMakeVisible(boxOutput);
    addAndMakeVisible(boxSampleRate);

    labelLatency.setText("Latency: 0ms", NotificationType::dontSendNotification);
    labelLatency.setJustificationType(Justification::topRight);
    addAndMakeVisible(labelLatency);
}

OptionPanelStandalone::DevicePanel::~DevicePanel()
{
    deviceManager.removeChangeListener(this);
}

//======================================================================================================================
void OptionPanelStandalone::DevicePanel::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    g.setFont(panel.font.withHeight(13.0f));

    jaut::FontFormat::drawSmallCaps(g, panel.locale.translate("options.category.standalone.device_type"),
                                    0, 0, getWidth(), 13, Justification::left);
    jaut::FontFormat::drawSmallCaps(g, panel.locale.translate("options.category.standalone.device.output"),
                                    0, 60, getWidth(), 13, Justification::left);
    jaut::FontFormat::drawSmallCaps(g, panel.locale.translate("options.category.standalone.device.input"),
                                    0, 120, getWidth(), 13, Justification::left);
    jaut::FontFormat::drawSmallCaps(g, panel.locale.translate("options.category.standalone.sample_rate"),
                                    0, 180, getWidth(), 13, Justification::left);
    jaut::FontFormat::drawSmallCaps(g, panel.locale.translate("options.category.standalone.buffer_size"),
                                    (getWidth() - 8) / 2 + 3, 180, getWidth(), 13, Justification::left);
}

void OptionPanelStandalone::DevicePanel::resized()
{
    const int max_width = getWidth() - 8;
    boxDevice.setBounds(0, 17,  max_width, 30);
    boxOutput.setBounds(0, 77,  max_width, 30);
    boxInput .setBounds(0, 137, max_width, 30);

    const int max_width_half = max_width / 2;
    boxSampleRate.setBounds(0, 197, max_width_half - 2, 30);
    boxBufferSize.setBounds(max_width_half + 3, 197, max_width_half - 3, 30);

    buttonControlPanel.setBounds(0, 232, 100, 30);

    labelLatency.setBounds(0, getHeight() - 20, getWidth() - 8, 14);
}

//======================================================================================================================
void OptionPanelStandalone::DevicePanel::changeListenerCallback(ChangeBroadcaster *source)
{
    panel.updateAllData();
}
#pragma endregion DevicePanel
#pragma region DeviceIOSelector
class OptionPanelStandalone::DeviceIOSelector final : private ChangeListener
{
public:
    DeviceIOSelector(DevicePanel &panel)
        : type(nullptr),
          devicePanel(panel),
          inputDeviceDropDown(panel.boxInput),
          outputDeviceDropDown(panel.boxOutput),
          sampleRateDropDown(panel.boxSampleRate),
          bufferSizeDropDown(panel.boxBufferSize),
          showUIButton(panel.buttonControlPanel)
    {
        devicePanel.deviceManager.addChangeListener(this);
    }

    ~DeviceIOSelector()
    {
        devicePanel.deviceManager.removeChangeListener(this);
    }

    void updateConfig(bool updateOutputDevice, bool updateInputDevice, bool updateSampleRate, bool updateBufferSize)
    {
        auto setup = devicePanel.deviceManager.getAudioDeviceSetup();
        String error;

        if (updateOutputDevice || updateInputDevice)
        {
            if (outputDeviceDropDown.isEnabled())
            {
                setup.outputDeviceName = outputDeviceDropDown.getSelectedId() < 0 ? "" : outputDeviceDropDown.getText();
            }

            if (inputDeviceDropDown.isEnabled())
            {
                setup.inputDeviceName = inputDeviceDropDown.getSelectedId() < 0 ? "" : inputDeviceDropDown.getText();
            }

            if (!type->hasSeparateInputsAndOutputs())
            {
                setup.inputDeviceName = setup.outputDeviceName;
            }
                
            if (updateInputDevice)
            {
                setup.useDefaultInputChannels = true;
            }
            else
            {
                setup.useDefaultOutputChannels = true;
            }

            error = devicePanel.deviceManager.setAudioDeviceSetup(setup, true);

            showCorrectDeviceName(inputDeviceDropDown, true);
            showCorrectDeviceName(outputDeviceDropDown, false);

            updateControlPanelButton();
        }
        else if(updateSampleRate)
        {
            if(sampleRateDropDown.getSelectedId() > 0)
            {
                setup.sampleRate = sampleRateDropDown.getSelectedId();
                error            = devicePanel.deviceManager.setAudioDeviceSetup(setup, true);
            }

            const float latency = (setup.bufferSize / setup.sampleRate) * 1000.0f;
            devicePanel.labelLatency.setText("Latency: " + String(roundf(latency * 100.0f) / 100) + "ms",
                                             NotificationType::dontSendNotification);
        }
        else if(updateBufferSize)
        {
            if(bufferSizeDropDown.getSelectedId() > 0)
            {
                setup.bufferSize = bufferSizeDropDown.getSelectedId();
                error            = devicePanel.deviceManager.setAudioDeviceSetup(setup, true);
            }

            const float latency = (setup.bufferSize / setup.sampleRate) * 1000.0f;
            devicePanel.labelLatency.setText("Latency: " + String(roundf(latency * 100.0f) / 100) + "ms",
                                             NotificationType::dontSendNotification);
        }

        if(error.isNotEmpty())
        {
            const String title = devicePanel.panel.locale.translate("options.category.standalone.error_device");
            AlertWindow::showMessageBoxAsync(AlertWindow::NoIcon, title, error);
        }
    }

    bool showDeviceControlPanel()
    {
        if (auto *device = devicePanel.deviceManager.getCurrentAudioDevice())
        {
            Component modal_window;
            modal_window.setOpaque (true);
            modal_window.addToDesktop (0);
            modal_window.enterModalState();

            return device->showControlPanel();
        }

        return false;
    }

    void showDeviceUIPanel()
    {
        if (showDeviceControlPanel())
        {
            devicePanel.deviceManager.closeAudioDevice();
            devicePanel.deviceManager.restartLastAudioDevice();
            devicePanel.getTopLevelComponent()->toFront(true);
        }
    }

    void updateAllControls(AudioIODeviceType *type)
    {
        if(type)
        {
            this->type = type;
            type->scanForDevices();
        }
        else
        {
            if(!this->type)
            {
                jassertfalse;
                return;
            }
        }

        updateOutputsComboBox();
        updateInputsComboBox();
        updateControlPanelButton();

        if (auto *current_device = devicePanel.deviceManager.getCurrentAudioDevice())
        {
            updateSampleRateComboBox(current_device);
            updateBufferSizeComboBox(current_device);

            const float latency = (current_device->getCurrentBufferSizeSamples()
                                   / current_device->getCurrentSampleRate()) * 1000.0f;
            devicePanel.labelLatency.setText("Latency: " + String(roundf(latency * 100.0f) / 100) + "ms",
                                             NotificationType::dontSendNotification);
        }
        else
        {
            jassert(devicePanel.deviceManager.getCurrentAudioDevice() == nullptr);

            sampleRateDropDown.setEnabled(false);
            bufferSizeDropDown.setEnabled(false);
            devicePanel.labelLatency.setText("Latency: 0ms", NotificationType::dontSendNotification);

            if (outputDeviceDropDown.isEnabled())
            {
                outputDeviceDropDown.setSelectedId(-1, NotificationType::dontSendNotification);
            }

            if (inputDeviceDropDown.isEnabled())
            {
                inputDeviceDropDown.setSelectedId(-1, NotificationType::dontSendNotification);
            }
        }
    }

    void changeListenerCallback(ChangeBroadcaster*) override
    {
        updateAllControls(nullptr);
    }

private:
    AudioIODeviceType *type;
    DevicePanel &devicePanel;

    ComboBox &inputDeviceDropDown;
    ComboBox &outputDeviceDropDown;
    ComboBox &sampleRateDropDown;
    ComboBox &bufferSizeDropDown;

    TextButton &showUIButton;

    void showCorrectDeviceName(ComboBox &box, bool isInput)
    {
        if (box.isEnabled())
        {
            const int index = type->getIndexOfDevice(devicePanel.deviceManager.getCurrentAudioDevice(), isInput);
            box.setSelectedId(index < 0 ? index : index + 1, NotificationType::dontSendNotification);
        }
    }

    void addNamesToDeviceBox(ComboBox &box, bool isInputs)
    {
        const StringArray devices(type->getDeviceNames(isInputs));
        box.clear(NotificationType::dontSendNotification);

        for (int i = 0; i < devices.size(); ++i)
        {
            box.addItem(devices[i], i + 1);
        }

        box.addItem(devicePanel.panel.locale.translate("general.none"), -1);
        box.setSelectedId(-1, NotificationType::dontSendNotification);
    }

    void updateControlPanelButton()
    {
        AudioIODevice *currentDevice = devicePanel.deviceManager.getCurrentAudioDevice();
        showUIButton.setEnabled(false);

        if (currentDevice != nullptr && currentDevice->hasControlPanel())
        {
            showUIButton.setEnabled(true);
            showUIButton.onClick = [this] { showDeviceUIPanel(); };
        }
    }

    void updateOutputsComboBox()
    {
        outputDeviceDropDown.setEnabled(true);
        outputDeviceDropDown.onChange = [this] { updateConfig(true, false, false, false); };
        addNamesToDeviceBox  (outputDeviceDropDown, false);
        showCorrectDeviceName(outputDeviceDropDown, false);
    }

    void updateInputsComboBox()
    {
        if (type->hasSeparateInputsAndOutputs())
        {
            inputDeviceDropDown.setEnabled(true);
            inputDeviceDropDown.onChange = [this] { updateConfig(false, true, false, false); };
            addNamesToDeviceBox(inputDeviceDropDown, true);
        }

        showCorrectDeviceName(inputDeviceDropDown, true);
    }

    void updateSampleRateComboBox(AudioIODevice *currentDevice)
    {
        sampleRateDropDown.setEnabled(true);
        sampleRateDropDown.clear();
        sampleRateDropDown.onChange = nullptr;

        for (double rate : currentDevice->getAvailableSampleRates())
        {
            int int_rate = roundToInt(rate);
            sampleRateDropDown.addItem(String(int_rate) + " Hz", int_rate);
        }

        sampleRateDropDown.setSelectedId(roundToInt(currentDevice->getCurrentSampleRate()),
                                         NotificationType::dontSendNotification);
        sampleRateDropDown.onChange = [this] { updateConfig(false, false, true, false); };
    }

    void updateBufferSizeComboBox(AudioIODevice *currentDevice)
    {
        bufferSizeDropDown.setEnabled(true);
        bufferSizeDropDown.clear();
        bufferSizeDropDown.onChange = nullptr;

        for (auto bs : currentDevice->getAvailableBufferSizes())
        {
            bufferSizeDropDown.addItem(String(bs), bs);
        }

        bufferSizeDropDown.setSelectedId(currentDevice->getCurrentBufferSizeSamples(),
                                         NotificationType::dontSendNotification);
        bufferSizeDropDown.onChange = [this] { updateConfig (false, false, false, true); };
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceIOSelector)
};
#pragma endregion DeviceIOSelector
#pragma region OptionPanelStandalone
/* ==================================================================================
 * ============================== OptionPanelStandalone =============================
 * ================================================================================== */
OptionPanelStandalone::OptionPanelStandalone(CossinAudioProcessorEditor &editor, jaut::Localisation &locale)
    : OptionCategory(editor, locale), plugin(*CossinPluginWrapper::getInstance()), devicePanel(*this)
{
    addAndMakeVisible(devicePanel);

    tickMuteInput.getToggleStateValue().referTo(plugin.getMuteInputValue());
    addAndMakeVisible(tickMuteInput);

    ioSelector.reset(new DeviceIOSelector(devicePanel));
    updateAllData();
}

OptionPanelStandalone::~OptionPanelStandalone() {}

//======================================================================================================================
void OptionPanelStandalone::resized()
{
    const int panel_half = getWidth() / 2;
    devicePanel.setBounds(panel_half, 27, panel_half, getHeight() - 27);
    tickMuteInput.setBounds(6, 44, panel_half, 16);
}

void OptionPanelStandalone::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    g.setFont(font);

    const int panel_half = getWidth() / 2;

    jaut::FontFormat::drawSmallCaps(g, locale.translate("options.category.standalone.audio_title"),
                                    0, 0, panel_half, 27, Justification::centred);
    jaut::FontFormat::drawSmallCaps(g, locale.translate("options.category.standalone.device_title"),
                                    panel_half, 0, panel_half, 27, Justification::centred);
}

//======================================================================================================================
bool OptionPanelStandalone::saveState(SharedData &sharedData) const
{
    std::unique_ptr<const XmlElement> xml(devicePanel.deviceManager.createStateXml());
    jaut::Config &config  = sharedData.Configuration();
    auto property_devices = config.getProperty("devices", res::Cfg_Standalone);

    property_devices.getProperty("output").setValue(xml->getStringAttribute("audioOutputDeviceName"));
    property_devices.getProperty("input") .setValue(xml->getStringAttribute("audioInputDeviceName"));
    config.getProperty("deviceType", res::Cfg_Standalone).setValue(xml->getStringAttribute("deviceType"));
    config.getProperty("sampleRate", res::Cfg_Standalone).setValue(xml->getStringAttribute("audioDeviceRate")
                                                                       .getFloatValue());

    const int buffer_size = xml->getStringAttribute("audioDeviceBufferSize").getIntValue();
    config.getProperty("bufferSize", res::Cfg_Standalone).setValue(buffer_size == 0 ? 512 : buffer_size);
    config.getProperty("muteInput",  res::Cfg_Standalone).setValue(tickMuteInput.getToggleState());

    return true;
}

//======================================================================================================================
void OptionPanelStandalone::updateAllData()
{
    ComboBox &boxDevice = devicePanel.boxDevice;
    boxDevice.setText(devicePanel.deviceManager.getCurrentAudioDeviceType());

    AudioDeviceManager &deviceManager = devicePanel.deviceManager;

    if (devicePanel.audioDeviceSettingsCompType != deviceManager.getCurrentAudioDeviceType())
    {
        devicePanel.audioDeviceSettingsCompType = deviceManager.getCurrentAudioDeviceType();
        const int device_id = !boxDevice.isEnabled() ? 0 : boxDevice.getSelectedId() - 1;

        if (auto *type = deviceManager.getAvailableDeviceTypes()[device_id])
        {
            ioSelector->updateAllControls(type);
        }
    }
}

//======================================================================================================================
void OptionPanelStandalone::reloadTheme(const jaut::ThemePointer &theme)
{
    font = theme->getThemeFont();
    devicePanel.labelLatency.setFont(font);
}

void OptionPanelStandalone::reloadLocale(const jaut::Localisation &locale)
{
    tickMuteInput.setButtonText(locale.translate("options.category.standalone.mute_input"));

    devicePanel.boxInput .changeItemText(-1, locale.translate("general.none"));
    devicePanel.boxOutput.changeItemText(-1, locale.translate("general.none"));
    devicePanel.buttonControlPanel.setButtonText(locale.translate("options.category.standalone.show_ct_panel"));

    tickMuteInput.setTooltip(locale.translate("tooltip.option.standalone.mute_input"));
    devicePanel.buttonControlPanel.setTooltip(locale.translate("tooltip.option.standalone.ctrl_panel"));
}
#pragma endregion OptionPanelStandalone
#pragma endregion CategoryStandalone
