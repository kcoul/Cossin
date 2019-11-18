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
    @file   PluginStyle.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "PluginStyle.h"

#include "PluginEditor.h"
#include "Resources.h"

//======================================================================================================================
PluginStyle::PluginStyle() noexcept
    : MetreLookAndFeel(*this),
      theme(nullptr),
      formatter(jaut::CharFormat::Options{'&', Colours::transparentBlack})
{}

PluginStyle::~PluginStyle()
{}

//======================================================================================================================
void PluginStyle::drawRotarySlider(Graphics &g, int x, int y, int width, int height, float sliderPosProportional,
                                   float rotaryStartAngle, float rotaryEndAngle, Slider &slider)
{
    const NamedValueSet &sliderprops = slider.getProperties();

    if (sliderprops.contains("CSSize") || sliderprops.contains("CSType"))
    {
        const bool isbigknob   = sliderprops["CSSize"].toString().equalsIgnoreCase("big");
        const bool ishalfknob  = sliderprops["CSType"].toString().equalsIgnoreCase("half");
        const Image &imgknob   = isbigknob ? imgKnobBig       : imgKnobSmall;
        const Image &imgcursor = isbigknob ? imgKnobBigCursor : imgKnobSmallCursor;
        Path p;

        if (ishalfknob)
        {
            const float startRad = 0.0f;
            const float endRad   = startRad + 2.38f * (sliderPosProportional * 2 - 1);
            p.addPieSegment(0.0f, 0.0f, JT_FIX_F(width), JT_FIX_F(height), startRad, endRad, 0.85f);
        }
        else
        {
            p.addPieSegment(0.0f, 0.0f, JT_FIX_F(width), JT_FIX_F(height), -2.38f,
                            -2.38f + (2.38f * 2 * sliderPosProportional), 0.85f);
        }

        g.drawImageAt(imgknob, 0, 0);
        g.drawImageTransformed(imgcursor, AffineTransform::rotation(-2.35f + (4.7f * sliderPosProportional),
                               width / 2.0f, height / 2.0f));

        g.setColour(findColour(CossinAudioProcessorEditor::ColourComponentForegroundId));
        g.fillPath(p);

        return;
    }

    LookAndFeel_V4::drawRotarySlider(g, x, y, width, height, sliderPosProportional,
                                     rotaryStartAngle, rotaryEndAngle, slider);
}

void PluginStyle::drawCornerResizer(Graphics &g, int width, int height, bool, bool mouseDragging)
{
    const Rectangle<float> resizerrect(0.0f, 0.0f, width * 1.5f, height * 1.0f);

    {
        Graphics::ScopedSaveState sss(g);

        g.addTransform(AffineTransform::translation(0.0f, height + 0.8f));
        g.addTransform(AffineTransform::rotation(-0.785398f));
        g.setColour(findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
        g.fillRect(resizerrect);
    }

    g.addTransform(AffineTransform::translation(0.0f, height + 4.0f));
    g.addTransform(AffineTransform::rotation(-0.785398f));
    g.setColour(mouseDragging ? findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                              : findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId));
    g.fillRect(resizerrect);
}

void PluginStyle::drawTooltip(Graphics &g, const String &text, int width, int height)
{
    const float tooltipfontsize = 13.0f;
    const int tooltippadding    = 2;

    g.fillAll(findColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId));
    g.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipBorderId));
    g.drawRect(0, 0, width, height);

    g.setFont(font.withHeight(tooltipfontsize));
    formatter.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipFontId));
    formatter.drawText(g, text, tooltippadding * 2, tooltippadding, width, height, Justification::topLeft);
}

void PluginStyle::drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour,
                                       bool mouseOver, bool mouseDown)
{
    g.setColour(mouseOver ? findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                          : findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId));
    g.fillAll();
}

