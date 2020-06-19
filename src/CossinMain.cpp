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
    @file   CossinMain.cpp
    @date   20, October 2019
    
    ===============================================================
 */

#include <juce_audio_plugin_client/juce_audio_plugin_client.h>

#include <memory>

#include "CossinDef.h"
#include "CossinMain.h"
#include "SharedData.h"
#include "Resources.h"

//**********************************************************************************************************************
// region Namespace
//======================================================================================================================
namespace
{
    inline constexpr char const *Id_Cache = "73783927-4465-43c1-8612-0f87be2b37ce";
    
    //==================================================================================================================
    int yncAlert(const jaut::Localisation &locale, const juce::String& title, const juce::String& message,
                 juce::AlertWindow::AlertIconType icon = juce::AlertWindow::WarningIcon) noexcept
    {
        const int result = juce::AlertWindow::showYesNoCancelBox(icon, locale.translate(title),
                                                                       locale.translate(message),
                                                                       locale.translate("alert.yes"),
                                                                       locale.translate("alert.no"),
                                                                       locale.translate("alert.cancel"));
        
        return result;
    }
}
//======================================================================================================================
// endregion Namespace
//**********************************************************************************************************************
// region Cossin
//======================================================================================================================
Cossin *JUCE_CALLTYPE Cossin::getInstance() noexcept
{
    return dynamic_cast<Cossin*>(JUCEApplicationBase::getInstance());
}

//======================================================================================================================
Cossin::Cossin()
{
    juce::PluginHostType::jucePlugInClientCurrentWrapperType = juce::AudioProcessor::wrapperType_Standalone;
}

Cossin::~Cossin()
{
    juce::Logger::setCurrentLogger(nullptr);
}

//======================================================================================================================
const juce::String Cossin::getApplicationName() // NOLINT
{
    return res::App_Name;
}

const juce::String Cossin::getApplicationVersion() // NOLINT
{
    return res::App_Version;
}

//======================================================================================================================
bool Cossin::isInputMuted() const noexcept
{
    return static_cast<bool>(mainWindow->pluginHolder->getMuteInputValue().getValue());
}

//======================================================================================================================
void Cossin::initialise(const juce::String&)
{
    mainWindow.reset(createWindow());

#if JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE
    Desktop::getInstance().setKioskModeComponent(mainWindow.get(), false);
#endif
    mainWindow->setVisible(true);
}

void Cossin::shutdown()
{
    if(mainWindow)
    {
        mainWindow->pluginHolder->savePluginCache();
    }

    mainWindow.reset();
}

//======================================================================================================================
void Cossin::systemRequestedQuit()
{
    if (mainWindow)
    {
        if(!mainWindow->pluginHolder->savePluginState(true, true))
        {
            return;
        }
    }

    if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
    {
        juce::Timer::callAfterDelay(100, []()
        {
            if (auto app = JUCEApplicationBase::getInstance())
            {
                app->systemRequestedQuit();
            }
        });
    }
    else
    {
        quit();
    }
}

//======================================================================================================================
CossinPluginWindow* Cossin::createWindow()
{
    return new CossinPluginWindow();
}
//======================================================================================================================
// endregion Cossin
//**********************************************************************************************************************
// region CossinPluginWrapper
//======================================================================================================================
CossinPluginWrapper::CossinPluginWrapper()
    : cacheLock(std::make_unique<juce::InterProcessLock>("COSSIN_" + juce::String(Id_Cache))),
#ifdef JucePlugin_PreferredChannelConfigurations
      channelConfiguration(juce::Array<StandalonePluginHolder::PluginInOuts>
                           (channels, juce::numElementsInArray(channels))),
#endif
      autoOpenMidiDevices(false)
{
    shouldMuteInput.addListener(this);
    shouldMuteInput = !isInterAppAudioConnected();

    createPlugin();

    int inChannels = (channelConfiguration.size() > 0 ? channelConfiguration[0].numIns
                                                      : processor->getMainBusNumInputChannels());

    const juce::String pref_device = "";
    const bool audioInputRequired  = inChannels > 0;

    if (audioInputRequired && juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio) &&
        !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio, [this, pref_device](bool granted)
        {
            init(granted, pref_device);
        });
    }
    else
    {
        init(audioInputRequired, pref_device);
    }
}

