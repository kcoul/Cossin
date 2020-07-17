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
    @file   EffectModules.cpp
    @date   24, December 2019
    
    ===============================================================
 */

#include "EffectModules.h"

#include "PluginProcessor.h"
#include "EffectModuleGuis.h"

#include <juce_dsp/juce_dsp.h>

EffectEqualiser::EffectEqualiser(juce::UndoManager &undoManager) noexcept
    : undoManager(undoManager)
{}

//======================================================================================================================
void EffectEqualiser::prepare(jaut::ProcessSpec spec)
{

}

void EffectEqualiser::release()
{

}

void EffectEqualiser::process(juce::AudioBuffer<float>&, juce::MidiBuffer&)
{

}

//======================================================================================================================
void EffectEqualiser::readData(juce::ValueTree state)
{

}

void EffectEqualiser::writeData(juce::ValueTree state)
{

}

//======================================================================================================================
std::unique_ptr<juce::Component> EffectEqualiser::createComponent()
{
    return std::make_unique<EffectEqualizerGui>();
}