void PluginStyle::drawButtonText(juce::Graphics &g, juce::TextButton &button, bool mouseOver, bool)
{
    g.setFont(font.withHeight(14.0f));
    g.setColour(mouseOver ? findColour(CossinAudioProcessorEditor::ColourComponentForegroundId).contrasting()
                          : findColour(CossinAudioProcessorEditor::ColourFontId)
                            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

    const int indent_y    = jmin (4, button.proportionOfHeight (0.3f));
    const int corner_size = jmin (button.getHeight(), button.getWidth()) / 2;

    const int font_height  = roundToInt (font.getHeight() * 0.6f);
    const int indent_left  = jmin(font_height, 2 + corner_size / (button.isConnectedOnLeft() ? 4 : 2));
    const int indent_right = jmin(font_height, 2 + corner_size / (button.isConnectedOnRight() ? 4 : 2));
    const int text_width   = button.getWidth() - indent_left - indent_right;

    if (text_width > 0)
    {
        g.drawFittedText(button.getButtonText(), indent_left, indent_y, text_width, button.getHeight() - indent_y * 2,
                         Justification::centred, 2);
    }
}

//======================================================================================================================
void PluginStyle::drawAlertBox(Graphics &g, AlertWindow &window, const Rectangle<int> &textArea, TextLayout &layout)
{
    const int iconWidth         = 80;
    const Rectangle<int> bounds = window.getLocalBounds();
    int iconSize                = jmin (iconWidth + 50, bounds.getHeight() + 20);
    Rectangle<int> iconRect(iconSize / -10, iconSize / -10, iconSize, iconSize);
    Path icon;
    juce_wchar character;
    uint32 colour;

    g.setColour(findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillAll();

    if (window.containsAnyExtraComponents() || window.getNumButtons() > 2)
    {
        iconSize = jmin(iconSize, textArea.getHeight() + 50);
    }

    if (window.getAlertType() == AlertWindow::WarningIcon)
    {
        character = '!';

        icon.addTriangle(iconRect.getX() + iconRect.getWidth() * 0.5f, (float)iconRect.getY(),
                         static_cast<float>(iconRect.getRight()), static_cast<float>(iconRect.getBottom()),
                         static_cast<float>(iconRect.getX()),     static_cast<float>(iconRect.getBottom()));

        icon   = icon.createPathWithRoundedCorners (5.0f);
        colour = 0x66ff2a00;
    }
    else if (window.getAlertType() == AlertWindow::NoIcon)
    {
        colour    = Colour(0xff6666b9).withAlpha(0.4f).getARGB();
        character = 'x';
        icon.addEllipse(iconRect.toFloat());
    }
    else
    {
        colour    = Colour(0xff00b0b9).withAlpha(0.4f).getARGB();
        character = window.getAlertType() == AlertWindow::InfoIcon ? 'i' : '?';
        icon.addEllipse(iconRect.toFloat());
    }

    GlyphArrangement ga;
    ga.addFittedText({iconRect.getHeight() * 0.9f, Font::bold}, String::charToString(character),
                      static_cast<float>(iconRect.getX()),     static_cast<float>(iconRect.getY()),
                      static_cast<float>(iconRect.getWidth()), static_cast<float>(iconRect.getHeight()),
                      Justification::centred, false);
    
    ga.createPath(icon);
    icon.setUsingNonZeroWinding(false);
    g.setColour(Colour(colour));
    g.fillPath(icon);

    g.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipBorderId));
    g.drawRect(bounds);

    g.setColour(findColour(CossinAudioProcessorEditor::ColourFontId));
    g.setFont(getAlertWindowFont());
    layout.draw(g, textArea.toFloat().withPosition(50.0f, 50.0f));
}

int PluginStyle::getAlertWindowButtonHeight()
{
    return 30;
}

Font PluginStyle::getAlertWindowTitleFont()
{
    return font.withStyle(Font::bold).withHeight(18.0f);
}

Font PluginStyle::getAlertWindowMessageFont()
{
    return font.withHeight(16.0f);
}

Font PluginStyle::getAlertWindowFont()
{
    return font;
}

//======================================================================================================================
int PluginStyle::getOptionListLabelWidth(const String &label)
{
    return font.withHeight(13.0f).getStringWidth(label);
}

