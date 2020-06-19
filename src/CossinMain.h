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
    @file   CossinMain.h
    @date   20, October 2019
    
    ===============================================================
 */

#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "PluginStyle.h"

#if JUCE_MAJOR_VERSION >= 6
#   include <juce_audio_plugin_client/utility/juce_CreatePluginFilter.h>
#else
#   if JUCE_MODULE_AVAILABLE_juce_audio_plugin_client
        extern juce::AudioProcessor *JUCE_CALLTYPE createPluginFilterOfType(juce::AudioProcessor::WrapperType type);
#   else
        extern juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter();
#   endif
#endif

class SharedData;

class CossinPluginWrapper final : private juce::AudioIODeviceCallback, private juce::Value::Listener,
                                  private juce::Timer
{
public:
    struct PluginInOuts
    {
        int numIns;
        int numOuts;
    };

    //==================================================================================================================
    CossinPluginWrapper();
    ~CossinPluginWrapper() override;

    //==================================================================================================================
    void init(bool, const juce::String&);

    //==================================================================================================================
    void createPlugin(bool = true);
    void deletePlugin();
    static juce::String getFilePatterns(const juce::String&);

    //==================================================================================================================
    juce::Value& getMuteInputValue();
    bool getProcessorHasPotentialFeedbackLoop() const;
    void valueChanged(juce::Value&) override;

    //==================================================================================================================
    juce::File getLastFile() const;
    void setLastFile(const juce::File&);
    bool askUserToSaveState(const juce::String& = juce::String());
    bool askUserToLoadState(const juce::String& = juce::String());

    //==================================================================================================================
    void startPlaying();
    void stopPlaying();

    //==================================================================================================================
    bool reloadAudioDeviceState(bool, const juce::String&, const juce::AudioDeviceManager::AudioDeviceSetup*);
    bool savePluginCache();
    bool reloadPluginCache();
    bool savePluginState(bool askToSave   = false, bool askIfNotSuccessful = false, bool notifyOnFail = false);
    bool reloadPluginState(bool askToLoad = false, bool askIfNotSuccessful = false, bool notifyOnFail = false);

    //==================================================================================================================
    void switchToHostApplication();
    bool isInterAppAudioConnected();

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
    juce::Image getIAAHostIcon(int);
#endif

    static CossinPluginWrapper *getInstance();

    //==================================================================================================================
    juce::AudioDeviceManager deviceManager;
    juce::AudioProcessorPlayer player;
    juce::AudioBuffer<float> emptyBuffer;
    juce::Value shouldMuteInput;
    juce::Array<PluginInOuts> channelConfiguration;
    juce::Array<juce::MidiDeviceInfo> lastMidiDevices;
    juce::SharedResourcePointer<SharedData> sharedData;
    std::unique_ptr<juce::InterProcessLock> cacheLock;
    std::unique_ptr<juce::PropertiesFile> cossinCache;
    std::unique_ptr<juce::AudioProcessor> processor;
    std::unique_ptr<juce::AudioDeviceManager::AudioDeviceSetup> options;
    juce::File currentSaveFile;
    juce::String lastLoadedState;
    std::atomic<bool> muteInput { true };
    bool autoOpenMidiDevices;
    bool processorHasPotentialFeedbackLoop { true };


private:
    void audioDeviceIOCallback(const float**, int, float**, int, int) override;
    void audioDeviceAboutToStart(juce::AudioIODevice*) override;
    void audioDeviceStopped() override;

    //==================================================================================================================
    void setupAudioDevices(bool, const juce::String&, const juce::AudioDeviceManager::AudioDeviceSetup*);
    void shutDownAudioDevices();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossinPluginWrapper)
};

class CossinPluginWindow final : public juce::DocumentWindow
{
public:
    typedef CossinPluginWrapper::PluginInOuts PluginInOuts;

    //==================================================================================================================
    std::unique_ptr<CossinPluginWrapper> pluginHolder;

    //==================================================================================================================
    CossinPluginWindow();
    ~CossinPluginWindow() override;

    //==================================================================================================================
    juce::AudioProcessor* getAudioProcessor() const noexcept;
    juce::AudioDeviceManager& getDeviceManager() const noexcept;
    void resetToDefaultState();

    //==================================================================================================================
    void closeButtonPressed() override;
    virtual CossinPluginWrapper* getPluginHolder();

private:
    class MainContentComponent final : public Component, private juce::ComponentListener
    {
    public:
        explicit MainContentComponent(CossinPluginWindow&);
        ~MainContentComponent() override;

        //==============================================================================================================
        void resized() override;

    private:
        //==============================================================================================================
        void componentMovedOrResized (Component&, bool, bool) override;
        juce::Rectangle<int> getSizeToContainEditor() const;

        //==============================================================================================================
        CossinPluginWindow &owner;
        std::unique_ptr<juce::AudioProcessorEditor> editor;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CossinPluginWindow)
};

class Cossin final : public juce::JUCEApplication
{
public:
    static Cossin *JUCE_CALLTYPE getInstance() noexcept;

    //==================================================================================================================
    Cossin();
    ~Cossin() override;

    //==================================================================================================================
    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    bool moreThanOneInstanceAllowed() override { return true; }
    void anotherInstanceStarted(const juce::String&) override {}

    //==================================================================================================================
    bool isInputMuted() const noexcept;

    //==================================================================================================================
    void initialise(const juce::String&) override;
    void shutdown() override;

    //==================================================================================================================
    void systemRequestedQuit() override;

    //==================================================================================================================
    static CossinPluginWindow* createWindow();

private:
    std::unique_ptr<CossinPluginWindow> mainWindow;
};