void CossinPluginWrapper::init(bool enableAudioInput, const juce::String &preferredDefaultDeviceName)
{
    reloadPluginCache();
    currentSaveFile = getLastFile();

    setupAudioDevices(enableAudioInput, preferredDefaultDeviceName, options.get());
    reloadPluginState();
    startPlaying();

    if (autoOpenMidiDevices)
    {
        startTimer(500);
    }
}

CossinPluginWrapper::~CossinPluginWrapper()
{
    stopTimer();
    deletePlugin();
    shutDownAudioDevices();
}

//======================================================================================================================
void CossinPluginWrapper::createPlugin(bool loadInitialState)
{
#if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
    processor.reset(juce::createPluginFilterOfType(juce::AudioProcessor::wrapperType_Standalone));
#else
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Standalone);
    processor.reset(juce::createPluginFilter());
    juce::AudioProcessor::setTypeOfNextNewPlugin(juce::AudioProcessor::wrapperType_Undefined);
#endif

    jassert(processor != nullptr);

    processor->disableNonMainBuses();
    processor->setRateAndBufferSizeDetails(44100, 512);

    int inChannels = (channelConfiguration.size() > 0 ? channelConfiguration[0].numIns
                                                      : processor->getMainBusNumInputChannels());

    int outChannels = (channelConfiguration.size() > 0 ? channelConfiguration[0].numOuts
                                                       : processor->getMainBusNumOutputChannels());

    processorHasPotentialFeedbackLoop = (inChannels > 0 && outChannels > 0);

    if(loadInitialState)
    {
        juce::MemoryBlock data;
        processor->getStateInformation(data);
        lastLoadedState = data.toBase64Encoding();
    }
}

void CossinPluginWrapper::deletePlugin()
{
    stopPlaying();
    processor = nullptr;
}

juce::String CossinPluginWrapper::getFilePatterns(const juce::String &fileSuffix)
{
    if (fileSuffix.isEmpty())
    {
        return {};
    }

    return (fileSuffix.startsWithChar('.') ? "*" : "*.") + fileSuffix;
}

//======================================================================================================================
juce::Value& CossinPluginWrapper::getMuteInputValue()
{
    return shouldMuteInput;

}
bool CossinPluginWrapper::getProcessorHasPotentialFeedbackLoop() const
{
    return processorHasPotentialFeedbackLoop;
}

void CossinPluginWrapper::valueChanged(juce::Value &value)
{
    muteInput = static_cast<bool>(value.getValue());
}

//======================================================================================================================
juce::File CossinPluginWrapper::getLastFile() const
{
    return cossinCache->getValue("lastStateFile", sharedData->AppData().getChildFile("Data/Saves").getFullPathName());
}

void CossinPluginWrapper::setLastFile(const juce::File &file)
{
    cossinCache->setValue("lastStateFile", file.getFullPathName());
    savePluginCache();
}

bool CossinPluginWrapper::askUserToSaveState(const juce::String &fileSuffix)
{
#if JUCE_MODAL_LOOPS_PERMITTED
    const jaut::Localisation &locale = sharedData->Localisation();
    juce::FileChooser chooser(locale.translate("state.chooser.save.title"), getLastFile(), getFilePatterns(fileSuffix));

    if (chooser.browseForFileToSave(true))
    {
        juce::MemoryBlock data;
        const juce::File file = chooser.getResult();
        processor->getStateInformation(data);
        
        if (file.create().wasOk() && file.replaceWithText(data.toBase64Encoding()))
        {
            setLastFile(file);
            return true;
        }
        else
        {
            const int result = ::yncAlert(locale, "state.save.err.title", "state.save.err.text",
                                          juce::AlertWindow::QuestionIcon);

            if(result == 1)
            {
                return askUserToSaveState(fileSuffix);
            }

            return result;
        }
    }

    return false;
#else
    ignoreUnused(fileSuffix);
    return true;
#endif
}

