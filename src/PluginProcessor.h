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
    
    @author Elanda
    @file   PluginProcessor.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <ff_meters/ff_meters.h>

inline constexpr int Const_NumChannels = 2;

struct ParameterIds
{
    static constexpr const char *MasterLevel = "par_master_level";
    static constexpr const char *MasterMix   = "par_master_mix";
    static constexpr const char *MasterPan   = "par_master_pan";
    
    static constexpr const char *PropertyPanningMode = "property_panning_law";
    static constexpr const char *PropertyProcessMode = "property_process_mode";
};

class SharedData;

class CossinAudioProcessor : public juce::AudioProcessor
{
public:
    CossinAudioProcessor();
    ~CossinAudioProcessor();

    //==================================================================================================================
    void prepareToPlay(double, int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==================================================================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==================================================================================================================
    const juce::String getName() const override;
    
    // region Unused
    bool   acceptsMidi() const override  { return false; }
    bool   producesMidi() const override { return false; }
    bool   isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }

    //==================================================================================================================
    int  getNumPrograms() override    { return 0; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    void changeProgramName(int, const juce::String&) override {}
    const juce::String getProgramName(int) override { return juce::String(); }
    // endregion Unused

    //==================================================================================================================
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    //==================================================================================================================
    // GUI FUNCTIONS
    juce::Rectangle<int> &getWindowSize() noexcept;

private:
    static BusesProperties getDefaultBusesLayout()
    {
        return BusesProperties()
                   .withInput ("Input",     juce::AudioChannelSet::stereo())
                   .withOutput("Output",    juce::AudioChannelSet::stereo())
                   .withInput ("Sidechain", juce::AudioChannelSet::mono());
    }
    
    //==================================================================================================================
    juce::UndoManager undoManager;
    foleys::LevelMeterSource metreSource;
    juce::AudioProcessorValueTreeState parameters;
    juce::SharedResourcePointer<SharedData> sharedData;
    
    juce::AudioParameterFloat *parGain     { nullptr };
    juce::AudioParameterFloat *parPanning  { nullptr };
    juce::AudioParameterFloat *parMix      { nullptr };
    juce::AudioParameterInt   *parPanMode  { nullptr };
    juce::AudioParameterInt   *parProcMode { nullptr };
    
    float previousGain[Const_NumChannels] { 0.0f, 0.0f };

    //==================================================================================================================
    // GUI DATA (only data which is solely considered while loading and saving)
    juce::Rectangle<int> windowBounds;

    //==================================================================================================================
    void initialize();
    float calculatePanningGain(int, int) const noexcept;
    
    //======================================================================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout getParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossinAudioProcessor)

};
