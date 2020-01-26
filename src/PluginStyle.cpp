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
#include "SharedData.h"

//======================================================================================================================
PluginStyle::PluginStyle() noexcept
    : MetreLookAndFeel(*this),
      theme(SharedData::getInstance()->getDefaultTheme()),
      formatter(jaut::CharFormat::Options{'&', Colours::transparentBlack})
{
    formatter.setLookAndFeel(this);
    reset(theme);
}

PluginStyle::~PluginStyle()
{}

//======================================================================================================================
void PluginStyle::drawRotarySlider(Graphics &g, int x, int y, int width, int height, float sliderPosProportional,
                                   float rotaryStartAngle, float rotaryEndAngle, Slider &slider)
{
    const NamedValueSet &sliderprops = slider.getProperties();

    if (sliderprops.contains("CSSize") || sliderprops.contains("CSType"))
    {
        const bool is_big_knob    = sliderprops["CSSize"].toString().equalsIgnoreCase("big");
        const bool is_half_knob   = sliderprops["CSType"].toString().equalsIgnoreCase("half");
        const Image &image_knob   = is_big_knob ? imgKnobBig       : imgKnobSmall;
        const Image &image_cursor = is_big_knob ? imgKnobBigCursor : imgKnobSmallCursor;
        Path p;

        if (is_half_knob)
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

        g.drawImageAt(image_knob, 0, 0);
        g.drawImageTransformed(image_cursor, AffineTransform::rotation(-2.35f + (4.7f * sliderPosProportional),
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
    const Rectangle<float> rect_resize(0.0f, 0.0f, width * 1.5f, height * 1.0f);

    {
        Graphics::ScopedSaveState state(g);

        g.addTransform(AffineTransform::translation(0.0f, height + 0.8f));
        g.addTransform(AffineTransform::rotation(-0.785398f));
        g.setColour(findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
        g.fillRect(rect_resize);
    }

    g.addTransform(AffineTransform::translation(0.0f, height + 4.0f));
    g.addTransform(AffineTransform::rotation(-0.785398f));
    g.setColour(mouseDragging ? findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                              : findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId));
    g.fillRect(rect_resize);
}

void PluginStyle::drawTooltip(Graphics &g, const String &text, int width, int height)
{
    const float tooltip_font_size = 13.0f;
    const int tooltip_padding     = 2;

    g.fillAll(findColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId));
    g.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipBorderId));
    g.drawRect(0, 0, width, height);

    g.setFont(font.withHeight(tooltip_font_size));
    formatter.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipFontId));
    formatter.drawText(g, text, tooltip_padding * 2, tooltip_padding, width, height, Justification::topLeft);
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

void PluginStyle::drawComboBox(juce::Graphics &g, int width, int height, bool isMouseButtonDown,
                               int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox &box)
{
    const Colour colour_container_background = findColour(ComboBox::backgroundColourId);
    const Colour colour_button_background    = findColour(ComboBox::buttonColourId);

    g.setColour(colour_container_background);
    g.fillAll();

    const Rectangle<int> button(buttonX, buttonY, buttonW, buttonH);

    const int triangle_padding = 11;
    const int triangle_bottom  = buttonY + buttonH - triangle_padding;

    Path triangle_path;
    triangle_path.addTriangle(buttonX + buttonW / 2.0f, buttonY + triangle_padding,
                              buttonX + buttonW - triangle_padding, triangle_bottom,
                              buttonX + triangle_padding, triangle_bottom);
    triangle_path.applyTransform(AffineTransform::verticalFlip(buttonH));

    g.setColour(box.isPopupActive() ? findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                                    : colour_button_background);
    g.fillPath(triangle_path);
}

void PluginStyle::drawToggleButton(juce::Graphics &g, juce::ToggleButton &button, bool isMouseOver, bool)
{
    if(!button.isEnabled())
    {
        g.setOpacity(0.3f);
    }

    const int button_y = button.getHeight() / 2 - 8;

    g.drawImageAt(imgCheckbox, 0, button_y);

    if(button.getToggleState())
    {
        g.drawImageAt(imgCheckboxTick, 0, button_y);
    }
    else if(!button.getToggleState() && isMouseOver && button.isEnabled())
    {
        g.setOpacity(0.3f);
        g.drawImageAt(imgCheckboxTick, 0, button_y);
    }

    g.setColour(button.findColour(ToggleButton::textColourId));
    g.setFont(font);
    g.drawFittedText(button.getButtonText(), button.getLocalBounds().withX(23), Justification::centredLeft, 10);
}

