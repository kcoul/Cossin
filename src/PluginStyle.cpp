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
      formatter({'&', juce::Colours::transparentBlack})
{
    formatter.setLookAndFeel(this);
    reset(theme);
}

PluginStyle::~PluginStyle() = default;

//======================================================================================================================
void PluginStyle::drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPosProportional,
                                   float rotaryStartAngle, float rotaryEndAngle, juce::Slider &slider)
{
    const juce::NamedValueSet &sliderprops = slider.getProperties();

    if (sliderprops.contains("CSSize") || sliderprops.contains("CSType"))
    {
        const bool is_big_knob          = sliderprops["CSSize"].toString().equalsIgnoreCase("big");
        const bool is_half_knob         = sliderprops["CSType"].toString().equalsIgnoreCase("half");
        const juce::Image &image_knob   = is_big_knob ? imgKnobBig       : imgKnobSmall;
        const juce::Image &image_cursor = is_big_knob ? imgKnobBigCursor : imgKnobSmallCursor;
        juce::Path p;

        if (is_half_knob)
        {
            const float startRad = 0.0f;
            const float endRad   = startRad + 2.38f * (sliderPosProportional * 2 - 1);
            p.addPieSegment(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), startRad, endRad, 0.85f);
        }
        else
        {
            p.addPieSegment(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), -2.38f,
                            -2.38f + (2.38f * 2 * sliderPosProportional), 0.85f);
        }

        g.drawImageAt(image_knob, 0, 0);
        g.drawImageTransformed(image_cursor, juce::AffineTransform::rotation(-2.35f + (4.7f * sliderPosProportional),
                               static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f));

        g.setColour(findColour(CossinAudioProcessorEditor::ColourComponentForegroundId));
        g.fillPath(p);

        return;
    }

    LookAndFeel_V4::drawRotarySlider(g, x, y, width, height, sliderPosProportional,
                                     rotaryStartAngle, rotaryEndAngle, slider);
}

void PluginStyle::drawCornerResizer(juce::Graphics &g, int width, int height, bool, bool mouseDragging)
{
    const juce::Rectangle<float> rect_resize(0.0f, 0.0f, static_cast<float>(width) * 1.5f,
                                                         static_cast<float>(height) * 1.0f);

    {
        juce::Graphics::ScopedSaveState state(g);

        g.addTransform(juce::AffineTransform::translation(0.0f, static_cast<float>(height) + 0.8f));
        g.addTransform(juce::AffineTransform::rotation(-0.785398f));
        g.setColour(findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
        g.fillRect(rect_resize);
    }

    g.addTransform(juce::AffineTransform::translation(0.0f, static_cast<float>(height) + 4.0f));
    g.addTransform(juce::AffineTransform::rotation(-0.785398f));
    g.setColour(mouseDragging ? findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                              : findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId));
    g.fillRect(rect_resize);
}

void PluginStyle::drawTooltip(juce::Graphics &g, const juce::String &text, int width, int height)
{
    constexpr float tooltip_font_size = 13.0f;
    constexpr int tooltip_padding     = 2;

    g.fillAll(findColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId));
    g.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipBorderId));
    g.drawRect(0, 0, width, height);

    g.setFont(font.withHeight(tooltip_font_size));
    formatter.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipFontId));
    formatter.drawText(g, text, tooltip_padding * 2, tooltip_padding, width, height, juce::Justification::topLeft);
}

void PluginStyle::drawButtonBackground(juce::Graphics &g, juce::Button&, const juce::Colour&, bool mouseOver, bool)
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

    const int indent_y    = juce::jmin(4, button.proportionOfHeight(0.3f));
    const int corner_size = juce::jmin(button.getHeight(), button.getWidth()) / 2;

    const int font_height  = juce::roundToInt(font.getHeight() * 0.6f);
    const int indent_left  = juce::jmin(font_height, 2 + corner_size / (button.isConnectedOnLeft()  ? 4 : 2));
    const int indent_right = juce::jmin(font_height, 2 + corner_size / (button.isConnectedOnRight() ? 4 : 2));
    const int text_width   = button.getWidth() - indent_left - indent_right;

    if (text_width > 0)
    {
        g.drawFittedText(button.getButtonText(), indent_left, indent_y, text_width, button.getHeight() - indent_y * 2,
                         juce::Justification::centred, 2);
    }
}

