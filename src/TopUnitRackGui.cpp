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
    @file   TopUnitRackGui.cpp
    @date   12, January 2020
    
    ===============================================================
 */
#include "TopUnitRackGui.h"

#include "PluginEditor.h"
#include <jaut/audioprocessorrack.h>
#include <jaut/localisation.h>

//======================================================================================================================
TopUnitRackGui::TopUnitRackGui(jaut::AudioProcessorRack &rack, jaut::Localisation &locale)
    : DspGui(rack), audioRack(rack), locale(locale),
      currentActiveIndex(rack.getConnectionIndex()), currentShownIndex(-1)
{
    initialize();

    addAndMakeVisible(contentComponent);

    buttonActivate.onClick = [this]()
    {
        setGui(currentShownIndex, true);
    };
    addChildComponent(buttonActivate);

    buttonSwitch.onClick = [this]()
    {
        setGui(currentActiveIndex);
    };
    addChildComponent(buttonSwitch);
}

//======================================================================================================================
void TopUnitRackGui::paintOverChildren(Graphics &g)
{
    if(currentShownIndex != currentActiveIndex)
    {
        const LookAndFeel &lf = getLookAndFeel();

        g.setColour(Colours::black);
        g.setOpacity(0.5f);
        g.fillAll();

        g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourFontId));
        g.setFont(font);
        g.drawText(locale.translate("frame_rack.not_active"), getLocalBounds().toFloat(), Justification::centred);
    }
}

void TopUnitRackGui::resized()
{
    contentComponent.setBounds(getLocalBounds());

    for(auto *comp : contentComponent.getChildren())
    {
        comp->setBounds(contentComponent.getLocalBounds());
    }

    const int frame_centre_x = getWidth()  / 2;
    const int frame_centre_y = getHeight() / 2;

    buttonActivate.setBounds(frame_centre_x - 102, frame_centre_y + 20, 100, 30);
    buttonSwitch  .setBounds(frame_centre_x + 3,   frame_centre_y + 20, 100, 30);
}

//======================================================================================================================
void TopUnitRackGui::initialize()
{
    for(int i = 0; i < audioRack.getNumDevices(); ++i)
    {
        contentComponent.addChildComponent(guis.add((*audioRack.getDevice(i))->createGui()));
    }

    setGui(currentActiveIndex, true);
}

//======================================================================================================================
void TopUnitRackGui::setGui(int index, bool activate)
{
    bool needs_repaint = false;
    Component *const to_display = contentComponent.getChildComponent(index);

    if(activate && index != currentActiveIndex)
    {
        {
            const CriticalSection::ScopedLockType lock (audioRack.getCallbackLock());
            audioRack.createSoloChain(index);
        }

        currentActiveIndex = index;
        needs_repaint      = true;
    }

    if(index != currentShownIndex)
    {
        const bool index_active = index == currentActiveIndex;

        for(auto *cp : contentComponent.getChildren())
        {
            cp->setEnabled(cp == to_display && index_active);
            cp->setVisible(cp == to_display);
        }

        buttonActivate.setVisible(!index_active);
        buttonSwitch  .setVisible(!index_active);

        currentShownIndex = index;
        needs_repaint     = false;
    }

    if(needs_repaint)
    {
        repaint();
    }
}

//======================================================================================================================
int TopUnitRackGui::getProcessorCount() const noexcept
{
    return contentComponent.getNumChildComponents();
}

//======================================================================================================================
void TopUnitRackGui::reloadTheme(const jaut::ThemePointer &theme)
{
    font = theme->getThemeFont();
}

void TopUnitRackGui::reloadLocale(const jaut::Localisation &locale)
{
    buttonActivate.setButtonText(locale.translate("frame_rack.activate"));
    buttonSwitch  .setButtonText(locale.translate("frame_rack.switch"));
}
