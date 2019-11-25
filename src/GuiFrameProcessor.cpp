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
    @file   GuiFrameProcessor.cpp
    @date   09, October 2019
    
    ===============================================================
 */

#include "GuiFrameProcessor.h"
#include "PluginEditor.h"
#include "Resources.h"

#include <jaut/dspunitmanager.h>
#include <jaut/guidspunitmanager.h>
#include <jaut/ithemedefinition.h>
#include <jaut/thememanager.h>

inline constexpr bool Const_Debug = false;

/* ==================================================================================
 * ================================== ScrollButton ==================================
 * ================================================================================== */

GuiFrameProcessor::ScrollButton::ScrollButton(ScrollBar &scrollBar, bool scrollsUp) noexcept
    : Button("scrollbutton"),
      scrollBar(scrollBar), scrollsUpwards(scrollsUp)
{
    path.addTriangle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (MathConstants<float>::twoPi * (!scrollsUp ? 0.25f : 0.75f),
                                                    0.5f, 0.5f));
    path.applyTransform(AffineTransform::scale(10.0f));
    addListener(this);
}

//======================================================================================================================
void GuiFrameProcessor::ScrollButton::paintButton(Graphics &g, bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    const LookAndFeel &lf = getLookAndFeel();

    Path p(this->path);

    g.fillAll(Colours::black.withAlpha(0.3f));
    Rectangle<float> trianglebounds = path.getBounds().toFloat();
    
    p.applyTransform(AffineTransform::translation(getWidth() / 2 - trianglebounds.getWidth() / 2,
                                                  getHeight() / 2 - trianglebounds.getHeight() / 2));
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
    g.fillPath(p);
}

//======================================================================================================================
void GuiFrameProcessor::ScrollButton::buttonClicked(Button *button)
{
    scrollBar.moveScrollbarInSteps(scrollsUpwards ? -1 : 1);
}



/* ==================================================================================
 * =============================== GuiFrameProcessor ================================
 * ================================================================================== */

GuiFrameProcessor::GuiFrameProcessor(CossinAudioProcessorEditor &cossin, jaut::DspUnitManager &manager) noexcept
    : cossin(cossin), manager(manager), listBox("", this),
      guiManager(dynamic_cast<jaut::GuiDspUnitManager*>(manager.createGui())),
      bttScrollUp(listBox.getVerticalScrollBar(), true), bttScrollDown(listBox.getVerticalScrollBar(), false)
{
    listBox.setRowHeight(60);
    listBox.selectRow(guiManager->getActiveIndex());
    listBox.setColour(ListBox::backgroundColourId, Colours::transparentBlack);
    listBox.getViewport()->setScrollBarsShown(false, false, true, false);
    addAndMakeVisible(listBox);
    addAndMakeVisible(guiManager.get());
    addChildComponent(bttScrollUp);
    addChildComponent(bttScrollDown);

#if (defined(GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK) && GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK)
    startTimer(100);
#else
    listBox.getVerticalScrollBar().addListener(this);
#endif
}

GuiFrameProcessor::~GuiFrameProcessor() {}

//======================================================================================================================
void GuiFrameProcessor::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillRect(0, 0, 60, 6);
    g.fillRect(0, getHeight() - 6, 60, 6);

    if constexpr(Const_Debug)
    {
        g.fillRect(60, 0, getWidth() - 60, getHeight());
        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
        g.setFont(font.withHeight(13));
        g.drawText("No effects available yet!", 60, 0, getWidth() - 60, getHeight(), Justification::centred);
    }
}

void GuiFrameProcessor::resized()
{
    const ScrollBar &versb = listBox.getVerticalScrollBar();
    const int boxwid       = 60;
    const int boxdif       = getWidth() - 60;
    const bool canscroll   = listBox.getViewport()->canScrollVertically();

    listBox.setBounds(0, 6, boxwid, getHeight() - 12);
    guiManager->setBounds(boxwid, 0, boxdif, getHeight());
    bttScrollUp.setBounds(0, 6, boxwid, 20);
    bttScrollDown.setBounds(0, getHeight() - 26, boxwid, 20);

    bttScrollUp.setVisible(canscroll && versb.getCurrentRangeStart() > versb.getMinimumRangeLimit());
    bttScrollDown.setVisible(canscroll && versb.getCurrentRange().getEnd() < versb.getMaximumRangeLimit());
}

//======================================================================================================================
int GuiFrameProcessor::getNumRows()
{
    if constexpr(Const_Debug)
    {
        return 10;
    }

    return manager.getNumProcessors();
}

void GuiFrameProcessor::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    const LookAndFeel &lf = getLookAndFeel();

    const int x            = JT_FIX(rowNumber % 5 * 32);
    const int y            = JT_FIX(rowNumber / 5 * 32);
    const Image effecticon = imgEffects.getClippedImage({x, y, 32, 32});
    const jaut::DspUnit *unit;
    Array<String> names;

    if constexpr(Const_Debug)
    {
        names = Array<String>("Chorus", "Compressor", "Delay", "Distortion", "Equalizer",
                              "Filter", "Flanger", "Phaser", "Reverb", "Soundfield Imager");
    }
    else
    {
        unit = manager.getProcessor(rowNumber);
    }

    g.setColour(rowIsSelected ? lf.findColour(CossinAudioProcessorEditor::ColourComponentBackgroundId)
                              : lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.fillRect(0, 0, width, height);
    g.drawImage(effecticon, Rectangle(width / 2 - 15, height / 2 - 29, 32, 32).toFloat());
    g.setFont(font.withHeight(11.0f));
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));

    if constexpr(Const_Debug)
    {
        g.drawFittedText(names[rowNumber], 0, height / 2, width, height / 2, Justification::centred, 2, 1.0f);
    }
    else
    {
        g.drawFittedText(unit->getName(), 0, height / 2, width, height / 2, Justification::centred, 2, 1.0f);
    }
}

void GuiFrameProcessor::listBoxItemClicked (int row, const MouseEvent&)
{
    guiManager->setUnit(row);
}

//======================================================================================================================
void GuiFrameProcessor::reloadTheme(const jaut::ThemePointer &theme)
{
    font       = theme->getThemeFont();
    imgEffects = theme->getImage(res::Png_Fx_Icon_x32);
}

//======================================================================================================================
#if (defined(GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK) && GUI_FRAME_PROCESSOR_USE_TIMER_INSTEAD_OF_CALLBACK)
void GuiFrameProcessor::timerCallback()
{
    ScrollBar &versb = listBox.getVerticalScrollBar();

    if(listBox.getViewport()->canScrollVertically())
    {
        bttScrollUp.setVisible(versb.getCurrentRangeStart() > versb.getMinimumRangeLimit());
        bttScrollDown.setVisible(versb.getCurrentRange().getEnd() < versb.getMaximumRangeLimit());
    }
}
#else
void GuiFrameProcessor::scrollBarMoved(ScrollBar *sb, double newRangeStart)
{
    if(sb->isVertical() && listBox.getViewport()->canScrollVertically())
    {
        bttScrollUp.setVisible(newRangeStart > sb->getMinimumRangeLimit());
        bttScrollDown.setVisible(sb->getCurrentRange().getEnd() < sb->getMaximumRangeLimit());
    }
}
#endif
