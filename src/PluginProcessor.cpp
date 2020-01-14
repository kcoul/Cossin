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
    @file   PluginProcessor.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "SharedData.h"
#include "Resources.h"

#include <jaut/appdata.h>
#include <jaut/config.h>

inline constexpr float Const_Pi                        = 3.14159f;
inline constexpr float Const_LinearPanningCompensation = 2.0f;
inline constexpr float Const_SquarePanningCompensation = 1.41421356238f;
inline constexpr float Const_SinePanningCompensation   = 1.41421356238f;

//======================================================================================================================
CossinAudioProcessor::CossinAudioProcessor()
     : AudioProcessor (BusesProperties().withInput ("Input",     AudioChannelSet::stereo())
                                        .withOutput("Output",    AudioChannelSet::stereo())
                                        .withInput ("Sidechain", AudioChannelSet::mono())),
       atcd(JT_IS_STANDALONE_INLINE(nullptr, new jaut::ScopedATCD)),
       parameters(*this, nullptr),
       topUnitRack(*this, &parameters, &undoManager),
       previousGain{0.0f, 0.0f}
{
    initialize();
    makeParameters();
}

CossinAudioProcessor::~CossinAudioProcessor() {}

//======================================================================================================================
const String CossinAudioProcessor::getName() const
{
    return "Cossin";
}

//======================================================================================================================
void CossinAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    previousGain[0] = parGain->getValue() * calculatePanningGain(0);
    previousGain[1] = parGain->getValue() * calculatePanningGain(1);
    metreSource.resize(getMainBusNumOutputChannels(), static_cast<int>(0.02f * sampleRate / samplesPerBlock));

    topUnitRack.beginPlayback(sampleRate, samplesPerBlock);
}

void CossinAudioProcessor::releaseResources()
{
    topUnitRack.finishPlayback();
}

bool CossinAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    const AudioChannelSet main_bus = layouts.getMainOutputChannelSet();

    if((main_bus != AudioChannelSet::mono() && main_bus != AudioChannelSet::stereo())
       || main_bus == AudioChannelSet::disabled())
    {
        return false;
    }

    return main_bus == layouts.getMainInputChannelSet();
}

void CossinAudioProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midi)
{
    ScopedNoDenormals no_denormals;

    for (auto i = 0; i < buffer.getNumChannels(); ++i)
    {
        float current_gain = parGain->getValue() * calculatePanningGain(i);
        
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

    topUnitRack.process(buffer, midi);
    metreSource.measureBlock(buffer);
}

//======================================================================================================================
bool CossinAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor *CossinAudioProcessor::createEditor()
{
    return new CossinAudioProcessorEditor(*this, parameters, properties, metreSource, topUnitRack);
}

//======================================================================================================================
void CossinAudioProcessor::getStateInformation(MemoryBlock &destData)
{
    ValueTree state = parameters.copyState();

    if (state.isValid())
    {
        JT_IS_STANDALONE({})
        JT_STANDALONE_ELSE
        (
            // Gui data
            state.setProperty("GuiWidth",  windowBounds.getWidth(), nullptr);
            state.setProperty("GuiHeight", windowBounds.getHeight(), nullptr);
        )

        // Property data
        ValueTree property_tree = state.getOrCreateChildWithName("Properties", nullptr);
        property_tree.copyPropertiesAndChildrenFrom(properties.writeTo("Properties"), nullptr);

        // Dump data
        std::unique_ptr<XmlElement> state_xml(std::move(state.createXml()));
        this->copyXmlToBinary(*state_xml, destData);

        // Store output debug information
      #ifdef JUCE_DEBUG
        state_xml->writeTo(File::getCurrentWorkingDirectory().getChildFile("../../plugin_state.xml"));
      #endif
    }
}

void CossinAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> state_xml(getXmlFromBinary(data, sizeInBytes));

    if (state_xml.get() && state_xml->hasTagName(parameters.state.getType()))
    {
        const ValueTree state = ValueTree::fromXml(*state_xml);

        if (state.isValid())
        {
            JT_IS_STANDALONE({})
            JT_STANDALONE_ELSE
            (
                // Gui data
                windowBounds.setBounds(0, 0, state.getProperty("GuiWidth", -1), state.getProperty("GuiHeight", -1));
            )

            // Property data
            properties.readFrom(state.getChildWithName("Properties"));

            // Contain data
            parameters.replaceState(state);
        }
    }
}

