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
    @file   PluginEditor.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"
#include "OptionPanel.h"
#include "PluginStyle.h"
#include "ProcessorPanel.h"
#include "ReloadListener.h"
#include <jaut/localisation.h>
#include <jaut/propertyattribute.h>

//======================================================================================================================
// MACROS
#if JUCE_OPENGL
  #define COSSIN_USE_OPENGL 1
#endif

//======================================================================================================================
// CONSTANTS
inline constexpr int Flag_AnimationEffects     = 0;
inline constexpr int Flag_AnimationComponents  = 1;
inline constexpr int Flag_HardwareAcceleration = 2;
inline constexpr int Flag_DefaultPanning       = 3;
inline constexpr int Flag_DefaultProcessor     = 4;
inline constexpr int Flag_End                  = 7;

//======================================================================================================================
// FORWARD DECLERATIONS
class CossinAudioProcessor;
class ProcessorContainer;
class SharedData;

//======================================================================================================================
// CLASSES
struct PluginSession final
{
    const Time startTime;
    const Uuid id;

    PluginSession() noexcept
        : startTime(Time::getCurrentTime())
    {}
};

class CossinAudioProcessorEditor : public AudioProcessorEditor, public ActionListener, Button::Listener,
                                   Slider::Listener, LookAndFeel_V4,
#if COSSIN_USE_OPENGL
                                   public OpenGLRenderer,
#endif
                                   Timer
{
public:
    enum ColourIds
    {
        ColourComponentBackgroundId = 0x2000100,
        ColourComponentForegroundId = 0x2000101,
        ColourContainerBackgroundId = 0x2000102,
        ColourContainerForegroundId = 0x2000103,
        ColourHeaderBackgroundId    = 0x2000104,
        ColourTooltipBackgroundId   = 0x2000105,
        ColourTooltipBorderId       = 0x2000106,
        ColourTooltipFontId         = 0x2000107,
        ColourFontId                = 0x2000108
    };

    CossinAudioProcessorEditor(CossinAudioProcessor&, juce::AudioProcessorValueTreeState&, jaut::PropertyMap&,
                               FFAU::LevelMeterSource&, ProcessorContainer&);
    ~CossinAudioProcessorEditor();

private:
    void initializeData();
    void initializeComponents();
    void initializeWindow();

public:
    void paint(Graphics&) override;
    void resized() override;

    //==================================================================================================================
    constexpr bool canSelfResize() noexcept;

private:
    void paintBasicInterface(Graphics&) const;

public:
    void reloadConfig(const jaut::Config&);
    void reloadLocale(const jaut::Localisation&);
    void reloadTheme (const jaut::ThemePointer&);

    //==================================================================================================================
    bool getOption(int) const noexcept;
    const PluginSession &getSession() const noexcept;

    //==================================================================================================================
    void addReloadListener(ReloadListener*);
    void removeReloadListener(ReloadListener*);

private:
    using t_ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;
    using t_SliderAttachment = AudioProcessorValueTreeState::SliderAttachment;
    using p_SliderAttachment = std::unique_ptr<t_SliderAttachment>;
    using p_ButtonAttachment = std::unique_ptr<t_ButtonAttachment>;

    class BackgroundBlur : public Component
    {
    public:
        void paint(Graphics &g) override
        {
            g.fillAll(Colours::black);
        }
    };

    //==================================================================================================================
    // General
    PluginSession session;
    SharedResourcePointer<SharedData> sharedData;
    std::unique_ptr<Logger> logger;
    CossinAudioProcessor   &processor;
    FFAU::LevelMeterSource &sourceMetre;
    bool initialized;
    ListenerList<ReloadListener> listeners;
    PluginStyle lookAndFeel;

    // Shared data
    String lastLocale;
    String lastTheme;
    jaut::Localisation locale;
    std::atomic<bool> needsUpdate;
    bool options[Flag_End + 1];

    // Components
    BackgroundBlur backgroundBlur;
    ToggleButton   buttonPanningLaw;
    DrawableButton buttonPanningLawSelection;
    DrawableButton buttonSettings;
    FFAU::LevelMeter metreLevel;
    OptionPanel      optionsPanel;
    ProcessorPanel   processorPanel;
    Slider sliderLevel;
    Slider sliderMix;
    Slider sliderPanning;
    Slider sliderTabControl;

    // Processor data
    jaut::PropertyAttribute atrPanningLaw;
    jaut::PropertyAttribute atrProcessor;
    p_SliderAttachment attLevel;
    p_SliderAttachment attMix;
    p_SliderAttachment attPanning;

#if COSSIN_USE_OPENGL
    OpenGLContext glContext;
#endif

    // Drawing
    Image imgBackground;
    Image imgHeader;
    Image imgLogo;
    Image imgPanningLaw;
    Image imgTabControl;
    Image imgTabSettings;
    Font  fontTheme;

    //==================================================================================================================
    void mouseDown(const MouseEvent&) override;
    void mouseMove(const MouseEvent&) override;
    void buttonClicked(Button*) override;
    void sliderValueChanged(Slider*) override;
    void sliderDragEnded(Slider*) override;

    //==================================================================================================================
    void drawToggleButton(Graphics&, ToggleButton&, bool, bool) override;
    void drawDrawableButton(Graphics&, DrawableButton&, bool, bool) override;
    void drawLinearSlider(Graphics&, int, int, int, int, float, float, float, Slider::SliderStyle, Slider&) override;

    //==================================================================================================================
    void timerCallback() override;
    void actionListenerCallback(const String&) override;
    void reloadAllData();

#if COSSIN_USE_OPENGL
    //==================================================================================================================
    void newOpenGLContextCreated();
    void renderOpenGL();
    void openGLContextClosing();
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CossinAudioProcessorEditor)
};

//======================================================================================================================
// Functions
inline PluginStyle &getPluginStyle(Component &component)
{
    return *dynamic_cast<PluginStyle*>(&component.getLookAndFeel());
}

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
    const String session_id  = session.id.toDashedString();
    const String cpu_model   = !SystemStats::getCpuModel().isEmpty()  ? SystemStats::getCpuModel()  : "n/a";
    const String cpu_vendor  = !SystemStats::getCpuVendor().isEmpty() ? " (" + SystemStats::getCpuVendor() + ")" : "";
    const String gpu_model   = String((char*)(unsigned char*)glGetString(GL_VENDOR)) + " " + String((char*)(unsigned char*)glGetString(GL_RENDERER));
    const String memory_size = String(SystemStats::getMemorySizeInMegabytes()) + "mb";

    String logmsg;

    logmsg << "                        --Program--                       " << newLine
           << "App:        " << appName                                    << newLine
           << "Version:    " << appVersion                                 << newLine
           << "Session-ID: " << session_id                                 << newLine
           << "**********************************************************" << newLine
           << "                        --Machine--                       " << newLine
           << "System:     " << SystemStats::getOperatingSystemName()      << newLine
           << "Memory:     " << memory_size                                << newLine
           << "CPU:        " << cpu_model << cpu_vendor                    << newLine
           << "Graphics:   " << (gpu_model.containsNonWhitespaceChars()
                                 ? gpu_model : "Unknown")                  << newLine
           << "**********************************************************";

    return new FileLogger(logFile, logmsg);
}
