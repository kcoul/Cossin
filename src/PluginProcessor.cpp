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
    @file   PluginProcessor.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "SharedData.h"
#include "Resources.h"

#include <jaut_provider/jaut_provider.h>

namespace
{
struct ProcInit
{
    //==================================================================================================================
    using TopSet   = CossinAudioProcessor::TopProcessor;
    using FrameSet = CossinAudioProcessor::FrameProcessor;
    
    //==================================================================================================================
    template<class>
    struct FrameInit;
    
    template<class ...Processors>
    struct FrameInit<jaut::TypeArray<Processors...>>
    {
        ProcInit &procInit;
        
        //==============================================================================================================
        explicit FrameInit(ProcInit &procInit)
            : procInit(procInit)
        {}
        
        //==============================================================================================================
        auto operator()() -> typename FrameSet::ProcessorArray
        {
            // Automatic effect initialisation, doesn't need to be touched
            return {
                (FrameSet::template makeProcessor<Processors>(procInit.undoManager), ...)
            };
        }
    };
    
    //==================================================================================================================
    juce::UndoManager &undoManager;
    
    //==================================================================================================================
    explicit ProcInit(juce::UndoManager &undoManager)
        : undoManager(undoManager)
    {}
    
    //==================================================================================================================
    auto operator()() -> typename CossinAudioProcessor::TopProcessor::ProcessorArray
    {
        // TODO more processors
        return {
            TopSet::template makeProcessor<FrameSet>(FrameInit<CossinAudioProcessor::EffectProcessorList>(*this))
        };
    }
};

//======================================================================================================================
inline constexpr float Const_Pi                        = 3.14159f;
inline constexpr float Const_LinearPanningCompensation = 2.0f;
inline constexpr float Const_SquarePanningCompensation = 1.41421356238f;
inline constexpr float Const_SinePanningCompensation   = 1.41421356238f;

//======================================================================================================================
template<class Member, class ...Args>
juce::RangedAudioParameter* newParameter(Member *&member, const char *id, Args &&...args)
{
    return (member = new Member(id, std::forward<Args>(args)...));
}

auto createSineTable() noexcept
{
    std::array<float, CossinAudioProcessor::Resolution_LookupTable + 1> table {};
    constexpr int size = static_cast<jaut::SizeTypes::Array>(table.size());
    
    for (int i = 0; i < size; ++i)
    {
        table[static_cast<jaut::SizeTypes::Array>(i)] = std::sqrt(static_cast<float>(i) / 100.0f) *
                                                        Const_SquarePanningCompensation;
    }
    
    return table;
}

auto createSquareTable() noexcept
{
    std::array<float, CossinAudioProcessor::Resolution_LookupTable + 1> table {};
    constexpr int size = static_cast<jaut::SizeTypes::Array>(table.size());
    
    for (int i = 0; i < size; ++i)
    {
        table[static_cast<jaut::SizeTypes::Array>(i)] = std::sin((static_cast<float>(i) / 100.0f) * (Const_Pi / 2.0f))
                                                        * Const_SinePanningCompensation;
    }
    
    return table;
}
}

//======================================================================================================================
CossinAudioProcessor::CossinAudioProcessor()
     : AudioProcessor(getDefaultBusesLayout()),
       sineTable(::createSineTable()),
       sqrtTable(::createSquareTable()),
       parameters(*this),
       processors(::ProcInit(undoManager))
{
    initialize();
}

CossinAudioProcessor::~CossinAudioProcessor() = default;

//======================================================================================================================
const juce::String CossinAudioProcessor::getName() const
{
    return "Cossin";
}

//======================================================================================================================
void CossinAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const float gain   = parameters.parGain   ->get();
    const int pan_mode = parameters.parPanMode->get();
    previousGain[0]    = gain * calculatePanningGain(pan_mode, 0);
    previousGain[1]    = gain * calculatePanningGain(pan_mode, 1);
    
    metreSource.resize(getMainBusNumOutputChannels(), static_cast<int>(0.02f * sampleRate / samplesPerBlock));
}

void CossinAudioProcessor::releaseResources()
{}

bool CossinAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    const juce::AudioChannelSet main_bus = layouts.getMainOutputChannelSet();

    if((main_bus != juce::AudioChannelSet::mono() && main_bus != juce::AudioChannelSet::stereo())
       || main_bus == juce::AudioChannelSet::disabled())
    {
        return false;
    }

    return main_bus == layouts.getMainInputChannelSet();
}

void CossinAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals denormals;

    const float gain   = parameters.parGain   ->get();
    const int pan_mode = parameters.parPanMode->get();
    
    for (auto i = 0; i < buffer.getNumChannels(); ++i)
    {
        const float current_gain = gain * calculatePanningGain(pan_mode, i);
        
        if (current_gain == previousGain[i])
        {
            buffer.applyGain(i, 0, buffer.getNumSamples(), current_gain);
        }
        else
        {
            buffer.applyGainRamp(i, 0, buffer.getNumSamples(), previousGain[i], current_gain);
            previousGain[i] = current_gain;
        }
    }

    metreSource.measureBlock(buffer);
}

