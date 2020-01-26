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
    @file   PluginProcessor.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"

#include <jaut/dspunitmanager.h>
#include <jaut/propertymap.h>
#include <jaut/config.h>
#include <jaut/audioprocessorrack.h>

inline constexpr int Const_NumChannels = 2;

class SharedData;

class CossinAudioProcessor : public AudioProcessor
{
public:
    CossinAudioProcessor();
    ~CossinAudioProcessor();

    //==================================================================================================================
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

    //==================================================================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==================================================================================================================
    const String getName() const override;
#pragma region Unused
    bool   acceptsMidi() const override  { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }

    //==================================================================================================================
    int  getNumPrograms() override    { return 0; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    void changeProgramName(int, const String&) override {}
    const String getProgramName(int) override { return String(); }
#pragma endregion Unused

    //==================================================================================================================
    void getStateInformation (MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    //==================================================================================================================
    // GUI FUNCTIONS
    Rectangle<int> &getWindowSize() noexcept;

private:
    SharedResourcePointer<SharedData> sharedData;
    std::unique_ptr<jaut::ScopedATCD> atcd;

    AudioProcessorValueTreeState parameters;
    jaut::PropertyMap            properties;
    UndoManager                  undoManager;

    RangedAudioParameter *parGain;
    RangedAudioParameter *parPanning;

    FFAU::LevelMeterSource   metreSource;
    jaut::AudioProcessorRack topUnitRack;

    float previousGain[Const_NumChannels];

    //==================================================================================================================
    // GUI DATA (only data which is solely considered while loading and saving)
    Rectangle<int> windowBounds;

    //==================================================================================================================
    void initialize();
    void makeParameters();
    float calculatePanningGain(int) const noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossinAudioProcessor)

};
