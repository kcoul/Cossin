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
    @file   GuiFrameProcessor.h
    @date   09, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"
#include <jaut/thememanager.h>

#define GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK 1

namespace jaut
{
    class DspUnitManager;
    class GuiDspUnitManager;
}

class CossinAudioProcessorEditor;

class GuiFrameProcessor final : public Component, ListBoxModel, 
#if (defined(GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK) && GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK)
Timer
#else
ScrollBar::Listener
#endif
{
public:
    GuiFrameProcessor(CossinAudioProcessorEditor&, jaut::DspUnitManager&) noexcept;
    ~GuiFrameProcessor();

    //==================================================================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==================================================================================================================
    int getNumRows() override;
    void paintListBoxItem(int, Graphics&, int, int, bool) override;
    void listBoxItemClicked(int, const MouseEvent&) override;

    //==================================================================================================================
    void reloadTheme(const jaut::ThemeManager::ThemePointer&);
    
private:
    class ScrollButton final : public Button, Button::Listener
    {
    public:
        ScrollButton(ScrollBar&, bool) noexcept;

        //==============================================================================================================
        void paintButton(Graphics&, bool, bool) override;

    private:
        ScrollBar &scrollBar;
        bool scrollsUpwards;
        Path path;
        Colour colFont;

        //==============================================================================================================
        void buttonClicked(Button*) override;
    };

    CossinAudioProcessorEditor &cossin;
    jaut::DspUnitManager &manager;
    ListBox listBox;
    std::unique_ptr<jaut::GuiDspUnitManager> guiManager;
    ScrollButton bttScrollUp;
    ScrollButton bttScrollDown;

    Font font;
    Image imgEffects;

    //==================================================================================================================
#if (defined(GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK) && GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK)
    void timerCallback() override;
#else
    void scrollBarMoved(ScrollBar*, double) override;
#endif

    JUCE_DECLARE_NON_COPYABLE(GuiFrameProcessor)
};
