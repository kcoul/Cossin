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
    @file   CossinMain.h
    @date   20, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"

#if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
extern juce::AudioProcessor *JUCE_CALLTYPE createPluginFilterOfType(juce::AudioProcessor::WrapperType type);
#else
extern juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter();
#endif

namespace jaut
{
    class Localisation;
    class ThemeManager;
    class AppData;
    class Config;
}

class SharedData;

struct PluginSession final
{
    const Time startTime;
    const Uuid id;
};

class CossinPluginWrapper final : private AudioIODeviceCallback, private Value::Listener, private Timer
{
public:
    struct PluginInOuts
    {
        int numIns;
        int numOuts;
    };

    //==================================================================================================================
    CossinPluginWrapper();
    virtual ~CossinPluginWrapper() override;

    //==================================================================================================================
    void init(bool, const String&);

    //==================================================================================================================
    virtual void createPlugin(bool = true);
    virtual void deletePlugin();
    static String getFilePatterns(const String&);

    //==================================================================================================================
    Value &getMuteInputValue();
    bool getProcessorHasPotentialFeedbackLoop() const;
    void valueChanged(Value&) override;

    //==================================================================================================================
    File getLastFile() const;
    void setLastFile(const File&);
    bool askUserToSaveState(const String& = String());
    bool askUserToLoadState(const String& = String());

    //==================================================================================================================
    void startPlaying();
    void stopPlaying();

    //==================================================================================================================
    bool saveAudioDeviceState();
    bool reloadAudioDeviceState(bool, const String&, const AudioDeviceManager::AudioDeviceSetup*);
    bool savePluginCache();
    bool reloadPluginCache();
    bool savePluginState(bool askToSave   = false, bool askIfNotSuccessful = false, bool notifyOnFail = false);
    bool reloadPluginState(bool askToLoad = false, bool askIfNotSuccessful = false, bool notifyOnFail = false);

    //==================================================================================================================
    void switchToHostApplication();
    bool isInterAppAudioConnected();

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
    Image getIAAHostIcon(int);
#endif

    static CossinPluginWrapper *getInstance();

    //==================================================================================================================
    SharedResourcePointer<SharedData> sharedData;
    std::unique_ptr<InterProcessLock> cacheLock;
    std::unique_ptr<PropertiesFile> cossinCache;
    std::unique_ptr<AudioProcessor> processor;
    AudioDeviceManager deviceManager;
    AudioProcessorPlayer player;
    Array<PluginInOuts> channelConfiguration;
    bool processorHasPotentialFeedbackLoop = true;
    std::atomic<bool> muteInput { true };
    Value shouldMuteInput;
    AudioBuffer<float> emptyBuffer;
    bool autoOpenMidiDevices;
    std::unique_ptr<AudioDeviceManager::AudioDeviceSetup> options;
    Array<MidiDeviceInfo> lastMidiDevices;
    File currentSaveFile;
    String lastLoadedState;

private:
    class SettingsComponent : public Component
    {
    public:
        SettingsComponent (CossinPluginWrapper&, AudioDeviceManager&, int, int, int, int);

        //==============================================================================================================
        void paint (Graphics&) override;
        void resized() override;

    private:
        CossinPluginWrapper& owner;
        AudioDeviceSelectorComponent deviceSelector;
        Label shouldMuteLabel;
        ToggleButton shouldMuteButton;

        //==============================================================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
    };

    //==================================================================================================================
    void audioDeviceIOCallback(const float**, int, float**, int, int) override;
    void audioDeviceAboutToStart(AudioIODevice*) override;
    void audioDeviceStopped() override;

    //==================================================================================================================
    void setupAudioDevices(bool, const String&, const AudioDeviceManager::AudioDeviceSetup*);
    void shutDownAudioDevices();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossinPluginWrapper)
};

class CossinPluginWindow final : public DocumentWindow
{
public:
    typedef CossinPluginWrapper::PluginInOuts PluginInOuts;

    //==================================================================================================================
    std::unique_ptr<CossinPluginWrapper> pluginHolder;

    //==================================================================================================================
    CossinPluginWindow();
    ~CossinPluginWindow() override;

    //==================================================================================================================
    AudioProcessor *getAudioProcessor() const noexcept;
    AudioDeviceManager &getDeviceManager() const noexcept;
    void resetToDefaultState();

    //==================================================================================================================
    void closeButtonPressed() override;
    virtual CossinPluginWrapper *getPluginHolder();

private:
    class MainContentComponent : public Component, private ComponentListener
    {
    public:
        MainContentComponent(CossinPluginWindow&);
        ~MainContentComponent() override;

        //==============================================================================================================
        void resized() override;

    private:
        //==============================================================================================================
        void componentMovedOrResized (Component&, bool, bool) override;
        Rectangle<int> getSizeToContainEditor() const;

        //==============================================================================================================
        CossinPluginWindow &owner;
        std::unique_ptr<AudioProcessorEditor> editor;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CossinPluginWindow)
};

class Cossin final : public JUCEApplication
{
public:
    static Cossin *JUCE_CALLTYPE getInstance() noexcept;

    //==================================================================================================================
    Cossin();
    ~Cossin();

    //==================================================================================================================
    const String getApplicationName() override { return JucePlugin_Name; }
    const String getApplicationVersion() override { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override { return true; }
    void anotherInstanceStarted(const String&) override {}

    //==================================================================================================================
    const PluginSession &getSession() const noexcept;

    //==================================================================================================================
    const bool isInputMuted() const noexcept;

    //==================================================================================================================
    void initialise(const String&) override;
    void shutdown() override;

    //==================================================================================================================
    void systemRequestedQuit() override;

    //==================================================================================================================
    CossinPluginWindow *createWindow();

private:
    PluginSession session;
    std::unique_ptr<Logger> logger;
    std::unique_ptr<CossinPluginWindow> mainWindow;
};

inline Logger *createDummyLogger()
{
    struct DummyLogger final : public Logger
    {
        DummyLogger() = default;
        void logMessage(const String&) override {}
    };
    
    return new DummyLogger;
}

inline FileLogger *createLoggerFromSession(const File &logFile, const PluginSession &session, const String &appName,
                                           const String &appVersion)
{
    const String sessionid = session.id.toDashedString();
    const String cpumodel  = !SystemStats::getCpuModel().isEmpty()  ? SystemStats::getCpuModel()  : "n/a";
    const String cpuvendor = !SystemStats::getCpuVendor().isEmpty() ? " (" + SystemStats::getCpuVendor() + ")" : "";
    const String gpumodel  = String((char*)glGetString(GL_VENDOR)) + " " + String((char*)glGetString(GL_RENDERER));
    const String memorysz  = String(SystemStats::getMemorySizeInMegabytes()) + "mb";

    String logmsg;

    logmsg << "                        --Program--                       " << newLine
           << "App:        " << appName                                    << newLine
           << "Version:    " << appVersion                                 << newLine
           << "Session-ID: " << sessionid                                  << newLine
           << "**********************************************************" << newLine
           << "                        --Machine--                       " << newLine
           << "System:     " << SystemStats::getOperatingSystemName()      << newLine
           << "Memory:     " << memorysz                                   << newLine
           << "CPU:        " << cpumodel << cpuvendor                      << newLine
           << "Graphics:   " << (!gpumodel.isEmpty() ? gpumodel : "n/a")   << newLine
           << "**********************************************************";

    return new FileLogger(logFile, logmsg);
}
