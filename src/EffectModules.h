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
    @file   EffectModules.h
    @date   24, December 2019
    
    ===============================================================
 */

#pragma once

#include <jaut_audio/jaut_audio.h>
#include <juce_audio_processors/juce_audio_processors.h>

class EffectModule : public jaut::SfxUnit
{
public:
    EffectModule(DspUnit &unit, AudioProcessorValueTreeState &vts, UndoManager *undoManager = nullptr)
        : SfxUnit(unit, vts, undoManager)
    {}

    //==================================================================================================================
    virtual Rectangle<int> getIconCoordinates() const = 0;
    virtual Colour getColour() const = 0;
};

class EffectEqualizer final : public EffectModule
{
public:
    EffectEqualizer(DspUnit&, AudioProcessorValueTreeState&, UndoManager*);

    //==================================================================================================================
    const String getName() const override { return "Equalizer"; }
    bool hasEditor() const override { return true; }

    //==================================================================================================================
    void processEffect(int index, AudioBuffer<float> &buffer,  MidiBuffer &midiBuffer) override;
    void processEffect(int index, AudioBuffer<double> &buffer, MidiBuffer &midiBuffer) override;
    void beginPlayback(int index, double sampleRate, int bufferSize) override;
    void finishPlayback(int index) override;

    //==================================================================================================================
    std::vector<SfxParameter> createParameters() const override;
    int getMaxInstances() const override { return 5; }
    DataContext *getNewContext() const override;

    //==================================================================================================================
    int getMaxBands() const noexcept { return 30; }
    Rectangle<int> getIconCoordinates() const override { return {128, 0, 32, 32}; }
    Colour getColour() const override { return Colour(255, 123, 59); }

private:
    jaut::DspGui *getGuiType() override;
};
