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
    @file   TopUnitRackGui.h
    @date   12, January 2020
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"
#include "ReloadListener.h"
#include <jaut/dspgui.h>

namespace jaut
{
class AudioProcessorRack;
class Localisation;
class ThemePointer;
}

class TopUnitRackGui final : public jaut::DspGui, public ReloadListener
{
public:
    TopUnitRackGui(jaut::AudioProcessorRack&, jaut::Localisation&);

    //==================================================================================================================
    void paintOverChildren(Graphics&) override;
    void resized() override;

    //==================================================================================================================
    void initialize();

    //==================================================================================================================
    void setGui(int, bool = false);

    //==================================================================================================================
    int getProcessorCount() const noexcept;
    
private:
    jaut::AudioProcessorRack &audioRack;
    jaut::Localisation &locale;
    Component contentComponent;
    TextButton buttonActivate;
    TextButton buttonSwitch;

    OwnedArray<jaut::DspGui> guis;
    int currentActiveIndex;
    int currentShownIndex;

    Font font;

    //==================================================================================================================
    void reloadTheme(const jaut::ThemePointer&) override;
    void reloadLocale(const jaut::Localisation&) override;
};