bool CossinPluginWrapper::askUserToLoadState(const juce::String &fileSuffix)
{
#if JUCE_MODAL_LOOPS_PERMITTED
    const jaut::Localisation &locale = sharedData->Localisation();
    juce::FileChooser chooser(locale.translate("state.chooser.load.title"), getLastFile(), getFilePatterns(fileSuffix));

    if (chooser.browseForFileToOpen())
    {
        juce::MemoryBlock data;
        const juce::File file = chooser.getResult();
        const juce::String base64 = file.loadFileAsString();

        if (data.fromBase64Encoding(base64))
        {
            processor->setStateInformation(data.getData(), static_cast<int>(data.getSize()));
            currentSaveFile = file;
            lastLoadedState = base64;
            setLastFile(file);

            return true;
        }
        else
        {
            const int result = juce::AlertWindow::showOkCancelBox(juce::AlertWindow::WarningIcon,
                                                                  locale.translate("state.chooser.load.err.title"),
                                                                  locale.translate("state.chooser.load.err.text"));

            if(result == 1)
            {
                return askUserToLoadState(fileSuffix);
            }

            return result;
        }
    }

    return false;
#else
    ignoreUnused(fileSuffix);
    return true;
#endif
}

//======================================================================================================================
void CossinPluginWrapper::startPlaying()
{
    player.setProcessor(processor.get());

#if JucePlugin_Enable_IAA && JUCE_IOS
    if (auto device = dynamic_cast<juce::iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
    {
        processor->setPlayHead(device->getAudioPlayHead());
        device->setMidiMessageCollector(&player.getMidiMessageCollector());
    }
#endif
}

void CossinPluginWrapper::stopPlaying()
{
    player.setProcessor(nullptr);
}

//======================================================================================================================
bool CossinPluginWrapper::reloadAudioDeviceState(bool enableAudioInput, const juce::String &preferredDefaultDeviceName,
                                                 const juce::AudioDeviceManager::AudioDeviceSetup *preferredSetupOptions)
{
    const auto savedState = std::make_unique<juce::XmlElement>("DEVICESETUP");
    auto &config          = sharedData->Configuration();
    auto prop_devices     = config.getProperty("devices", res::Cfg_Standalone);

    const juce::String output_device = prop_devices.getProperty("output").getValue();
    const juce::String input_device  = prop_devices.getProperty("input") .getValue();
    const juce::String device_type   = config.getProperty("deviceType", res::Cfg_Standalone).getValue();
    const double sample_rate         = config.getProperty("sampleRate", res::Cfg_Standalone).getValue();
    const int    buffer_size         = config.getProperty("bufferSize", res::Cfg_Standalone).getValue();

    savedState->setAttribute("audioOutputDeviceName", output_device.isEmpty() ? "default" : output_device);
    savedState->setAttribute("audioInputDeviceName",  input_device.isEmpty()  ? "default" : input_device);
    savedState->setAttribute("deviceType",            device_type.isEmpty()   ? "default" : device_type);
    savedState->setAttribute("audioDeviceRate",       sample_rate);
    savedState->setAttribute("audioDeviceBufferSize", buffer_size);

#if !(JUCE_IOS || JUCE_ANDROID)
    shouldMuteInput.setValue(static_cast<bool>(config.getProperty("muteInput", res::Cfg_Standalone).getValue()));
#endif

    int totalInChannels  = processor->getMainBusNumInputChannels();
    int totalOutChannels = processor->getMainBusNumOutputChannels();

    if (channelConfiguration.size() > 0)
    {
        auto defaultConfig = channelConfiguration.getReference(0);
        totalInChannels    = defaultConfig.numIns;
        totalOutChannels   = defaultConfig.numOuts;
    }

    const juce::String result = deviceManager.initialise(enableAudioInput ? totalInChannels : 0, totalOutChannels,
                                                         savedState.get(), true, preferredDefaultDeviceName,
                                                         preferredSetupOptions);

    if (!result.isEmpty())
    {
    
        juce::Logger::writeToLog("An exception occurred while trying to initialize the main audio device:\n" + result);
        
        return false;
    }

    return true;
}

bool CossinPluginWrapper::savePluginCache() // NOLINT
{
    if (cossinCache)
    {
        return cossinCache->save();
    }

    return false;
}

bool CossinPluginWrapper::reloadPluginCache()
{
    if (!cossinCache)
    {
        juce::PropertiesFile::Options opts;
        opts.storageFormat = juce::PropertiesFile::storeAsCompressedBinary;
        opts.processLock   = cacheLock.get();
        cossinCache        = std::make_unique<juce::PropertiesFile>(sharedData->AppData().getChildFile(".cache"),
                                                                    opts);
        return true;
    }

    return cossinCache->reload();
}

bool CossinPluginWrapper::savePluginState(bool askToSave, bool askIfNotSuccessful, bool notifyOnFail)
{
    if (!processor)
    {
        return false;
    }
    
    juce::MemoryBlock data;
    processor->getStateInformation(data);
    const juce::String base64 = data.toBase64Encoding();
    const auto &locale  = sharedData->Localisation();

    if (base64 == lastLoadedState)
    {
        return true;
    }

    if (askToSave)
    {
        const int result = ::yncAlert(locale, "state.save.new.title", "state.save.new.text",
                                      juce::AlertWindow::QuestionIcon);

        if(result != 1)
        {
            return result;
        }
    }

    if (!currentSaveFile.isDirectory() && currentSaveFile.replaceWithText(base64))
    {
        if(cossinCache)
        {
            cossinCache->setValue("lastStateFile", currentSaveFile.getFullPathName());
        }
    }
    else if (askIfNotSuccessful || askToSave)
    {
        if(!askToSave)
        {
            const int result = ::yncAlert(locale, "state.save.err.title", "state.save.err.text",
                                          juce::AlertWindow::WarningIcon);

            if(result == 1)
            {
                return askUserToSaveState(".dat");
            }

            return result;
        }

        return askUserToSaveState(".dat");
    }
    else if (notifyOnFail)
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::NoIcon, locale.translate("state.save.fail.title"),
                                          locale.translate("state.save.fail.text"), locale.translate("alert.ok"));

        return false;
    }

    return true;
}

