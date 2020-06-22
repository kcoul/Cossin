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
    
    @author Elanda (elanda@elandasunshine.xyz)
    @file   EffectModuleGuis.cpp
    @date   25, December 2019
    
    ===============================================================
 */

#include "EffectModuleGuis.h"

#include "EffectModules.h"
#include "PluginEditor.h"

#pragma region EffectModule::Equalizer
#pragma region EffectEqualizerGui
/* ==================================================================================
 * =============================== EffectEqualizerGui ===============================
 * ================================================================================== */
EffectEqualizerGui::EffectEqualizerGui(EffectEqualizer &processor)
    : DspGui(processor)
{}

//======================================================================================================================
void EffectEqualizerGui::paint(Graphics &g)
{
    const LookAndFeel &lf = getLookAndFeel();
    
    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerForegroundId));
    g.fillAll();

    g.setColour(lf.findColour(CossinAudioProcessorEditor::ColourContainerBackgroundId));
    g.fillRect(getLocalBounds().reduced(6));
}

void EffectEqualizerGui::resized()
{

}
#pragma endregion EffectEqualizerGui
#pragma endregion EffectModule::Equalizer
