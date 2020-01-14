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
    @file   EffectModules.cpp
    @date   24, December 2019
    
    ===============================================================
 */

#include "EffectModules.h"
#include "EffectModuleGuis.h"

#pragma region EffectModuleEqualizer
#pragma region EffectEqualizerContext
/* ==================================================================================
 * ============================= EffectEqualizerContext =============================
 * ================================================================================== */
struct EffectEqualizerContext : public EffectEqualizer::DataContext {};
#pragma endregion EffectEqualizerContext
#pragma region EffectEqualizer
/* ==================================================================================
 * ================================= EffectEqualizer ================================
 * ================================================================================== */
EffectEqualizer::EffectEqualizer(DspUnit &processor, AudioProcessorValueTreeState &vts, UndoManager *undoManager)
    : EffectModule(processor, vts, undoManager)
{
    initialize();
}

//======================================================================================================================
void EffectEqualizer::processEffect(int index, AudioBuffer<float> &buffer,  MidiBuffer &midiBuffer)
{

}

void EffectEqualizer::processEffect(int index, AudioBuffer<double> &buffer, MidiBuffer &midiBuffer)
{

}

void EffectEqualizer::beginPlayback(int index, double sampleRate, int bufferSize)
{

}

void EffectEqualizer::finishPlayback(int index)
{

}

/*
const String &id, const String& name, const String& label,
                              NormalisableRange<float> range, float defaultValue,
                              std::function<String(float)> toTextFunction,
                              std::function<float(const String&)> fromTextFunction, bool isMetaParameter = false,
                              bool isAutomatableParameter = true, bool isDiscrete = false,
                              AudioProcessorParameter::Category category = AudioProcessorParameter::genericParameter,
                              bool isBoolean = false
*/

//======================================================================================================================
std::vector<EffectEqualizer::SfxParameter> EffectEqualizer::createParameters() const
{
    std::vector<SfxParameter> parameters;

    auto frequency_band_value_to_text = [](float value) -> String
    {
        return String(static_cast<int>(value)) + " Hz";
    };

    auto gain_band_value_to_text = [](float value) -> String
    {
        return String(Decibels::gainToDecibels(value, -30.0f)) + "dBFS";
    };

    for(int i = 0; i < getMaxBands(); ++i)
    {
        parameters.emplace_back(SfxParameter("band_" + String(i) + "_freq", "Band " + String(i + 1) + " Frequency", "",
                                {10.0f, 20000.0f}, 10.0f, frequency_band_value_to_text, nullptr));
        parameters.emplace_back(SfxParameter("band_" + String(i) + "_gain", "Band " + String(i + 1) + " Gain", "",
                                {0.0f, 31.6227766f}, 1.0f, gain_band_value_to_text, nullptr));
        parameters.emplace_back(SfxParameter("band_" + String(i) + "_q", "Band " + String(i + 1) + " Q", "",
                                {0.1f, 50.0f}, 1.0f, nullptr, nullptr));
    }

    return parameters;
}

//======================================================================================================================
EffectEqualizer::DataContext *EffectEqualizer::getNewContext() const
{
    return new EffectEqualizerContext();
}

jaut::DspGui *EffectEqualizer::getGuiType()
{
    return new EffectEqualizerGui(*this);
}
#pragma endregion EffectEqualizer
#pragma endregion EffectModuleEqualizer