bool CossinPluginWrapper::reloadPluginState(bool askToLoad, bool askIfNotSuccessful, bool notifyOnFail)
{
    if (!processor)
    {
        return false;
    }

    const auto &locale  = sharedData->Localisation();
    const juce::String base64 = currentSaveFile.loadFileAsString();
    juce::MemoryBlock data;

    if (askToLoad)
    {
        const int result = ::yncAlert(locale, "state.load.file.title", "state.load.file.text",
                                      juce::AlertWindow::QuestionIcon);

        if(result != 1)
        {
            return result;
        }
    }

    if (currentSaveFile.exists() && !currentSaveFile.isDirectory() && data.fromBase64Encoding(base64)
        && data.getSize() > 0)
    {
        processor->setStateInformation(data.getData(), static_cast<int>(data.getSize()));
        lastLoadedState = base64;
    }
    else if (askIfNotSuccessful || askToLoad)
    {
        if (!askToLoad)
        {
            const int result = ::yncAlert(locale, "state.load.err.title", "state.load.err.text",
                                          juce::AlertWindow::WarningIcon);

            if (result == 1)
            {
                return askUserToLoadState(".dat");
            }
            
            return result;
        }
        
        return askUserToLoadState(".dat");
    }
    else if (notifyOnFail)
    {
        juce::AlertWindow::showMessageBox(juce::AlertWindow::NoIcon, locale.translate("state.load.fail.title"),
                                          locale.translate("state.load.fail.text"), locale.translate("alert.ok"));
        return false;
    }

    return true;
}

//======================================================================================================================
void CossinPluginWrapper::switchToHostApplication()
{
#if JUCE_IOS
    if (auto device = dynamic_cast<juce::iOSAudioIODevice*>(deviceManager.getCurrentAudioDevice()))
    {
        device->switchApplication();
    }
#endif
}

bool CossinPluginWrapper::isInterAppAudioConnected() // NOLINT
{
#if JUCE_IOS
    if (auto device = dynamic_cast<juce::iOSAudioIODevice*>(deviceManager.getCurrentAudioDevice()))
    {
        return device->isInterAppAudioConnected();
    }
#endif

    return false;
}

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
juce::Image CossinPluginWrapper::getIAAHostIcon(int size) // NOLINT
{
#if JUCE_IOS && JucePlugin_Enable_IAA
    if (auto device = dynamic_cast<juce::iOSAudioIODevice*>(deviceManager.getCurrentAudioDevice()))
    {
        return device->getIcon(size);
    }
#else
    juce::ignoreUnused (size);
#endif

    return {};
}
#endif

//======================================================================================================================
CossinPluginWrapper *CossinPluginWrapper::getInstance()
{
#if JucePlugin_Enable_IAA || JucePlugin_Build_Standalone
    if (juce::PluginHostType::getPluginLoadedAs() == juce::AudioProcessor::wrapperType_Standalone)
    {
        const juce::Desktop &desktop = juce::Desktop::getInstance();
        const int numTopLevelWindows = desktop.getNumComponents();

        for (int i = 0; i < numTopLevelWindows; ++i)
        {
            if (auto *const window = dynamic_cast<CossinPluginWindow*>(desktop.getComponent(i)))
            {
                return window->getPluginHolder();
            }
        }
    }
#endif

    return nullptr;
}