void PluginStyle::drawBubble(juce::Graphics &g, juce::BubbleComponent &comp, const juce::Point<float> &tip,
                             const juce::Rectangle<float> &body)
{
    Path p;
    p.addBubble(body.reduced(0.5f), body.getUnion(Rectangle<float>(tip.x, tip.y, 1.0f, 1.0f)), {}, 0.0f, 0.0f);

    g.setColour(comp.findColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId));
    g.fillPath(p);

    g.setColour(comp.findColour(CossinAudioProcessorEditor::ColourTooltipBorderId));
    g.strokePath(p, PathStrokeType(1.0f));
}

//======================================================================================================================
void PluginStyle::drawPopupMenuBackground(juce::Graphics &g, int width, int height)
{
    g.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId));
    g.fillAll();

    g.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipBorderId));
    g.drawRect(0, 0, width, height);
}

void PluginStyle::drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area, bool isSeparator,
                                    bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                                    const juce::String &text, const juce::String &shortcutKeyText,
                                    const juce::Drawable *icon, const juce::Colour *textColour)
{
    const bool active_and_selected = isHighlighted && isActive;
    Colour colour_background       = Colours::transparentBlack;
    Colour colour_font             = findColour(CossinAudioProcessorEditor::ColourTooltipFontId);

    if(active_and_selected)
    {
        colour_background = findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId);
    }
    else if(!isActive)
    {
        colour_font = colour_font.darker(0.5f);
    }

    g.setColour(colour_background);
    g.fillRect(area.reduced(3, 1));

    g.setColour(colour_font);
    g.setFont(font);
    g.drawText(text, area.withLeft(6), Justification::centredLeft);
}

Font PluginStyle::getPopupMenuFont()
{
    return font;
}