//======================================================================================================================
bool CossinAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* CossinAudioProcessor::createEditor()
{
    return new CossinMainEditorWindow(*this, metreSource);
}

//======================================================================================================================
void CossinAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::XmlElement plugin_state("Cossin");
    
    {
        juce::XmlElement *const params_state = plugin_state.createNewChildElement("Parameters");
        
        // Main parameters
        {
            juce::XmlElement *const master_state = params_state->createNewChildElement("Master");
            
            for (auto &[key, par] : parameters.mainParameters)
            {
                juce::XmlElement *const parameter_state = master_state->createNewChildElement("Parameter");
                parameter_state->setAttribute("id",    key);
                parameter_state->setAttribute("value", par->getValue());
            }
        }
        
        // Macro parameters
        {
            juce::XmlElement *const macro_state = params_state->createNewChildElement("Macros");
            
            for (int i = 0; i < static_cast<int>(parameters.macroParameters.size()); ++i)
            {
                juce::XmlElement *const parameter_state = macro_state->createNewChildElement("Macro");
                parameter_state->setAttribute("index", i);
                parameter_state->setAttribute("value",
                                              parameters.macroParameters.at(static_cast<jaut::SizeTypes::Array>(i))
                                                        ->getValue());
            }
        }
    }
    
    COSSIN_IS_STANDALONE()
    COSSIN_STANDALONE_ELSE
    (
        // Gui data
        plugin_state.setAttribute("width",  windowBounds.getWidth());
        plugin_state.setAttribute("height", windowBounds.getHeight());
    )
    
    juce::ValueTree processor_state("Processors");
    processors.writeData(processor_state);
    plugin_state.addChildElement(processor_state.createXml().release());
    
    copyXmlToBinary(plugin_state, destData);

#ifdef JUCE_DEBUG
    // Store output debug information
    plugin_state.writeTo(juce::File::getCurrentWorkingDirectory().getChildFile("../../plugin_state.xml"));
#endif
}

void CossinAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    const std::unique_ptr<const juce::XmlElement> plugin_state(getXmlFromBinary(data, sizeInBytes));

    if (plugin_state.get() && plugin_state->hasTagName("Cossin"))
    {
        if (const juce::XmlElement *const params_state = plugin_state->getChildByName("Parameters"))
        {
            if (const juce::XmlElement *const master_state = params_state->getChildByName("Master"))
            {
                forEachXmlChildElement(*master_state, parameter_state)
                {
                    auto it = parameters.mainParameters.find(parameter_state->getStringAttribute("id", ""));
                    
                    if (it != parameters.mainParameters.end())
                    {
                        const auto value = static_cast<float>(parameter_state->getDoubleAttribute("value", 1.0));
                        it->second->setValueNotifyingHost(value);
                    }
                }
            }
            
            if (const juce::XmlElement *const macro_state = params_state->getChildByName("Macros"))
            {
                forEachXmlChildElement(*macro_state, parameter_state)
                {
                    const auto index = static_cast<jaut::SizeTypes::Array>(parameter_state
                                                                           ->getIntAttribute("index", -1));
                    
                    if (jaut::fit<int>(index, 0, Const_MaxMacros))
                    {
                        const auto value = static_cast<float>(parameter_state->getDoubleAttribute("value", 1.0));
                        parameters.macroParameters.at(index)->setValueNotifyingHost(value);
                    }
                }
            }
        }
        
        COSSIN_IS_STANDALONE()
        COSSIN_STANDALONE_ELSE
        (
            const int width  = plugin_state->getIntAttribute("width",  -1);
            const int height = plugin_state->getIntAttribute("height", -1);
            windowBounds.setBounds(0, 0, width, height);
        )
        
        if (const juce::XmlElement *const processor_state_xml = plugin_state->getChildByName("Processors"))
        {
            const juce::ValueTree processor_state = juce::ValueTree::fromXml(*processor_state_xml);
            processors.readData(processor_state);
        }
    }
}

//======================================================================================================================
juce::Rectangle<int>& CossinAudioProcessor::getWindowSize() noexcept
{
    return windowBounds;
}

CossinAudioProcessor::ParameterList& CossinAudioProcessor::getParameterList() noexcept
{
    return parameters;
}

