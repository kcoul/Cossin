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

#include "CossinMain.h"

#include "SharedData.h"
#include "Resources.h"
#include <jaut/appdata.h>
#include <jaut/localisation.h>
#include <jaut/config.h>

namespace
{
    static constexpr char const *Id_Cache = "73783927-4465-43c1-8612-0f87be2b37ce";

    inline int yncAlert(const jaut::Localisation &locale, const String& title, const String& message, 
                        std::function<void(bool)> callback = 0,
                        AlertWindow::AlertIconType icon    = AlertWindow::WarningIcon) noexcept
    {
        const int result = AlertWindow::showYesNoCancelBox(icon, locale.translate(title), locale.translate(message),
                                                           locale.translate("alert.yes"), locale.translate("alert.no"),
                                                           locale.translate("alert.cancel"));

        if(callback)
        {
            callback(result == 1);
        }

        return result;
    }
}



/* ==================================================================================
 * ===================================== Cossin =====================================
 * ================================================================================== */
#if(1)
Cossin *JUCE_CALLTYPE Cossin::getInstance() noexcept
{
    return dynamic_cast<Cossin*>(JUCEApplicationBase::getInstance());
}

//======================================================================================================================
Cossin::Cossin()
{
    PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_Standalone;
    jaut::JAUT_DISABLE_THREAD_DIST_EXPLICIT(true);
}

Cossin::~Cossin()
{
    Logger::setCurrentLogger(nullptr);
}

//======================================================================================================================
const String Cossin::getApplicationName()
{
    return res::App_Name;
}

const String Cossin::getApplicationVersion()
{
    return res::App_Version;
}

//==================================================================================================================
const bool Cossin::isInputMuted() const noexcept
{
    return static_cast<bool>(mainWindow->pluginHolder->getMuteInputValue().getValue());
}

//======================================================================================================================
void Cossin::initialise(const String&)
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

    if (ModalComponentManager::getInstance()->cancelAllModalComponents())
    {
        Timer::callAfterDelay(100, []()
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
CossinPluginWindow *Cossin::createWindow()
{
    return new CossinPluginWindow();
}
#endif



/* ==================================================================================
 * =============================== CossinPluginWrapper ==============================
 * ================================================================================== */
#if(1)
CossinPluginWrapper::CossinPluginWrapper()
    : cacheLock(std::make_unique<InterProcessLock>("COSSIN_" + String(Id_Cache))),
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

    const String prefdefdevice    = "";
    const bool audioInputRequired = inChannels > 0;

    if (audioInputRequired && RuntimePermissions::isRequired(RuntimePermissions::recordAudio)
        && !RuntimePermissions::isGranted(RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request(RuntimePermissions::recordAudio, [this, prefdefdevice](bool granted)
        {
            init(granted, prefdefdevice);
        });
    }
    else
    {
        init(audioInputRequired, prefdefdevice);
    }
}

void CossinPluginWrapper::init(bool enableAudioInput, const String &preferredDefaultDeviceName)
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
    processor.reset(::createPluginFilterOfType (AudioProcessor::wrapperType_Standalone));
#else
    AudioProcessor::setTypeOfNextNewPlugin(AudioProcessor::wrapperType_Standalone);
    processor.reset(createPluginFilter());
    AudioProcessor::setTypeOfNextNewPlugin(AudioProcessor::wrapperType_Undefined);
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
        MemoryBlock data;
        processor->getStateInformation(data);
        lastLoadedState = data.toBase64Encoding();
    }
}

void CossinPluginWrapper::deletePlugin()
{
    stopPlaying();
    processor = nullptr;
}

String CossinPluginWrapper::getFilePatterns(const String &fileSuffix)
{
    if (fileSuffix.isEmpty())
    {
        return {};
    }

    return (fileSuffix.startsWithChar('.') ? "*" : "*.") + fileSuffix;
}

//======================================================================================================================
Value &CossinPluginWrapper::getMuteInputValue()
{
    return shouldMuteInput;

}
bool CossinPluginWrapper::getProcessorHasPotentialFeedbackLoop() const
{
    return processorHasPotentialFeedbackLoop;
}