//======================================================================================================================
void CossinPluginWrapper::audioDeviceIOCallback(const float **inputChannelData, int numInputChannels,
                                                float **outputChannelData, int numOutputChannels, int numSamples)
{
    if (muteInput)
    {
        emptyBuffer.clear();
        inputChannelData = emptyBuffer.getArrayOfReadPointers();
    }

    player.audioDeviceIOCallback(inputChannelData, numInputChannels, outputChannelData, numOutputChannels, numSamples);
}

void CossinPluginWrapper::audioDeviceAboutToStart(juce::AudioIODevice *device)
{
    emptyBuffer.setSize(device->getActiveInputChannels().countNumberOfSetBits(), device->getCurrentBufferSizeSamples());
    emptyBuffer.clear();
    player.audioDeviceAboutToStart(device);
    player.setMidiOutput(deviceManager.getDefaultMidiOutput());
}

void CossinPluginWrapper::audioDeviceStopped()
{
    player.setMidiOutput(nullptr);
    player.audioDeviceStopped();
    emptyBuffer.setSize(0, 0);
}

//======================================================================================================================
void CossinPluginWrapper::setupAudioDevices(bool enableAudioInput, const juce::String &preferredDefaultDeviceName,
                                            const juce::AudioDeviceManager::AudioDeviceSetup *preferredSetupOptions)
{
    deviceManager.addAudioCallback(this);
    deviceManager.addMidiInputDeviceCallback ({}, &player);
    reloadAudioDeviceState(enableAudioInput, preferredDefaultDeviceName, preferredSetupOptions);
}

void CossinPluginWrapper::shutDownAudioDevices()
{
    savePluginCache();
    deviceManager.removeMidiInputDeviceCallback({}, &player);
    deviceManager.removeAudioCallback(this);
}

void CossinPluginWrapper::timerCallback()
{
    auto newMidiDevices = juce::MidiInput::getAvailableDevices();

    if (newMidiDevices != lastMidiDevices)
    {
        for (auto &oldDevice : lastMidiDevices)
        {
            if (!newMidiDevices.contains(oldDevice))
            {
                deviceManager.setMidiInputDeviceEnabled(oldDevice.identifier, false);
            }
        }

        for (auto &newDevice : newMidiDevices)
        {
            if (!lastMidiDevices.contains(newDevice))
            {
                deviceManager.setMidiInputDeviceEnabled(newDevice.identifier, true);
            }
        }
        
        lastMidiDevices = newMidiDevices;
    }
}
//======================================================================================================================
// endregion CossinPluginWrapper
//**********************************************************************************************************************
// region CossinPluginWindow
//======================================================================================================================
CossinPluginWindow::CossinPluginWindow()
    : DocumentWindow("Cossin",
                     juce::LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), 7)
{
#if JUCE_IOS || JUCE_ANDROID
    setTitleBarHeight(0);
#else
    setTitleBarButtonsRequired(7, false);
    setUsingNativeTitleBar(true);
#endif

    pluginHolder = std::make_unique<CossinPluginWrapper>();

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
    setContentOwned(new MainContentComponent(*this), false);
#else

    setContentOwned(new MainContentComponent(*this), true);

    if (auto* cache = pluginHolder->cossinCache.get())
    {
        auto shared_data = SharedData::getInstance();

        const auto prop_size = shared_data->Configuration().getProperty("size", res::Cfg_Defaults);
        const int x          = cache->getIntValue("windowX", -100);
        const int y          = cache->getIntValue("windowY", -100);
        const int width      = cache->getIntValue("windowW", prop_size.getProperty("width") ->getValue());
        const int height     = cache->getIntValue("windowH", prop_size.getProperty("height")->getValue());

        if (x != -100 && y != -100)
        {
            setBoundsConstrained({x, y, width, height});
        }
        else
        {
            centreWithSize(width, height);
        }
    }
    else
    {
        centreWithSize(getWidth(), getHeight());
    }
#endif
}

