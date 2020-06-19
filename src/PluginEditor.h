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

#include <ff_meters/ff_meters.h>
#include <jaut_audio/jaut_audio.h>
#include <jaut_provider/jaut_provider.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_opengl/juce_opengl.h>

#include "CossinDef.h"
#include "PluginStyle.h"
#include "OptionPanel.h"
#include "ReloadListener.h"
#include "AttachmentList.h"

#include <bitset>

class CossinMainEditorWindow;
class SharedData;

//======================================================================================================================
// CONSTANTS
inline constexpr int Flag_AnimationEffects     = 0;
inline constexpr int Flag_AnimationComponents  = 1;
#if COSSIN_USE_OPENGL
inline constexpr int Flag_HardwareAcceleration = 2;
inline constexpr int Flag_GlMultisampling      = 3;
inline constexpr int Flag_GlTextureSmoothing   = 4;
inline constexpr int Flag_End                  = 4;
#else
inline constexpr int Flag_End                  = 1;
#endif
inline constexpr int Flag_Num                  = Flag_End + 1;

inline constexpr int Const_WindowDefaultWidth  = 800;
inline constexpr int Const_WindowDefaultHeight = 500;

//======================================================================================================================
// FORWARDING
class CossinAudioProcessor;

//======================================================================================================================
// CLASSES
struct PluginSession final
{
    const juce::Time startTime;
    const juce::Uuid id;

    PluginSession() noexcept
        : startTime(juce::Time::getCurrentTime())
    {}
};

class CossinAudioProcessorEditor final : public juce::Component, public juce::ActionListener,
                                         private juce::Button::Listener, private juce::Slider::Listener,
                                         private juce::LookAndFeel_V4, private juce::Timer
#if COSSIN_USE_OPENGL
                                   , public juce::OpenGLRenderer
#endif
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
    
    //==================================================================================================================
    using AttachmentTypes = AttachmentList<
        AttachmentEntry<juce::Button,   juce::ButtonParameterAttachment>,
        AttachmentEntry<juce::Slider,   juce::SliderParameterAttachment>,
        AttachmentEntry<juce::ComboBox, juce::ComboBoxParameterAttachment>,
        AttachmentEntry<juce::Value,    jaut::ValueParameterAttachment>
        // FUTURE mappings
    >;
    
    using VariantType      = AttachmentTypes::UnpackAttachments<std::variant>;
    using AttachmentVector = std::vector<VariantType>;
    
    //==================================================================================================================
    CossinAudioProcessorEditor(CossinAudioProcessor&, juce::AudioProcessorValueTreeState&, foleys::LevelMeterSource&,
                               CossinMainEditorWindow&, bool, const juce::String&);
    ~CossinAudioProcessorEditor() override;

private:
    void initializeData(CossinMainEditorWindow&, juce::String);
    void initializeComponents();

public:
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void paintBasicInterface(juce::Graphics&) const;

public:
    void reloadConfig(const jaut::Config&);
    void reloadLocale(const jaut::Localisation&);
    void reloadTheme (const jaut::ThemePointer&);

    //==================================================================================================================
    bool getOption(int) const noexcept;
    void setOption(int, bool) noexcept;
    const PluginSession &getSession() const noexcept;
#if COSSIN_USE_OPENGL
    bool isOpenGLSupported() const noexcept;
#endif

    //==================================================================================================================
    void addReloadListener(ReloadListener*);
    void removeReloadListener(ReloadListener*);

private:
    class BackgroundBlur : public Component
    {
    public:
        void paint(juce::Graphics &g) override
        {
            g.fillAll(juce::Colours::black);
        }
    };

    //==================================================================================================================
    friend class CossinMainEditorWindow;
    
    //==================================================================================================================
    
    //==================================================================================================================
    // General
    PluginSession session;
    juce::SharedResourcePointer<SharedData> sharedData;
    CossinAudioProcessor   &processor;
    foleys::LevelMeterSource &sourceMetre;
    bool initialized;
    juce::ListenerList<ReloadListener> listeners;
    PluginStyle lookAndFeel;
    juce::TooltipWindow tooltipServer;

#if COSSIN_USE_OPENGL
    std::unique_ptr<juce::OpenGLContext> glContext;
#endif

    // Shared data
    juce::String lastLocale;
    juce::String lastTheme;
    jaut::Localisation locale;
    std::atomic<bool> needsUpdate;
    std::bitset<Flag_Num> options;

    // Components
    BackgroundBlur backgroundBlur;
    juce::ToggleButton   buttonPanningLaw;
    juce::DrawableButton buttonPanningLawSelection;
    juce::DrawableButton buttonSettings;
    foleys::LevelMeter metreLevel;
    OptionPanel optionsPanel;
    juce::Slider sliderLevel;
    juce::Slider sliderMix;
    juce::Slider sliderPanning;
    juce::Slider sliderTabControl;
    
    // Processor data
    AttachmentVector parameterAttachments;
    juce::Value valuePanningMode;
    juce::Value valueProcessMode;
    
    // Drawing
    juce::Image imgBackground;
    juce::Image imgHeader;
    juce::Image imgLogo;
    juce::Image imgPanningLaw;
    juce::Image imgTabControl;
    juce::Image imgTabSettings;
    juce::Font  fontTheme;

    //==================================================================================================================
    void mouseDown(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void buttonClicked(juce::Button*) override;
    void sliderValueChanged(juce::Slider*) override;
    void sliderDragEnded(juce::Slider*) override;

    //==================================================================================================================
    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawDrawableButton(juce::Graphics&, juce::DrawableButton&, bool, bool) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override;

    //==================================================================================================================
    void timerCallback() override;
    void actionListenerCallback(const juce::String&) override;
    void reloadAllData();
    std::vector<VariantType> createAttachments(juce::AudioProcessorValueTreeState&);
    
#if COSSIN_USE_OPENGL
    //==================================================================================================================
    void newOpenGLContextCreated() override;
    void renderOpenGL() override;
    void openGLContextClosing() override;
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CossinAudioProcessorEditor)
};

class CossinMainEditorWindow final : public juce::AudioProcessorEditor, private juce::AsyncUpdater
#if COSSIN_USE_OPENGL
                                     , private juce::OpenGLRenderer
#endif
{
public:
    CossinMainEditorWindow(CossinAudioProcessor&, juce::AudioProcessorValueTreeState&, foleys::LevelMeterSource&);
    ~CossinMainEditorWindow() override;
    
    //==================================================================================================================
    void resized() override;
    
private:
    std::unique_ptr<CossinAudioProcessorEditor> editor;
    CossinAudioProcessor &processor;
    juce::AudioProcessorValueTreeState &vts;
    foleys::LevelMeterSource &metreSource;

#if COSSIN_USE_OPENGL
    juce::MessageManager::Lock messageManagerLock;
    juce::OpenGLContext testContext;
    bool initialized { false };
    std::atomic<bool> isSupported { false };
    juce::String graphicsCardDetails;
#endif

    //==================================================================================================================
    void initializeWindow();

    //==================================================================================================================
#if COSSIN_USE_OPENGL
    void newOpenGLContextCreated() override;
    void renderOpenGL() override {}
    void openGLContextClosing() override {}
#endif

    //==================================================================================================================
    void handleAsyncUpdate() override;
};

//======================================================================================================================
// Functions
inline PluginStyle &getPluginStyle(const juce::Component &component)
{
    return *dynamic_cast<PluginStyle*>(&component.getLookAndFeel());
}