void PluginStyle::drawOptionListOptionBox(Graphics &g, Rectangle<int> bounds, bool isCheckBox, bool checked,
                                          bool enabled, bool isMouseOver, bool isMouseDown)
{
    const Image &box = checked ? imgCheckboxChecked : imgCheckbox;
    g.drawImageAt(box, 0, 0);

    if(!checked && isMouseOver && enabled)
    {
        g.setOpacity(0.3f);
        g.drawImageAt(imgCheckboxChecked, 0, 0);
    }
    else if(!enabled)
    {
        g.setOpacity(1.0f);
        g.setColour(findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId).withAlpha(0.3f));
        g.fillRect(bounds);
    }
}

void PluginStyle::drawOptionListOptionLabel(Graphics &g, const String &label, Rectangle<int> bounds, bool isCheckBox,
                                            bool isRightAligned, bool checked, bool enabled, bool isMouseOver,
                                            bool isMouseDown)
{
    g.setFont(font.withHeight(13.0f));
    jaut::LookAndFeel::drawOptionListOptionLabel(g, label, bounds, isCheckBox, isRightAligned, checked, enabled,
                                                 isMouseOver, isMouseDown);
}

//======================================================================================================================
void PluginStyle::drawScrollbar(juce::Graphics &g, juce::ScrollBar &scrollBar, int x, int y, int width, int height,
                                bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool)
{
    Rectangle<int> thumb_bounds;

    if (isScrollbarVertical)
    {
        thumb_bounds = { x, thumbStartPosition, width, thumbSize };
    }
    else
    {
        thumb_bounds = { thumbStartPosition, y, thumbSize, height };
    }

    g.setColour(isMouseOver ? findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                            : findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId));
    g.fillRect(thumb_bounds.reduced(1));
}

