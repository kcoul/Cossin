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
    @file   PluginProcessor.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include "CossinDef.h"
#include "EffectModules.h"

#include <ff_meters/ff_meters.h>
#include <jaut_audio/jaut_audio.h>

inline constexpr int Const_NumChannels = 2;
inline constexpr int Const_MaxMacros   = 30;

struct ParameterIds
{
    static constexpr const char *MasterLevel = "par_master_level";
    static constexpr const char *MasterMix   = "par_master_mix";
    static constexpr const char *MasterPan   = "par_master_pan";
    
    static constexpr const char *PropertyPanningMode = "property_panning_law";
    static constexpr const char *PropertyProcessMode = "property_process_mode";
};

class SharedData;

class CossinAudioProcessor final : public juce::AudioProcessor
{
public:
    class ParameterList
    {
    public:
        explicit ParameterList(CossinAudioProcessor &p)
            : p(p)
        {
            {
                auto macro_group = std::make_unique<juce::AudioProcessorParameterGroup>("macros", "Macros", "-");
                
                for (int i = 0; i < Const_MaxMacros; ++i)
                {
                    const juce::String macnr(i + 1);
                    auto par = std::make_unique<juce::AudioParameterFloat>("macro_" + macnr, "Macro #" + macnr,
                                                                           juce::NormalisableRange<float>(0.0f, 1.0f),
                                                                           1.0f);
                    
                    macroParameters[static_cast<jaut::SizeTypes::Array>(i)] = par.get();
                    macro_group->addChild(std::move(par));
                }
                
                parGroupMacros = macro_group.get();
                p.addParameterGroup(std::move(macro_group));
            }
            
            for (auto &par : createMainParameters())
            {
                if (par)
                {
                    mainParameters.emplace(par->paramID, par);
                    p.addParameter(par);
                }
            }
        }
    
        //==============================================================================================================
        juce::RangedAudioParameter* getMainParameter(const juce::String &id)
        {
            auto it = mainParameters.find(id);
            return it != mainParameters.end() ? it->second : nullptr;
        }
        
        juce::RangedAudioParameter* getMacroParameter(int index)
        {
            jassert(jaut::fit(index, 0, Const_MaxMacros));
            return macroParameters.at(static_cast<jaut::SizeTypes::Array>(index));
        }
        
    private:
        friend class CossinAudioProcessor;
        
        //==============================================================================================================
        CossinAudioProcessor &p;
        
        std::unordered_map<juce::String, juce::RangedAudioParameter*> mainParameters;
        std::array<juce::RangedAudioParameter*, Const_MaxMacros>      macroParameters { nullptr };
        
        juce::AudioProcessorParameterGroup *parGroupMacros { nullptr };
        juce::AudioParameterFloat          *parGain        { nullptr };
        juce::AudioParameterFloat          *parPanning     { nullptr };
        juce::AudioParameterFloat          *parMix         { nullptr };
        juce::AudioParameterInt            *parPanMode     { nullptr };
        juce::AudioParameterInt            *parProcMode    { nullptr };
    
        //==============================================================================================================
        std::vector<juce::RangedAudioParameter*> createMainParameters();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterList)
    };
    
    //==================================================================================================================
    using EffectProcessorList = jaut::TypeArray<EffectEqualiser>;
    using TopProcessorList    = jaut::TypeArray<EffectProcessorList::to<jaut::AudioProcessorSet>>;
    using FrameProcessor      = EffectProcessorList::to<jaut::AudioProcessorSet>;
    using TopProcessor        = TopProcessorList   ::to<jaut::AudioProcessorSet>;
    
    //==================================================================================================================
    static constexpr int Resolution_LookupTable = 200;
    
    //==================================================================================================================
    CossinAudioProcessor();
    ~CossinAudioProcessor() override;
    
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
    juce::Rectangle<int>& getWindowSize()    noexcept;
    ParameterList&        getParameterList() noexcept;
    
private:
    static BusesProperties getDefaultBusesLayout()
    {
        return BusesProperties().withInput ("Input",     juce::AudioChannelSet::stereo())
                                .withOutput("Output",    juce::AudioChannelSet::stereo())
                                .withInput ("Sidechain", juce::AudioChannelSet::mono());
    }
    
    //==================================================================================================================
    std::array<float, Resolution_LookupTable + 1> sineTable;
    std::array<float, Resolution_LookupTable + 1> sqrtTable;
    juce::SharedResourcePointer<SharedData> sharedData;
    juce::UndoManager undoManager;
    ParameterList parameters;
    TopProcessor processors;
    foleys::LevelMeterSource metreSource;
    float previousGain[Const_NumChannels] { 0.0f, 0.0f };

    //==================================================================================================================
    // GUI DATA (only data which is solely considered while loading and saving)
    juce::Rectangle<int> windowBounds;

    //==================================================================================================================
    void initialize();
    float calculatePanningGain(int, int) const noexcept;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CossinAudioProcessor)
};