void PluginStyle::drawComboBox(juce::Graphics &g, int,  int,  bool, int buttonX, int buttonY, int buttonW, int buttonH,
                               juce::ComboBox &box)
{
    const juce::Colour colour_container_background = findColour(juce::ComboBox::backgroundColourId);
    const juce::Colour colour_button_background    = findColour(juce::ComboBox::buttonColourId);

    g.setColour(colour_container_background);
    g.fillAll();
    
    constexpr int triangle_padding = 11;
    const int triangle_bottom      = buttonY + buttonH - triangle_padding;
    
    juce::Path triangle_path;
    triangle_path.addTriangle(static_cast<float>(buttonX + (buttonW / 2.0f)),
                              static_cast<float>(buttonY + triangle_padding),
                              static_cast<float>(buttonX + buttonW - triangle_padding),
                              static_cast<float>(triangle_bottom),
                              static_cast<float>(buttonX + triangle_padding),
                              static_cast<float>(triangle_bottom));
    triangle_path.applyTransform(juce::AffineTransform::verticalFlip(static_cast<float>(buttonH)));

    g.setColour(box.isPopupActive() ? findColour(CossinAudioProcessorEditor::ColourComponentForegroundId)
                                    : colour_button_background);
    g.fillPath(triangle_path);
}

juce::Label* PluginStyle::createComboBoxTextBox(juce::ComboBox &box)
{
    auto label = new juce::Label();
    label->setFont(getComboBoxFont(box));
    label->setJustificationType(juce::Justification::left);
    label->setMinimumHorizontalScale(1.0f);
    
    return label;
}

juce::Font PluginStyle::getComboBoxFont(juce::ComboBox&)
{
    return font;
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

    g.setColour(button.findColour(juce::ToggleButton::textColourId));
    g.setFont(font);
    g.drawFittedText(button.getButtonText(), button.getLocalBounds().withX(23), juce::Justification::centredLeft, 10);
}

void PluginStyle::drawBubble(juce::Graphics &g, juce::BubbleComponent &comp, const juce::Point<float> &tip,
                             const juce::Rectangle<float> &body)
{
    juce::Path p;
    p.addBubble(body.reduced(0.5f), body.getUnion(juce::Rectangle<float>(tip.x, tip.y, 1.0f, 1.0f)), {}, 0.0f, 0.0f);

    g.setColour(comp.findColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId));
    g.fillPath(p);

    g.setColour(comp.findColour(CossinAudioProcessorEditor::ColourTooltipBorderId));
    g.strokePath(p, juce::PathStrokeType(1.0f));
}

//======================================================================================================================
void PluginStyle::drawPopupMenuBackground(juce::Graphics &g, int width, int height)
{
    g.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId));
    g.fillAll();

    g.setColour(findColour(CossinAudioProcessorEditor::ColourTooltipBorderId));
    g.drawRect(0, 0, width, height);
}

void PluginStyle::drawPopupMenuItem(juce::Graphics &g, const juce::Rectangle<int> &area, bool, bool isActive,
                                    bool isHighlighted, bool, bool, const juce::String &text, const juce::String&,
                                    const juce::Drawable*, const juce::Colour*)
{
    const bool active_and_selected = isHighlighted && isActive;
    juce::Colour colour_background = juce::Colours::transparentBlack;
    juce::Colour colour_font       = findColour(CossinAudioProcessorEditor::ColourTooltipFontId);

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
    g.drawText(text, area.withLeft(6), juce::Justification::centredLeft);
}

juce::Font PluginStyle::getPopupMenuFont()
{
    return font;
}

