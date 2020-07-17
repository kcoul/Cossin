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
    @file   EffectModules.h
    @date   24, December 2019
    
    ===============================================================
 */

#pragma once

#include <jaut_audio/jaut_audio.h>

class CossinAudioProcessor;
class EffectEqualiser : public jaut::SerialisableAudioProcessor
{
public:
    enum class FilterType
    {
        LowPass,
        HighPass,
        LowShelf,
        HighShelf,
        Notch,
        Bell
    };
    
    struct Band
    {
        FilterType type;
        float q;
        float gain;
        float frequency;
    };
    
    //==================================================================================================================
    explicit EffectEqualiser(juce::UndoManager&) noexcept;
    
    //==================================================================================================================
    juce::String getName() const noexcept override { return "Equalizer"; }
    
    //==================================================================================================================
    void prepare(jaut::ProcessSpec) override;
    void release() override;
    void process(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    
    //==================================================================================================================
    void readData(juce::ValueTree) override;
    void writeData(juce::ValueTree) override;
    
    //==================================================================================================================
    std::unique_ptr<juce::Component> createComponent() override;

private:
    juce::UndoManager &undoManager;
};