//======================================================================================================================
void PluginStyle::drawAlertBox(Graphics &g, AlertWindow &window, const Rectangle<int> &textArea, TextLayout &layout)
{
    const int icon_width        = 80;
    const Rectangle<int> bounds = window.getLocalBounds();
    int icon_size               = jmin (icon_width + 50, bounds.getHeight() + 20);
    Rectangle<int> rect_icon(icon_size / -10, icon_size / -10, icon_size, icon_size);
    Path icon;
    juce_wchar character;
    uint32 colour;

    g.setColour(findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillAll();

    if (window.containsAnyExtraComponents() || window.getNumButtons() > 2)
    {
        icon_size = jmin(icon_size, textArea.getHeight() + 50);
    }

    if (window.getAlertType() == AlertWindow::WarningIcon)
    {
        character = '!';

        icon.addTriangle(rect_icon.getX() + rect_icon.getWidth() * 0.5f, (float)rect_icon.getY(),
                         static_cast<float>(rect_icon.getRight()), static_cast<float>(rect_icon.getBottom()),
                         static_cast<float>(rect_icon.getX()),     static_cast<float>(rect_icon.getBottom()));

        icon   = icon.createPathWithRoundedCorners (5.0f);
        colour = 0x66ff2a00;
    }
    else if (window.getAlertType() == AlertWindow::NoIcon)
    {
        colour    = Colour(0xff6666b9).withAlpha(0.4f).getARGB();
        character = 'x';
        icon.addEllipse(rect_icon.toFloat());
    }
    else
    {
        colour    = Colour(0xff00b0b9).withAlpha(0.4f).getARGB();
        character = window.getAlertType() == AlertWindow::InfoIcon ? 'i' : '?';
        icon.addEllipse(rect_icon.toFloat());
    }

    GlyphArrangement ga;
    ga.addFittedText({rect_icon.getHeight() * 0.9f, Font::bold}, String::charToString(character),
                      static_cast<float>(rect_icon.getX()),     static_cast<float>(rect_icon.getY()),
                      static_cast<float>(rect_icon.getWidth()), static_cast<float>(rect_icon.getHeight()),
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
void PluginStyle::reset(const jaut::ThemePointer &theme) noexcept
{
    const Image image_check_box  = theme->getImage(res::Png_Check_Box);
    const Image image_knob_small = theme->getImage(res::Png_Knob_Small);
    const Image image_knob_big   = theme->getImage(res::Png_Knob_Big);

    imgCheckbox        = image_check_box .getClippedImage({0, 0, 16, 16});
    imgCheckboxTick    = image_check_box .getClippedImage({0, 16, 16, 16});
    imgKnobBig         = image_knob_big  .getClippedImage({0, 0, 60, 60});
    imgKnobBigCursor   = image_knob_big  .getClippedImage({60, 0, 60, 60});
    imgKnobSmall       = image_knob_small.getClippedImage({0, 0, 36, 36});
    imgKnobSmallCursor = image_knob_small.getClippedImage({36, 0, 36, 36});
    imgSliderPeakMetre = theme->getImage(res::Png_Metre_H);

    const Colour colour_font                 = theme->getThemeColour(res::Col_Font);
    const Colour colour_component_background = theme->getThemeColour(res::Col_Component_Bg);
    const Colour colour_component_foreground = theme->getThemeColour(res::Col_Component_Fg);
    const Colour colour_container_background = theme->getThemeColour(res::Col_Container_Bg);
    const Colour colour_container_foreground = theme->getThemeColour(res::Col_Container_Fg);

    setColour(CossinAudioProcessorEditor::ColourFontId,                colour_font);
    setColour(CossinAudioProcessorEditor::ColourComponentBackgroundId, colour_component_background);
    setColour(CossinAudioProcessorEditor::ColourComponentForegroundId, colour_component_foreground);
    setColour(CossinAudioProcessorEditor::ColourContainerBackgroundId, colour_container_background);
    setColour(CossinAudioProcessorEditor::ColourContainerForegroundId, colour_container_foreground);
    setColour(CossinAudioProcessorEditor::ColourHeaderBackgroundId,    theme->getThemeColour(res::Col_Header_Bg));
    setColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId,   theme->getThemeColour(res::Col_Tooltip_Bg));
    setColour(CossinAudioProcessorEditor::ColourTooltipFontId,         theme->getThemeColour(res::Col_Tooltip_Font));
    setColour(CossinAudioProcessorEditor::ColourTooltipBorderId,       theme->getThemeColour(res::Col_Tooltip_Border));

    //override look and feel colours
    // juce::AlertWindow
    setColour(AlertWindow::textColourId, colour_font);

    // juce::ComboBox
    setColour(ComboBox::textColourId,       colour_font);
    setColour(ComboBox::arrowColourId,      colour_font);
    setColour(ComboBox::backgroundColourId, colour_container_background);
    setColour(ComboBox::buttonColourId,     colour_component_background);

    // juce::Label
    setColour(Label::textColourId,    colour_font);
    setColour(Label::outlineColourId, Colours::transparentBlack);

    // juce::ListBox
    setColour(ListBox::textColourId,       colour_font);
    setColour(ListBox::backgroundColourId, colour_container_background);
    setColour(ListBox::outlineColourId,    Colours::transparentBlack);

    // juce::TextEditor
    setColour(TextEditor::textColourId,           colour_font);
    setColour(TextEditor::backgroundColourId,     colour_container_background);
    setColour(TextEditor::focusedOutlineColourId, colour_component_foreground);
    setColour(TextEditor::outlineColourId,        Colours::transparentBlack);

    // juce::ToggleButton
    setColour(ToggleButton::textColourId,         colour_font);

    // FFAU::LevelMeter
    setColour(FFAU::LevelMeter::lmTextColour,             colour_font);
    setColour(FFAU::LevelMeter::lmTextDeactiveColour,     colour_container_foreground);
    setColour(FFAU::LevelMeter::lmMeterBackgroundColour,  colour_container_background);
    setColour(FFAU::LevelMeter::lmMeterGradientLowColour, colour_component_background);
    setColour(FFAU::LevelMeter::lmMeterGradientMidColour, colour_component_background);
    setColour(FFAU::LevelMeter::lmMeterGradientMaxColour, colour_component_foreground);
    setColour(FFAU::LevelMeter::lmBackgroundClipColour,   Colours::red);
    setColour(FFAU::LevelMeter::lmMeterForegroundColour,  Colours::green);
    setColour(FFAU::LevelMeter::lmMeterMaxWarnColour,     Colours::orange);
    setColour(FFAU::LevelMeter::lmTicksColour,            Colours::orange);
    setColour(FFAU::LevelMeter::lmOutlineColour,          Colours::orange);
    setColour(FFAU::LevelMeter::lmMeterReductionColour,   Colours::orange);
    setColour(FFAU::LevelMeter::lmMeterMaxOverColour,     Colours::darkred);
    setColour(FFAU::LevelMeter::lmMeterMaxNormalColour,   Colours::lightgrey);
    setColour(FFAU::LevelMeter::lmMeterOutlineColour,     Colours::transparentBlack);
    setColour(FFAU::LevelMeter::lmBackgroundColour,       Colours::transparentBlack);
    setColour(FFAU::LevelMeter::lmTextClipColour,         Colours::transparentBlack);

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

const jaut::ThemePointer PluginStyle::getTheme() const noexcept
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