//======================================================================================================================
void PluginStyle::reset(const jaut::ThemeManager::ThemePointer &theme) noexcept
{
    if(!theme || theme == this->theme)
    {
        return;
    }

    const Image imgcb = theme->getImage(res::Png_Check_Box);
    const Image imgks = theme->getImage(res::Png_Knob_Small);
    const Image imgkb = theme->getImage(res::Png_Knob_Big);

    imgCheckbox        = imgcb.getClippedImage({0, 0, 16, 16});
    imgCheckboxChecked = imgcb.getClippedImage({0, 16, 16, 16});
    imgKnobBig         = imgkb.getClippedImage({0, 0, 60, 60});
    imgKnobBigCursor   = imgkb.getClippedImage({60, 0, 60, 60});
    imgKnobSmall       = imgks.getClippedImage({0, 0, 36, 36});
    imgKnobSmallCursor = imgks.getClippedImage({36, 0, 36, 36});
    imgSliderPeakMetre = theme->getImage(res::Png_Metre_H);

    setColour(CossinAudioProcessorEditor::ColourComponentBackgroundId, theme->getThemeColour(res::Col_Component_Bg));
    setColour(CossinAudioProcessorEditor::ColourComponentForegroundId, theme->getThemeColour(res::Col_Component_Fg));
    setColour(CossinAudioProcessorEditor::ColourContainerBackgroundId, theme->getThemeColour(res::Col_Container_Bg));
    setColour(CossinAudioProcessorEditor::ColourContainerForegroundId, theme->getThemeColour(res::Col_Container_Fg));
    setColour(CossinAudioProcessorEditor::ColourHeaderBackgroundId,    theme->getThemeColour(res::Col_Header_Bg));
    setColour(CossinAudioProcessorEditor::ColourTooltipBorderId,       theme->getThemeColour(res::Col_Tooltip_Border));
    setColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId,   theme->getThemeColour(res::Col_Tooltip_Bg));
    setColour(CossinAudioProcessorEditor::ColourTooltipFontId,         theme->getThemeColour(res::Col_Tooltip_Font));
    setColour(CossinAudioProcessorEditor::ColourFontId,                theme->getThemeColour(res::Col_Font));

    //override look and feel colours
    // juce::AlertWindow
    setColour(AlertWindow::textColourId, theme->getThemeColour(res::Col_Font));

    // FFAU::LevelMeter
    setColour(FFAU::LevelMeter::lmTextColour,             theme->getThemeColour(res::Col_Font));
    setColour(FFAU::LevelMeter::lmTextClipColour,         Colours::transparentBlack);
    setColour(FFAU::LevelMeter::lmTextDeactiveColour,     theme->getThemeColour(res::Col_Container_Fg));
    setColour(FFAU::LevelMeter::lmTicksColour,            juce::Colours::orange);
    setColour(FFAU::LevelMeter::lmOutlineColour,          juce::Colours::orange);
    setColour(FFAU::LevelMeter::lmBackgroundColour,       Colours::transparentBlack);
    setColour(FFAU::LevelMeter::lmBackgroundClipColour,   juce::Colours::red);
    setColour(FFAU::LevelMeter::lmMeterForegroundColour,  juce::Colours::green);
    setColour(FFAU::LevelMeter::lmMeterOutlineColour,     Colours::transparentBlack);
    setColour(FFAU::LevelMeter::lmMeterBackgroundColour,  theme->getThemeColour(res::Col_Container_Bg));
    setColour(FFAU::LevelMeter::lmMeterMaxNormalColour,   juce::Colours::lightgrey);
    setColour(FFAU::LevelMeter::lmMeterMaxWarnColour,     juce::Colours::orange);
    setColour(FFAU::LevelMeter::lmMeterMaxOverColour,     juce::Colours::darkred);
    setColour(FFAU::LevelMeter::lmMeterGradientLowColour, theme->getThemeColour(res::Col_Component_Bg));
    setColour(FFAU::LevelMeter::lmMeterGradientMidColour, theme->getThemeColour(res::Col_Component_Bg));
    setColour(FFAU::LevelMeter::lmMeterGradientMaxColour, theme->getThemeColour(res::Col_Component_Fg));
    setColour(FFAU::LevelMeter::lmMeterReductionColour,   juce::Colours::orange);

    // jaut::OptionList
    setColour(jaut::OptionList::ColourOptionLabelId, theme->getThemeColour(res::Col_Font));

    // jaut::CharFormat
    setColour(jaut::CharFormat::ColourFormat0Id, theme->getThemeColour(res::Col_Format_0));
    setColour(jaut::CharFormat::ColourFormat1Id, theme->getThemeColour(res::Col_Format_1));
    setColour(jaut::CharFormat::ColourFormat2Id, theme->getThemeColour(res::Col_Format_2));
    setColour(jaut::CharFormat::ColourFormat3Id, theme->getThemeColour(res::Col_Format_3));
    setColour(jaut::CharFormat::ColourFormat4Id, theme->getThemeColour(res::Col_Format_4));
    setColour(jaut::CharFormat::ColourFormat5Id, theme->getThemeColour(res::Col_Format_5));
    setColour(jaut::CharFormat::ColourFormat6Id, theme->getThemeColour(res::Col_Format_6));
    setColour(jaut::CharFormat::ColourFormat7Id, theme->getThemeColour(res::Col_Format_7));
    setColour(jaut::CharFormat::ColourFormat8Id, theme->getThemeColour(res::Col_Format_8));
    setColour(jaut::CharFormat::ColourFormat9Id, theme->getThemeColour(res::Col_Format_9));
    setColour(jaut::CharFormat::ColourFormatAId, theme->getThemeColour(res::Col_Format_a));
    setColour(jaut::CharFormat::ColourFormatBId, theme->getThemeColour(res::Col_Format_b));
    setColour(jaut::CharFormat::ColourFormatCId, theme->getThemeColour(res::Col_Format_c));
    setColour(jaut::CharFormat::ColourFormatDId, theme->getThemeColour(res::Col_Format_d));
    setColour(jaut::CharFormat::ColourFormatEId, theme->getThemeColour(res::Col_Format_e));
    setColour(jaut::CharFormat::ColourFormatFId, theme->getThemeColour(res::Col_Format_f));

    font = theme->getThemeFont();
    MetreLookAndFeel::reloadResources();
    
    this->theme = theme;
}

const jaut::ThemeManager::ThemePointer PluginStyle::getTheme() const noexcept
{
    return theme;
}

//======================================================================================================================
const Font &PluginStyle::getFont() const noexcept
{
    return font;
}

Font PluginStyle::getFont(float newHeight, int newStyleFlags, float newHorizontalScale,
                          float newKerningAmount) const noexcept
{
    return font.withHeight(newHeight).withStyle(newStyleFlags).withHorizontalScale(newHorizontalScale)
               .withExtraKerningFactor(newKerningAmount);
}
