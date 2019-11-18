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

#include "ProcessorContainer.h"
#include <jaut/dspunitmanager.h>
#include <jaut/propertymap.h>

constexpr float F_Skew_Factor               = 0.45f;
constexpr float F_Pi                        = 3.14159f;
constexpr float F_LinearPanningCompensation = 2.0f;
constexpr float F_SquarePanningCompensation = 1.41421356238f;
constexpr float F_SinePanningCompensation   = 1.41421356238f;

class SharedData;

class CossinAudioProcessor : public AudioProcessor, jaut::PropertyMap::Listener
{
public:
    CossinAudioProcessor();
    ~CossinAudioProcessor();

    //==================================================================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout &layouts) const override;
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==================================================================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==================================================================================================================
    const String getName() const override;
#if(1) //unused
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==================================================================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String &newName) override;
#endif //unused

    //==================================================================================================================
    void getStateInformation (MemoryBlock &destData) override;
    void setStateInformation (const void *data, int sizeInBytes) override;

    //==================================================================================================================
    // GUI FUNCTIONS
    Rectangle<int> &getWindowSize() noexcept;

private:
    std::unique_ptr<jaut::ScopedATCD> atcd;
    RangedAudioParameter *parGain;
    RangedAudioParameter *parPanning;
    FFAU::LevelMeterSource metreSource;
    jaut::PropertyMap properties;
    AudioProcessorValueTreeState parameters;
    UndoManager undoer;
    float previousGain[2];
    ProcessorContainer processorContainer;

    //==================================================================================================================
    // GUI DATA (only data which is solely considered while loading and saving)
    Rectangle<int> windowBounds;

    //==================================================================================================================
    void initialization();
    void makeParameters();
    float calculatePanningGain(int channel) const noexcept;
    void onValueChanged(const String &name, var oldValue, var newValue) override;
    void onPropertyAdded(const String &name, var value) override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossinAudioProcessor)

};