//======================================================================================================================
void CossinAudioProcessor::initialize()
{
    {
        SharedData::ReadLock lock(*sharedData);
        
        // Default init properties
        const jaut::Config &config = sharedData->Configuration();
        auto property_window_size = config.getProperty("size", res::Cfg_Defaults);
        
        COSSIN_IS_STANDALONE()
        COSSIN_STANDALONE_ELSE
        (
            windowBounds.setBounds(0, 0, property_window_size->getProperty("width")->getValue(),
                                         property_window_size->getProperty("height")->getValue());
        )
    }
    
    // Misc
    metreSource.setMaxHoldMS(50);
}

float CossinAudioProcessor::calculatePanningGain(int panMode, int channel) const noexcept
{
    jassert(jaut::fit<int>(panMode, 0, res::List_PanningModes.size()));

    if (panMode == 0) // linear
    {
        const float panning_p    = parameters.parPanning->get() / 2.0f + 0.5f;
        const float channel_mod  = channel == 0 ? 1.0f - panning_p : panning_p;
        return channel_mod * Const_LinearPanningCompensation;
    }
    else
    {
        const int panning_p   = juce::roundToInt(parameters.parPanning->get() * 100.0f) + 100;
        const int table_index = channel == 0 ? 200 - panning_p : panning_p;
        return panMode == 1 ? sqrtTable[static_cast<jaut::SizeTypes::Array>(table_index)]
                            : sineTable[static_cast<jaut::SizeTypes::Array>(table_index)];
    }
}

//======================================================================================================================
std::vector<juce::RangedAudioParameter*> CossinAudioProcessor::ParameterList::createMainParameters()
{
    using Range = juce::NormalisableRange<float>;
    
    //==================================================================================================================
    const int last_panning_mode = res::List_PanningModes.size() - 1;
    const int last_process_mode = res::List_ProcessModes.size() - 1;
    
    int default_pan_mode;
    int default_processor;
    
    {
        SharedData::ReadLock lock(*p.sharedData);
        
        const jaut::Config &config = p.sharedData->Configuration();
        default_pan_mode  = std::clamp<int>(config.getProperty(res::Prop_DefaultsPanningMode, res::Cfg_Defaults)
                                                  ->getValue(), 0, last_panning_mode);
        default_processor = std::clamp<int>(config.getProperty(res::Prop_DefaultsProcessMode, res::Cfg_Defaults)
                                                  ->getValue(), 0, last_process_mode);
    }
    
    return {
        // Volume parameter
        ::newParameter(parGain, ParameterIds::MasterLevel, "Global level", Range(0.0f, 1.0f, 0.0f, 0.5f), 1.0f, "",
                       juce::AudioProcessorParameter::Category::genericParameter,
                       [](float value, int maximumStringLength)
                       {
                           const juce::String db(std::round(juce::Decibels::gainToDecibels(value) * 100.0f) / 100.0f);
                           return ((value > 0 ? db : "-INF") + "dB").substring(maximumStringLength);
                       }),
        
        // Mix parameter
        ::newParameter(parMix, ParameterIds::MasterMix, "Global mix", Range(0.0f, 1.0f), 1.0f, "",
                       juce::AudioProcessorParameter::Category::genericParameter,
                       [](float value, int maximumStringLength)
                       {
                            return (juce::String(static_cast<int>(value * 100)) + "%")
                                    .substring(maximumStringLength);
                       }),
        
        // Panning parameter
        ::newParameter(parPanning, ParameterIds::MasterPan, "Global pan", Range(-1.0f, 1.0f, 0.01f, 0.5f, true), 0.0f,
                       "", juce::AudioProcessorParameter::Category::genericParameter,
                       [](float value, int maximumStringLength) -> juce::String
                       {
                           if (std::fmod(value, 1.0f) == 0.0f)
                           {
                               return value == 0.0f ? "Center" : (value == 1.0f ? "Right" : "Left");
                           }
    
                           const int mod = static_cast<int>(value * 100.0f);
                           return (juce::String(100 - mod) + "% Left, " + juce::String(100 + mod) + "% Right")
                                   .substring(maximumStringLength);
                       }),
                       
        // Panning law parameter
        ::newParameter(parPanMode, ParameterIds::PropertyPanningMode, "Pan mode", 0, last_panning_mode,
                       default_pan_mode, "",
                       [](int value, int maximumStringLength)
                       {
                           return juce::String(res::List_PanningModes[static_cast<jaut::SizeTypes::Array>(value)])
                                                    .substring(maximumStringLength);
                       }),
                       
        // Process mode
        (last_process_mode <= 0
             ? nullptr
             : ::newParameter(parProcMode, ParameterIds::PropertyProcessMode, "Process mode", 0, last_process_mode,
                              default_processor, "",
                              [](int value, int maximumStringLength)
                              {
                                  return juce::String(res::List_ProcessModes[static_cast<jaut::SizeTypes::Array>(value)])
                                                          .substring(maximumStringLength);
                              })
        )
    };
}

//======================================================================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CossinAudioProcessor();
}
