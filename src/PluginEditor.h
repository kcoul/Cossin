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
#include "ReloadListener.h"
#include "Resources.h"

#include <jaut/localisation.h>
#include <jaut/propertyattribute.h>

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
// FORWARD DECLERATIONS
namespace jaut
{
class AudioProcessorRack;
}

class CossinAudioProcessor;
class CossinMainEditorWindow;
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

class CossinAudioProcessorEditor : public Component, public ActionListener, private Button::Listener,
                                   private Slider::Listener, private LookAndFeel_V4,
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
                               FFAU::LevelMeterSource&, jaut::AudioProcessorRack&, CossinMainEditorWindow&, bool,
                               const String&);
    ~CossinAudioProcessorEditor();

private:
    void initializeData(CossinMainEditorWindow&,String);
    void initializeComponents();

public:
    void paint(Graphics&) override;
    void resized() override;

private:
    void paintBasicInterface(Graphics&) const;

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
    using ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;
    using SliderAttachment = AudioProcessorValueTreeState::SliderAttachment;
    using SliderAttachmentPtr = std::unique_ptr<SliderAttachment>;
    using ButtonAttachmentPtr = std::unique_ptr<ButtonAttachment>;

    //==================================================================================================================
    class BackgroundBlur : public Component
    {
    public:
        void paint(Graphics &g) override
        {
            g.fillAll(Colours::black);
        }
    };

    //==================================================================================================================
    friend class CossinMainEditorWindow;

    //==================================================================================================================
    // General
    PluginSession session;
    SharedResourcePointer<SharedData> sharedData;
    CossinAudioProcessor   &processor;
    FFAU::LevelMeterSource &sourceMetre;
    bool initialized;
    ListenerList<ReloadListener> listeners;
    PluginStyle lookAndFeel;
    TooltipWindow tooltipServer;

#if COSSIN_USE_OPENGL
    std::unique_ptr<OpenGLContext> glContext;
#endif

    // Shared data
    String lastLocale;
    String lastTheme;
    jaut::Localisation locale;
    std::atomic<bool> needsUpdate;
    bool options[Flag_Num] = { false };

    // Components
    BackgroundBlur backgroundBlur;
    ToggleButton   buttonPanningLaw;
    DrawableButton buttonPanningLawSelection;
    DrawableButton buttonSettings;
    FFAU::LevelMeter metreLevel;
    OptionPanel optionsPanel;
    Slider sliderLevel;
    Slider sliderMix;
    Slider sliderPanning;
    Slider sliderTabControl;

    // Processor data
    jaut::PropertyAttribute atrPanningLaw;
    jaut::PropertyAttribute atrProcessor;
    SliderAttachmentPtr attLevel;
    SliderAttachmentPtr attMix;
    SliderAttachmentPtr attPanning;

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

class CossinMainEditorWindow final : public AudioProcessorEditor, private AsyncUpdater
#if COSSIN_USE_OPENGL
                                     , private OpenGLRenderer
#endif
{
public:
    CossinMainEditorWindow(CossinAudioProcessor&, juce::AudioProcessorValueTreeState&, jaut::PropertyMap&,
                           FFAU::LevelMeterSource&, jaut::AudioProcessorRack&);
    ~CossinMainEditorWindow();
    
    //==================================================================================================================
    void resized() override;
    
private:
    std::unique_ptr<CossinAudioProcessorEditor> editor;
    CossinAudioProcessor &processor;
    AudioProcessorValueTreeState &vts;
    jaut::PropertyMap &properties;
    FFAU::LevelMeterSource &metreSource;
    jaut::AudioProcessorRack &processorRack;

#if COSSIN_USE_OPENGL
    MessageManager::Lock messageManagerLock;
    OpenGLContext testContext;
    bool initialized { false };
    std::atomic<bool> isSupported;
    String graphicsCardDetails;
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
inline PluginStyle &getPluginStyle(const Component &component)
{
    return *dynamic_cast<PluginStyle*>(&component.getLookAndFeel());
}