//======================================================================================================================
Rectangle<int> &CossinAudioProcessor::getWindowSize() noexcept
{
    return windowBounds;
}

//======================================================================================================================
void CossinAudioProcessor::initialize()
{
    SharedData::ReadLock lock(*sharedData);

    // Default init properties
    const jaut::Config &config = sharedData->Configuration();
    const jaut::Config::Property property_window_size = config.getProperty("size", res::Cfg_Defaults);
    const jaut::Config::Property property_panning     = config.getProperty("panning", res::Cfg_Defaults);
    const jaut::Config::Property property_processor   = config.getProperty("processor", res::Cfg_Defaults);

    properties.createProperty("PanningLaw",        property_panning.getValue());
    properties.createProperty("SelectedProcessor", property_processor.getValue());

    JT_IS_STANDALONE({})
    JT_STANDALONE_ELSE
    (
        windowBounds.setBounds(0, 0, property_window_size.getProperty("width").getValue(),
                                     property_window_size.getProperty("height").getValue());
    )

    // Misc
    metreSource.setMaxHoldMS(50);

    jaut::JAUT_DISABLE_THREAD_DIST_EXPLICIT(false);
}

void CossinAudioProcessor::makeParameters()
{
    using Parameter = AudioProcessorValueTreeState::Parameter;

    parGain = parameters.createAndAddParameter(std::make_unique<Parameter>("par_master_level", "Global level", "",
                                               NormalisableRange(0.0f, 1.0f, 0.0f, 0.5f), 1.0f,
    [](float value)
    {
        const String db       = String(round(Decibels::gainToDecibels(value) * 100.0f) / 100.0f);
        const String floating = String(round(value * 100.0f) / 100.0f);

        return (value > 0 ? db : "-Inf") + "dB " + floating;
    },
    nullptr));

    parameters.createAndAddParameter(std::make_unique<Parameter>("par_master_mix", "Global mix", "",
                                     NormalisableRange(0.0f, 1.0f), 1.0f,
    [](float value)
    {
        return String(static_cast<int>(value * 100)) + "%";
    },
    nullptr));

    parPanning = parameters.createAndAddParameter(std::make_unique<Parameter>("par_master_panning", "Global panning",
                                                  "", NormalisableRange<float>(-1.0f, 1.0f, 0.0f, 0.5f, true), 0.0f,
    [](float value)
    {
        const int abs_value    = static_cast<int>(abs(value * 100));
        const int lesser_value = 100 - abs_value;
        const int bigger_value = 100 + abs_value;
        const String mixedText = value < 0.0f ? (String(bigger_value) + "% Left, " + String(lesser_value) + "% Right")
                                               : (String(lesser_value) + "% Left, " + String(bigger_value) + "% Right");
        const String end = value == 1.0f ? "Right" : (value == -1.0f ? "Left" : (value == 0.0f ? "Center" : mixedText));

        return end;
    },
    nullptr));

    // Create and finish new state object
    parameters.state = ValueTree("PluginState");
}

float CossinAudioProcessor::calculatePanningGain(int channel) const noexcept
{
    const float panning_p     = parPanning->convertFrom0to1(parPanning->getValue()) / 2.0 + 0.5;
    const float channel_pan   = channel == 0 ? 1.0 - panning_p : panning_p;
    const int    panning_mode = properties.getProperty("PanningLaw");
    double       result       = 1.0f;

    jassert(jaut::fit_s(panning_mode, 0, res::Pan_Modes_Num));

    if (panning_mode == 0) // linear
    {
        result = channel_pan * Const_LinearPanningCompensation;
    }
    else if (panning_mode == 1) // square
    {
        result = std::sqrt(channel_pan) * Const_SquarePanningCompensation;
    }
    else if (panning_mode == 2) // sine
    {
        result = std::sin(channel_pan * (Const_Pi / 2.0)) * Const_SinePanningCompensation;
    }

    return result;
}

//======================================================================================================================
AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new CossinAudioProcessor();
}