void CossinPluginWrapper::valueChanged(Value &value)
{
    muteInput = static_cast<bool>(value.getValue());
}

//======================================================================================================================
File CossinPluginWrapper::getLastFile() const
{
    const String defaultloc = sharedData->AppData().getDir("Data").getDir("Saves").toString(true);
    return File(cossinCache->getValue("lastStateFile", defaultloc));
}

void CossinPluginWrapper::setLastFile(const File &file)
{
    cossinCache->setValue("lastStateFile", file.getFullPathName());
    savePluginCache();
}

bool CossinPluginWrapper::askUserToSaveState(const String &fileSuffix)
{
#if JUCE_MODAL_LOOPS_PERMITTED
    const jaut::Localisation &locale = sharedData->Localisation();
    FileChooser fc(locale.translate("state.chooser.save.title"), getLastFile(), getFilePatterns(fileSuffix));

    if (fc.browseForFileToSave(true))
    {
        MemoryBlock data;
        const File file(fc.getResult());
        processor->getStateInformation(data);

        if (file.create().wasOk() && file.replaceWithText(data.toBase64Encoding()))
        {
            setLastFile(file);
            return true;
        }
        else
        {
            const int result = ::yncAlert(locale, "state.save.err.title", "state.save.err.text", nullptr,
                                          AlertWindow::QuestionIcon);

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

bool CossinPluginWrapper::askUserToLoadState(const String &fileSuffix)
{
#if JUCE_MODAL_LOOPS_PERMITTED
    const jaut::Localisation &locale = sharedData->Localisation();
    FileChooser fc(locale.translate("state.chooser.load.title"), getLastFile(), getFilePatterns(fileSuffix));

    if (fc.browseForFileToOpen())
    {
        MemoryBlock data;
        const File file(fc.getResult());
        const String base64 = file.loadFileAsString();

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
            const int result = AlertWindow::showOkCancelBox(AlertWindow::WarningIcon,
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
    if (auto device = dynamic_cast<iOSAudioIODevice*> (deviceManager.getCurrentAudioDevice()))
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
bool CossinPluginWrapper::saveAudioDeviceState()
{
    return true;
}

bool CossinPluginWrapper::reloadAudioDeviceState(bool enableAudioInput, const String &preferredDefaultDeviceName,
                                                 const AudioDeviceManager::AudioDeviceSetup *preferredSetupOptions)
{
    std::unique_ptr<XmlElement> savedState(std::make_unique<XmlElement>("DEVICESETUP"));
    auto &config = sharedData->Configuration();
    auto propdev = config.getProperty("devices", res::Cfg_Standalone);

    const String outputdev  = propdev.getProperty("output").getValue();
    const String inputdev   = propdev.getProperty("input") .getValue();
    const String devtype    = config.getProperty("deviceType", res::Cfg_Standalone).getValue();
    const double samplerate = config.getProperty("sampleRate", res::Cfg_Standalone).getValue();
    const int buffersize    = config.getProperty("bufferSize", res::Cfg_Standalone).getValue();

    savedState->setAttribute("audioOutputDeviceName", outputdev.isEmpty() ? "default" : outputdev);
    savedState->setAttribute("audioInputDeviceName",  inputdev.isEmpty()  ? "default" : inputdev);
    savedState->setAttribute("deviceType",            devtype.isEmpty()   ? "default" : devtype);
    savedState->setAttribute("audioDeviceRate",       samplerate);
    savedState->setAttribute("audioDeviceBufferSize", buffersize);

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

    const String result = deviceManager.initialise(enableAudioInput ? totalInChannels : 0, totalOutChannels,
                                                   savedState.get(), true, preferredDefaultDeviceName,
                                                   preferredSetupOptions);

    if(!result.isEmpty())
    {
        if(Cossin *cossin = Cossin::getInstance())
        {
            Logger::getCurrentLogger()->writeToLog("An exception occurred while trying to initialize the main"
                                                   "audio device:\n" + result);
        }
        
        return false;
    }

    return true;
}

bool CossinPluginWrapper::savePluginCache()
{
    if(cossinCache)
    {
        return cossinCache->save();
    }

    return false;
}

bool CossinPluginWrapper::reloadPluginCache()
{
    if(!cossinCache)
    {
        try
        {
            PropertiesFile::Options opts;
            opts.storageFormat = PropertiesFile::storeAsCompressedBinary;
            opts.processLock   = cacheLock.get();
            cossinCache.reset(new PropertiesFile(sharedData->AppData().getFile(".cache"), opts));
            
            return true;
        }
        catch(std::bad_alloc&){}
    }
    else
    {
        return cossinCache->reload();
    }

    return false;
}

bool CossinPluginWrapper::savePluginState(bool askToSave, bool askIfNotSuccessful, bool notifyOnFail)
{
    if(!processor)
    {
        return false;
    }

    MemoryBlock data;
    processor->getStateInformation(data);
    const String base64 = data.toBase64Encoding();
    const auto &locale  = sharedData->Localisation();

    if(base64 == lastLoadedState)
    {
        return true;
    }

    if(askToSave)
    {
        const int result = ::yncAlert(locale, "state.save.new.title", "state.save.new.text", nullptr,
                                      AlertWindow::QuestionIcon);

        if(result != 1)
        {
            return result;
        }
    }

    if(!currentSaveFile.isDirectory() && currentSaveFile.replaceWithText(base64))
    {
        if(cossinCache)
        {
            cossinCache->setValue("lastStateFile", currentSaveFile.getFullPathName());
        }
    }
    else if(askIfNotSuccessful || askToSave)
    {
        if(!askToSave)
        {
            const int result = ::yncAlert(locale, "state.save.err.title", "state.save.err.text", nullptr,
                                      AlertWindow::WarningIcon);

            if(result == 1)
            {
                return askUserToSaveState(".dat");
            }

            return result;
        }

        return askUserToSaveState(".dat");
    }
    else if(notifyOnFail)
    {
        AlertWindow::showMessageBox(AlertWindow::NoIcon, locale.translate("state.save.fail.title"),
                                    locale.translate("state.save.fail.text"), locale.translate("alert.ok"));

        return false;
    }

    return true;
}

bool CossinPluginWrapper::reloadPluginState(bool askToLoad, bool askIfNotSuccessful, bool notifyOnFail)
{
    if(!processor)
    {
        return false;
    }

    const auto &locale  = sharedData->Localisation();
    const String base64 = currentSaveFile.loadFileAsString();
    MemoryBlock data;

    if(askToLoad)
    {
        const int result = ::yncAlert(locale, "state.load.file.title", "state.load.file.text", nullptr,
                                      AlertWindow::QuestionIcon);

        if(result != 1)
        {
            return result;
        }
    }

    if(currentSaveFile.exists() && !currentSaveFile.isDirectory() && data.fromBase64Encoding(base64)
       && data.getSize() > 0)
    {
        processor->setStateInformation(data.getData(), static_cast<int>(data.getSize()));
        lastLoadedState = base64;
    }
    else if(askIfNotSuccessful || askToLoad)
    {
        if(!askToLoad)
        {
            const int result = ::yncAlert(locale, "state.load.err.title", "state.load.err.text", nullptr,
                                      AlertWindow::WarningIcon);

            if(result == 1)
            {
                return askUserToLoadState(".dat");
            }
            
            return result;
        }
        
        return askUserToLoadState(".dat");
    }
    else if(notifyOnFail)
    {
        AlertWindow::showMessageBox(AlertWindow::NoIcon, locale.translate("state.load.fail.title"),
                                    locale.translate("state.load.fail.text"), locale.translate("alert.ok"));

        return false;
    }

    return true;
}

//======================================================================================================================
void CossinPluginWrapper::switchToHostApplication()
{
#if JUCE_IOS
    if (auto device = dynamic_cast<iOSAudioIODevice*>(deviceManager.getCurrentAudioDevice()))
    {
        device->switchApplication();
    }
#endif
}

bool CossinPluginWrapper::isInterAppAudioConnected()
{
#if JUCE_IOS
    if (auto device = dynamic_cast<iOSAudioIODevice*>(deviceManager.getCurrentAudioDevice()))
    {
        return device->isInterAppAudioConnected();
    }
#endif

    return false;
}

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
Image CossinPluginWrapper::getIAAHostIcon(int size)
{
#if JUCE_IOS && JucePlugin_Enable_IAA
    if (auto device = dynamic_cast<iOSAudioIODevice*>(deviceManager.getCurrentAudioDevice()))
    {
        return device->getIcon(size);
    }
#else
    ignoreUnused (size);
#endif

    return {};
}
#endif

//======================================================================================================================
CossinPluginWrapper *CossinPluginWrapper::getInstance()
{
#if JucePlugin_Enable_IAA || JucePlugin_Build_Standalone
    if (PluginHostType::getPluginLoadedAs() == AudioProcessor::wrapperType_Standalone)
    {
        Desktop &desktop             = Desktop::getInstance();
        const int numTopLevelWindows = desktop.getNumComponents();

        for (int i = 0; i < numTopLevelWindows; ++i)
        {
            if (CossinPluginWindow *window = dynamic_cast<CossinPluginWindow*>(desktop.getComponent(i)))
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

void CossinPluginWrapper::audioDeviceAboutToStart(AudioIODevice *device)
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
void CossinPluginWrapper::setupAudioDevices(bool enableAudioInput, const String &preferredDefaultDeviceName,
                                            const AudioDeviceManager::AudioDeviceSetup *preferredSetupOptions)
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
    auto newMidiDevices = MidiInput::getAvailableDevices();

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
#endif



/* ==================================================================================
 * =============================== CossinPluginWindow ===============================
 * ================================================================================== */
#if(1)
CossinPluginWindow::CossinPluginWindow()
    : DocumentWindow("Cossin", LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId), 7)
{
#if JUCE_IOS || JUCE_ANDROID
    setTitleBarHeight(0);
#else
    setTitleBarButtonsRequired(7, false);
    setUsingNativeTitleBar(true);
#endif

    pluginHolder.reset(new CossinPluginWrapper());

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen(true);
    setContentOwned(new MainContentComponent(*this), false);
#else
    setContentOwned(new MainContentComponent(*this), true);
    
    auto shared_data (SharedData::getInstance());

    if (auto* cache = pluginHolder->cossinCache.get())
    {
        const auto propsize = shared_data->Configuration().getProperty("size", res::Cfg_Defaults);
        const int x         = cache->getIntValue("windowX", -100);
        const int y         = cache->getIntValue("windowY", -100);
        const int width     = cache->getIntValue("windowW", propsize.getProperty("width").getValue());
        const int height    = cache->getIntValue("windowH", propsize.getProperty("height").getValue());

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
AudioProcessor *CossinPluginWindow::getAudioProcessor() const noexcept
{
    return pluginHolder->processor.get();
}

AudioDeviceManager &CossinPluginWindow::getDeviceManager() const noexcept
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
        JUCEApplicationBase::quit();
    }
}

CossinPluginWrapper *CossinPluginWindow::getPluginHolder()
{
    return pluginHolder.get();
}
#endif



/* ==================================================================================
 * ============================== MainContentComponent ==============================
 * ================================================================================== */
#if(1)
CossinPluginWindow::MainContentComponent::MainContentComponent(CossinPluginWindow &filterWindow)
    : owner(filterWindow),
      editor(owner.getAudioProcessor()->hasEditor() ? owner.getAudioProcessor()->createEditorIfNeeded()
                                                    : new GenericAudioProcessorEditor(*owner.getAudioProcessor()))
{
    Value &inputMutedValue = owner.pluginHolder->getMuteInputValue();

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

Rectangle<int> CossinPluginWindow::MainContentComponent::getSizeToContainEditor() const
{
    if (editor != nullptr)
    {
        return getLocalArea(editor.get(), editor->getLocalBounds());
    }
        
    return {};
}
#endif

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

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
Image JUCE_CALLTYPE juce_getIAAHostIcon(int size)
{
    if (auto holder = CossinPluginWrapper::getInstance())
    {
        return holder->getIAAHostIcon(size);
    }

    return Image();
}
#endif
#endif

#if JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
JUCE_CREATE_APPLICATION_DEFINE(Cossin)
#endif