//======================================================================================================================
void PluginStyle::drawAlertBox(juce::Graphics &g, juce::AlertWindow &window, const juce::Rectangle<int> &textArea,
                               juce::TextLayout &layout)
{
    constexpr int icon_width          = 80;
    const juce::Rectangle<int> bounds = window.getLocalBounds();
    int icon_size                     = juce::jmin(icon_width + 50, bounds.getHeight() + 20);
    juce::Rectangle<int> rect_icon(icon_size / -10, icon_size / -10, icon_size, icon_size);
    juce::Path icon;
    juce::juce_wchar character;
    juce::uint32 colour;

    g.setColour(findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillAll();

    if (window.getAlertType() == juce::AlertWindow::WarningIcon)
    {
        character = '!';

        icon.addTriangle(rect_icon.getX() + rect_icon.getWidth() * 0.5f, (float)rect_icon.getY(),
                         static_cast<float>(rect_icon.getRight()), static_cast<float>(rect_icon.getBottom()),
                         static_cast<float>(rect_icon.getX()),     static_cast<float>(rect_icon.getBottom()));

        icon   = icon.createPathWithRoundedCorners (5.0f);
        colour = 0x66ff2a00;
    }
    else if (window.getAlertType() == juce::AlertWindow::NoIcon)
    {
        colour    = juce::Colour(0xff6666b9).withAlpha(0.4f).getARGB();
        character = 'x';
        icon.addEllipse(rect_icon.toFloat());
    }
    else
    {
        colour    = juce::Colour(0xff00b0b9).withAlpha(0.4f).getARGB();
        character = window.getAlertType() == juce::AlertWindow::InfoIcon ? 'i' : '?';
        icon.addEllipse(rect_icon.toFloat());
    }
    
    juce::GlyphArrangement ga;
    ga.addFittedText({rect_icon.getHeight() * 0.9f, juce::Font::bold}, juce::String::charToString(character),
                      static_cast<float>(rect_icon.getX()),     static_cast<float>(rect_icon.getY()),
                      static_cast<float>(rect_icon.getWidth()), static_cast<float>(rect_icon.getHeight()),
                     juce::Justification::centred, false);
    
    ga.createPath(icon);
    icon.setUsingNonZeroWinding(false);
    g.setColour(juce::Colour(colour));
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

juce::Font PluginStyle::getAlertWindowTitleFont()
{
    return font.withStyle(juce::Font::bold).withHeight(18.0f);
}

juce::Font PluginStyle::getAlertWindowMessageFont()
{
    return font.withHeight(16.0f);
}

juce::Font PluginStyle::getAlertWindowFont()
{
    return font;
}

//======================================================================================================================
void PluginStyle::drawScrollbar(juce::Graphics &g, juce::ScrollBar&, int x, int y, int width, int height,
                                bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool)
{
    juce::Rectangle<int> thumb_bounds;

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
void PluginStyle::reset(const jaut::ThemePointer &themePtr) noexcept
{
    theme = themePtr;
    
    const juce::Image image_check_box  = theme->getImage(res::Png_CheckBox);
    const juce::Image image_knob_small = theme->getImage(res::Png_KnobSmall);
    const juce::Image image_knob_big   = theme->getImage(res::Png_KnobBig);

    imgCheckbox        = image_check_box .getClippedImage({0,  0, 16, 16});
    imgCheckboxTick    = image_check_box .getClippedImage({0, 16, 16, 16});
    imgKnobBig         = image_knob_big  .getClippedImage({0,  0, 60, 60});
    imgKnobBigCursor   = image_knob_big  .getClippedImage({60, 0, 60, 60});
    imgKnobSmall       = image_knob_small.getClippedImage({0,  0, 36, 36});
    imgKnobSmallCursor = image_knob_small.getClippedImage({36, 0, 36, 36});
    imgSliderPeakMetre = theme->getImage(res::Png_MetreH);

    const juce::Colour colour_font                 = theme->getThemeColour(res::Col_Font);
    const juce::Colour colour_component_background = theme->getThemeColour(res::Col_ComponentBg);
    const juce::Colour colour_component_foreground = theme->getThemeColour(res::Col_ComponentFg);
    const juce::Colour colour_container_background = theme->getThemeColour(res::Col_ContainerBg);
    const juce::Colour colour_container_foreground = theme->getThemeColour(res::Col_ContainerFg);

    setColour(CossinAudioProcessorEditor::ColourFontId,                colour_font);
    setColour(CossinAudioProcessorEditor::ColourComponentBackgroundId, colour_component_background);
    setColour(CossinAudioProcessorEditor::ColourComponentForegroundId, colour_component_foreground);
    setColour(CossinAudioProcessorEditor::ColourContainerBackgroundId, colour_container_background);
    setColour(CossinAudioProcessorEditor::ColourContainerForegroundId, colour_container_foreground);
    setColour(CossinAudioProcessorEditor::ColourHeaderBackgroundId,    theme->getThemeColour(res::Col_HeaderBg));
    setColour(CossinAudioProcessorEditor::ColourTooltipBackgroundId,   theme->getThemeColour(res::Col_TooltipBg));
    setColour(CossinAudioProcessorEditor::ColourTooltipFontId,         theme->getThemeColour(res::Col_TooltipFont));
    setColour(CossinAudioProcessorEditor::ColourTooltipBorderId,       theme->getThemeColour(res::Col_TooltipBorder));

    //override look and feel colours
    // juce::AlertWindow
    setColour(juce::AlertWindow::textColourId, colour_font);

    // juce::ComboBox
    setColour(juce::ComboBox::textColourId,       colour_font);
    setColour(juce::ComboBox::arrowColourId,      colour_font);
    setColour(juce::ComboBox::backgroundColourId, colour_container_background);
    setColour(juce::ComboBox::buttonColourId,     colour_component_background);

    // juce::Label
    setColour(juce::Label::textColourId,    colour_font);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

    // juce::ListBox
    setColour(juce::ListBox::textColourId,       colour_font);
    setColour(juce::ListBox::backgroundColourId, colour_container_background);
    setColour(juce::ListBox::outlineColourId,    juce::Colours::transparentBlack);

    // juce::TextEditor
    setColour(juce::TextEditor::textColourId,           colour_font);
    setColour(juce::TextEditor::backgroundColourId,     colour_container_background);
    setColour(juce::TextEditor::focusedOutlineColourId, colour_component_foreground);
    setColour(juce::TextEditor::outlineColourId,        juce::Colours::transparentBlack);

    // juce::ToggleButton
    setColour(juce::ToggleButton::textColourId, colour_font);

    // FFAU::LevelMeter
    setColour(FFAU::LevelMeter::lmTextColour,             colour_font);
    setColour(FFAU::LevelMeter::lmTextDeactiveColour,     colour_container_foreground);
    setColour(FFAU::LevelMeter::lmMeterBackgroundColour,  colour_container_background);
    setColour(FFAU::LevelMeter::lmMeterGradientLowColour, colour_component_background);
    setColour(FFAU::LevelMeter::lmMeterGradientMidColour, colour_component_background);
    setColour(FFAU::LevelMeter::lmMeterGradientMaxColour, colour_component_foreground);
    setColour(FFAU::LevelMeter::lmBackgroundClipColour,   juce::Colours::red);
    setColour(FFAU::LevelMeter::lmMeterForegroundColour,  juce::Colours::green);
    setColour(FFAU::LevelMeter::lmMeterMaxWarnColour,     juce::Colours::orange);
    setColour(FFAU::LevelMeter::lmTicksColour,            juce::Colours::orange);
    setColour(FFAU::LevelMeter::lmOutlineColour,          juce::Colours::orange);
    setColour(FFAU::LevelMeter::lmMeterReductionColour,   juce::Colours::orange);
    setColour(FFAU::LevelMeter::lmMeterMaxOverColour,     juce::Colours::darkred);
    setColour(FFAU::LevelMeter::lmMeterMaxNormalColour,   juce::Colours::lightgrey);
    setColour(FFAU::LevelMeter::lmMeterOutlineColour,     juce::Colours::transparentBlack);
    setColour(FFAU::LevelMeter::lmBackgroundColour,       juce::Colours::transparentBlack);
    setColour(FFAU::LevelMeter::lmTextClipColour,         juce::Colours::transparentBlack);

    // jaut::CharFormat
    setColour(jaut::CharFormat::ColourFormat0Id, theme->getThemeColour(res::Col_Format0));
    setColour(jaut::CharFormat::ColourFormat1Id, theme->getThemeColour(res::Col_Format1));
    setColour(jaut::CharFormat::ColourFormat2Id, theme->getThemeColour(res::Col_Format2));
    setColour(jaut::CharFormat::ColourFormat3Id, theme->getThemeColour(res::Col_Format3));
    setColour(jaut::CharFormat::ColourFormat4Id, theme->getThemeColour(res::Col_Format4));
    setColour(jaut::CharFormat::ColourFormat5Id, theme->getThemeColour(res::Col_Format5));
    setColour(jaut::CharFormat::ColourFormat6Id, theme->getThemeColour(res::Col_Format6));
    setColour(jaut::CharFormat::ColourFormat7Id, theme->getThemeColour(res::Col_Format7));
    setColour(jaut::CharFormat::ColourFormat8Id, theme->getThemeColour(res::Col_Format8));
    setColour(jaut::CharFormat::ColourFormat9Id, theme->getThemeColour(res::Col_Format9));
    setColour(jaut::CharFormat::ColourFormatAId, theme->getThemeColour(res::Col_FormatA));
    setColour(jaut::CharFormat::ColourFormatBId, theme->getThemeColour(res::Col_FormatB));
    setColour(jaut::CharFormat::ColourFormatCId, theme->getThemeColour(res::Col_FormatC));
    setColour(jaut::CharFormat::ColourFormatDId, theme->getThemeColour(res::Col_FormatD));
    setColour(jaut::CharFormat::ColourFormatEId, theme->getThemeColour(res::Col_FormatE));
    setColour(jaut::CharFormat::ColourFormatFId, theme->getThemeColour(res::Col_FormatF));

    font = theme->getThemeFont();
    MetreLookAndFeel::reloadResources();
}

jaut::ThemePointer& PluginStyle::getTheme() noexcept
{
    return theme;
}

//======================================================================================================================
const juce::Font& PluginStyle::getFont() const noexcept
{
    return font;
}

juce::Font PluginStyle::getFont(float newHeight, int newStyleFlags, float newHorizontalScale,
                                float newKerningAmount) const
{
    return font.withHeight(newHeight).withStyle(newStyleFlags).withHorizontalScale(newHorizontalScale)
               .withExtraKerningFactor(newKerningAmount);
}
