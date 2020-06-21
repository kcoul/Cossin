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
    @file   PluginStyle.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include <ff_meters/ff_meters.h>
#include <jaut_provider/jaut_provider.h>
#include <jaut_gui/jaut_gui.h>

#include "MetreLookAndFeel.h"

class PluginStyle final : public juce::LookAndFeel_V4, public MetreLookAndFeel
{
public:
    PluginStyle() noexcept;
    ~PluginStyle() override;

protected:
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawCornerResizer(juce::Graphics&, int, int, bool, bool) override;
    void drawTooltip(juce::Graphics&, const juce::String&, int, int) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Label* createComboBoxTextBox(juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawBubble(juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>&,
                    const juce::Rectangle<float>&) override;
    
    //==================================================================================================================
    void drawPopupMenuBackground(juce::Graphics&, int, int) override;
    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>&, bool, bool, bool, bool, bool,
                           const juce::String&, const juce::String&, const juce::Drawable*,
                           const juce::Colour*) override;
    juce::Font getPopupMenuFont() override;

    //==================================================================================================================
    void drawAlertBox(juce::Graphics&, juce::AlertWindow&, const juce::Rectangle<int>&, juce::TextLayout&) override;
    int  getAlertWindowButtonHeight() override;
    juce::Font getAlertWindowTitleFont() override;
    juce::Font getAlertWindowMessageFont() override;
    juce::Font getAlertWindowFont() override;

    //==================================================================================================================
    void drawScrollbar(juce::Graphics&, juce::ScrollBar&, int, int, int, int, bool, int, int, bool, bool) override;

public:
    //==================================================================================================================
    void reset(const jaut::ThemePointer&) noexcept;
    jaut::ThemePointer &getTheme() noexcept;

    //==================================================================================================================
    const juce::Font &getFont() const noexcept;
    juce::Font getFont(float, int = 0, float = 1.0f, float = 0.0f) const;

private:
    jaut::ThemePointer theme;
    jaut::CharFormat formatter;
    
    juce::Image imgCheckbox;
    juce::Image imgCheckboxTick;
    juce::Image imgKnobBig;
    juce::Image imgKnobBigCursor;
    juce::Image imgKnobSmall;
    juce::Image imgKnobSmallCursor;
    juce::Image imgSliderPeakMetre;
    juce::Font font;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginStyle)
};
