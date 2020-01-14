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
    @file   PluginStyle.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"
#include "MetreLookAndFeel.h"
#include "ThemeFolder.h"

#include <jaut/componentoptionlist.h>
#include <jaut/fontformat.h>
#include <jaut/lookandfeel.h>
#include <jaut/thememanager.h>

class PluginStyle final : public jaut::LookAndFeel, public MetreLookAndFeel
{
public:
    PluginStyle() noexcept;
    ~PluginStyle() override;

protected:
    void drawRotarySlider(Graphics&, int, int, int, int, float, float, float, Slider&) override;
    void drawCornerResizer(Graphics&, int, int, bool, bool) override;
    void drawTooltip(Graphics&, const String&, int, int) override;
    void drawButtonBackground(Graphics&, Button&, const Colour&, bool, bool) override;
    void drawButtonText(Graphics&, TextButton&, bool, bool) override;
    void drawComboBox(Graphics&, int, int, bool, int, int, int, int, ComboBox&) override;
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawBubble(juce::Graphics&, juce::BubbleComponent&, const juce::Point<float>&,
                    const juce::Rectangle<float>&) override;

    //==================================================================================================================
    void drawPopupMenuBackground(Graphics&, int, int) override;
    void drawPopupMenuItem(Graphics&, const Rectangle<int>&, bool, bool, bool, bool, bool, const String&,
                           const String&, const Drawable*, const Colour*) override;
    Font getPopupMenuFont() override;

    //==================================================================================================================
    void drawAlertBox(Graphics&, AlertWindow&, const Rectangle<int>&, TextLayout&) override;
    int  getAlertWindowButtonHeight() override;
    Font getAlertWindowTitleFont() override;
    Font getAlertWindowMessageFont() override;
    Font getAlertWindowFont() override;

    //==================================================================================================================
    int getOptionListLabelWidth(const String&) override;
    void drawOptionListOptionBox(Graphics&, Rectangle<int>, bool, bool, bool, bool, bool) override;
    void drawOptionListOptionLabel(Graphics&, const String&, Rectangle<int>, bool, bool, bool, bool,
                                   bool, bool) override;

    //==================================================================================================================
    void drawScrollbar(Graphics&, ScrollBar&, int, int, int, int, bool, int, int, bool, bool) override;

public:
    //==================================================================================================================
    void reset(const jaut::ThemePointer&) noexcept;
    const jaut::ThemePointer getTheme() const noexcept;

    //==================================================================================================================
    const Font &getFont() const noexcept;
    Font getFont(float, int = 0, float = 1.0f, float = 0.0f) const noexcept;

private:
    jaut::ThemePointer theme;
    jaut::CharFormat formatter;

    Image imgCheckbox;
    Image imgCheckboxTick;
    Image imgKnobBig;
    Image imgKnobBigCursor;
    Image imgKnobSmall;
    Image imgKnobSmallCursor;
    Image imgSliderPeakMetre;
    Font font;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginStyle)
};
