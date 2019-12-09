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

//=====================================================================================================================
CossinAudioProcessor::CossinAudioProcessor()
     : AudioProcessor (BusesProperties().withInput("Input", AudioChannelSet::stereo(), true)
                                        .withOutput("Output", AudioChannelSet::stereo(), true)),
       atcd(JT_IS_STANDALONE_INLINE(nullptr, new jaut::ScopedATCD)),
       parameters(*this, nullptr),
       previousGain{0.0f, 0.0f},
       processorContainer(*this, parameters, undoer)
{
    initialization();
    makeParameters();
    metreSource.setMaxHoldMS(50);
    parameters.state = ValueTree("PluginState");

    jaut::JAUT_DISABLE_THREAD_DIST_EXPLICIT(false);
}

CossinAudioProcessor::~CossinAudioProcessor() {}

//=====================================================================================================================
const String CossinAudioProcessor::getName() const
{
    return "Cossin";
}

//=====================================================================================================================
void CossinAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    previousGain[0] = parGain->getValue() * calculatePanningGain(0);
    previousGain[1] = parGain->getValue() * calculatePanningGain(1);
    processorContainer.prepareToPlay(sampleRate, samplesPerBlock);
    metreSource.resize(getMainBusNumOutputChannels(), static_cast<int>(0.02f * sampleRate / samplesPerBlock));
}

void CossinAudioProcessor::releaseResources()
{
    processorContainer.releaseResources();
}

bool CossinAudioProcessor::isBusesLayoutSupported (const BusesLayout &layouts) const
{

    if ((layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        || layouts.getMainOutputChannelSet() == AudioChannelSet::disabled())
    {
        return false;
    }

    return layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}

void CossinAudioProcessor::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midi)
{
    ScopedNoDenormals nodenormals;

    for (auto i = 0; i < buffer.getNumChannels(); ++i)
    {
        float currentGain = parGain->getValue() * calculatePanningGain(i);
        
        if (currentGain == previousGain[i])
        {
            buffer.applyGain(i, 0, buffer.getNumSamples(), currentGain);
        }
        else
        {
            buffer.applyGainRamp(i, 0, buffer.getNumSamples(), previousGain[i], currentGain);
            previousGain[i] = currentGain;
        }
    }

    processorContainer.processBlock(buffer, midi);
    metreSource.measureBlock(buffer);
}

//=====================================================================================================================
bool CossinAudioProcessor::hasEditor() const
{
    return true;
}

AudioProcessorEditor* CossinAudioProcessor::createEditor()
{
    return new CossinAudioProcessorEditor(*this, parameters, properties, metreSource, processorContainer);
}

//=====================================================================================================================
void CossinAudioProcessor::getStateInformation(MemoryBlock &destData)
{
    ValueTree stateData = parameters.copyState();

    if (stateData.isValid())
    {
        JT_IS_STANDALONE({})
        JT_STANDALONE_ELSE
        (
            // set gui data
            stateData.setProperty("GuiWidth",  windowBounds.getWidth(), nullptr);
            stateData.setProperty("GuiHeight", windowBounds.getHeight(), nullptr);
        )

        // edit data
        ValueTree propertyData = stateData.getOrCreateChildWithName("Properties", nullptr);
        propertyData.copyPropertiesAndChildrenFrom(properties.writeTo("Properties"), nullptr);

        // save processor manager
        processorContainer.writeData(stateData.getOrCreateChildWithName("ProcessorContainer", 0));

        // save data
        std::unique_ptr<XmlElement> xml(std::move(stateData.createXml()));
        this->copyXmlToBinary(*xml, destData);
    
      #ifdef JUCE_DEBUG
        xml->writeTo(File::getCurrentWorkingDirectory().getChildFile("../../plugin_state.xml"));
      #endif
    }
}

void CossinAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() && xml->hasTagName(parameters.state.getType()))
    {
        ValueTree state = ValueTree::fromXml(*xml);

        if (state.isValid())
        {
            JT_IS_STANDALONE({})
            JT_STANDALONE_ELSE
            (
                // set gui data
                windowBounds.setBounds(0, 0, state.getProperty("GuiWidth", -1), state.getProperty("GuiHeight", -1));
            )

            // apply property data
            ValueTree propertyTree = state.getChildWithName("Properties");

            if (propertyTree.isValid())
            {
                properties.readFrom(propertyTree);
            }

            // load processor manager
            processorContainer.readData(state.getChildWithName("ProcessorContainer"));

            // apply parameter data
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
void CossinAudioProcessor::initialization()
{
    properties.addListener(this);
    SharedData::ReadLock lock(*sharedData);

    // default init properties
    const jaut::Config &config = sharedData->Configuration();
    const jaut::Config::Property property_window_size = config.getProperty("size",      res::Cfg_Defaults);
    const jaut::Config::Property property_panning     = config.getProperty("panning",   res::Cfg_Defaults);
    const jaut::Config::Property property_processor   = config.getProperty("processor", res::Cfg_Defaults);

    properties.createProperty("PanningLaw",        property_panning.getValue());
    properties.createProperty("SelectedProcessor", property_processor.getValue());

    JT_IS_STANDALONE({})
    JT_STANDALONE_ELSE
    (
        windowBounds.setBounds(0, 0, property_window_size.getProperty("width") .getValue(),
                                     property_window_size.getProperty("height").getValue());
    )
}

void CossinAudioProcessor::makeParameters()
{
    using Parameter = AudioProcessorValueTreeState::Parameter;

    parGain = parameters.createAndAddParameter(std::make_unique<Parameter>("par_master_level", "Global level", "",
                                               NormalisableRange(0.0f, 1.0f, 0.0f, 0.5f), 1.0f,
    [](float value)
    {
        String db = String(round(Decibels::gainToDecibels(value) * 100.0f) / 100.0f);
        String floating = String(round(value * 100.0f) / 100.0f);
        return (value > 0 ? db : "-Inf") + "dB " + floating;
    }, nullptr));

    parameters.createAndAddParameter(std::make_unique<Parameter>("par_master_mix", "Global mix", "",
                                     NormalisableRange(0.0f, 1.0f), 1.0f,
    [](float value)
    {
        return String(static_cast<int>(value * 100)) + "%";
    }, nullptr));

    parPanning = parameters.createAndAddParameter(std::make_unique<Parameter>("par_master_panning", "Global panning",
                                                  "", NormalisableRange<float>(-1.0f, 1.0f, 0.0f, 0.5f, true), 0.0f,
    [](float value)
    {
        int absValue = static_cast<int>(abs(value * 100));
        int lesserValue = 100 - absValue;
        int biggerValue = 100 + absValue;
        String mixedText = value < 0.0f ? (String(biggerValue) + "% Left, " + String(lesserValue) + "% Right")
                                        : (String(lesserValue) + "% Left, " + String(biggerValue) + "% Right");
        String valueText = value == 1.0f ? "Right" : (value == -1.0f ? "Left" : (value == 0.0f ? "Center" : mixedText));
        return valueText;
    }, nullptr));
}

float CossinAudioProcessor::calculatePanningGain(int channel) const noexcept
{
    const float panning_p   = parPanning->convertFrom0to1(parPanning->getValue()) / 2.0f + 0.5f;
    const float channel_pan = channel == 0 ? 1.0f - panning_p : panning_p;
    const int panning_mode  = properties.getProperty("PanningLaw");
    float result            = 1.0f;

    jassert(jaut::is_in_range(panning_mode, 0, 2)); // there are only linear, square and sine panning

    if(panning_mode == 0) // linear
    {
        result = channel_pan * Const_LinearPanningCompensation;
    }
    else if(panning_mode == 1) // square
    {
        result = std::sqrt(channel_pan) * Const_SquarePanningCompensation;
    }
    else if(panning_mode == 2) // sine
    {
        result = std::sin(channel_pan * (Const_Pi / 2.0f)) * Const_SinePanningCompensation;
    }

    JUCE_UNDENORMALISE(result)

    return result;
}

void CossinAudioProcessor::onValueChanged(const String &name, var oldValue, var newValue)
{
    if(name == "SelectedProcessor")
    {
        processorContainer.currentProcessor = newValue;
    }
}

//=====================================================================================================================
AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new CossinAudioProcessor();
}

#if(1) //unused
bool CossinAudioProcessor::acceptsMidi() const
{
   return false;
}
bool CossinAudioProcessor::producesMidi() const
{
   return false;
}
bool CossinAudioProcessor::isMidiEffect() const
{
   return false;
}
double CossinAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}
int CossinAudioProcessor::getNumPrograms()
{
    return 1;
}
int CossinAudioProcessor::getCurrentProgram()
{
    return 0;
}
void CossinAudioProcessor::setCurrentProgram (int)
{}
const String CossinAudioProcessor::getProgramName (int)
{
    return {};
}
void CossinAudioProcessor::changeProgramName (int, const String&)
{}
#endif //unused