CossinPluginWindow::~CossinPluginWindow()
{
#if (!JUCE_IOS) && (!JUCE_ANDROID)
    if (auto* cache = pluginHolder->cossinCache.get())
    {
        cache->setValue("windowX", getX());
        cache->setValue("windowY", getY());
        cache->setValue("windowW", getWidth());
        cache->setValue("windowH", getHeight());
    }
#endif

    pluginHolder->stopPlaying();
    clearContentComponent();
    pluginHolder = nullptr;
}

//======================================================================================================================
juce::AudioProcessor* CossinPluginWindow::getAudioProcessor() const noexcept
{
    return pluginHolder->processor.get();
}

juce::AudioDeviceManager& CossinPluginWindow::getDeviceManager() const noexcept
{
    return pluginHolder->deviceManager;
}

void CossinPluginWindow::resetToDefaultState()
{
    pluginHolder->stopPlaying();
    clearContentComponent();
    pluginHolder->deletePlugin();
    pluginHolder->currentSaveFile.replaceWithText("");
    pluginHolder->createPlugin();
    setContentOwned(new MainContentComponent(*this), true);
    pluginHolder->startPlaying();
}

//======================================================================================================================
void CossinPluginWindow::closeButtonPressed()
{
    if(pluginHolder->savePluginState(true, true))
    {
        juce::JUCEApplicationBase::quit();
    }
}

CossinPluginWrapper *CossinPluginWindow::getPluginHolder()
{
    return pluginHolder.get();
}
//======================================================================================================================
// endregion CossinPluginWindow
//**********************************************************************************************************************
// region MainContentComponent
//======================================================================================================================
CossinPluginWindow::MainContentComponent::MainContentComponent(CossinPluginWindow &filterWindow)
    : owner(filterWindow),
      editor(owner.getAudioProcessor()->hasEditor() ? owner.getAudioProcessor()->createEditorIfNeeded()
                                                    : new juce::GenericAudioProcessorEditor(*owner.getAudioProcessor()))
{
    if (editor != nullptr)
    {
        editor->addComponentListener(this);
        componentMovedOrResized(*editor, false, true);
        addAndMakeVisible(editor.get());
    }
}

CossinPluginWindow::MainContentComponent::~MainContentComponent()
{
    if (editor != nullptr)
    {
        editor->removeComponentListener(this);
        owner.pluginHolder->processor->editorBeingDeleted(editor.get());
        editor = nullptr;
    }
}

//======================================================================================================================
void CossinPluginWindow::MainContentComponent::resized()
{
    auto r = getLocalBounds();

    if (editor != nullptr)
    {
        editor->setBounds(editor->getLocalArea(this, r)
                                  .withPosition(r.getTopLeft().transformedBy(editor->getTransform().inverted())));
    }
}

//======================================================================================================================
void CossinPluginWindow::MainContentComponent::componentMovedOrResized(Component&, bool, bool)
{
    if (editor != nullptr)
    {
        auto rect = getSizeToContainEditor();
        setSize(rect.getWidth(), rect.getHeight());
    }
}

juce::Rectangle<int> CossinPluginWindow::MainContentComponent::getSizeToContainEditor() const
{
    if (editor != nullptr)
    {
        return getLocalArea(editor.get(), editor->getLocalBounds());
    }
        
    return {};
}
//======================================================================================================================
// endregion MainContentComponent
//**********************************************************************************************************************
// region Extern
//======================================================================================================================
#if JucePlugin_Build_Standalone && JUCE_IOS
    bool JUCE_CALLTYPE juce_isInterAppAudioConnected()
    {
        if (auto holder = CossinPluginWrapper::getInstance())
        {
            return holder->isInterAppAudioConnected();
        }
    
        return false;
    }
    
    void JUCE_CALLTYPE juce_switchToHostApplication()
    {
        if (auto holder = CossinPluginWrapper::getInstance())
        {
            holder->switchToHostApplication();
        }
    }

#   if JUCE_MODULE_AVAILABLE_juce_gui_basics
        Image JUCE_CALLTYPE juce_getIAAHostIcon(int size)
        {
            if (auto holder = CossinPluginWrapper::getInstance())
            {
                return holder->getIAAHostIcon(size);
            }
    
            return Image();
        }
#   endif
#endif

#if JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
    JUCE_CREATE_APPLICATION_DEFINE(Cossin)
#endif
//======================================================================================================================
// endregion Extern
//**********************************************************************************************************************
