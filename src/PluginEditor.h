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
#include "ProcessorPanel.h"
#include "ReloadListener.h"
#include <jaut/localisation.h>
#include <jaut/propertyattribute.h>
#include <jaut/thememanager.h>

inline constexpr int Flag_AnimationEffects     = 0;
inline constexpr int Flag_AnimationComponents  = 1;
inline constexpr int Flag_HardwareAcceleration = 2;
inline constexpr int Flag_DefaultPanning       = 3;

namespace jaut
{
    class Config;
    class Localisation;
}

class CossinAudioProcessor;
class ProcessorContainer;

#if JUCE_OPENGL
  #define COSSIN_USE_OPENGL 1
#endif

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

    using ButtonAttachment = AudioProcessorValueTreeState::ButtonAttachment;
    using SliderAttachment = AudioProcessorValueTreeState::SliderAttachment;

    CossinAudioProcessorEditor(CossinAudioProcessor&, juce::AudioProcessorValueTreeState&, jaut::PropertyMap&,
                               FFAU::LevelMeterSource&, ProcessorContainer&);
    ~CossinAudioProcessorEditor();

private:
    void initializeData(SharedData&);
    void initializeComponents();
    void initializeWindow();

public:
    void paint(Graphics&) override;
    void resized() override;

    //==================================================================================================================
    constexpr bool canSelfResize() noexcept;

private:
    void paintBasicInterface(Graphics&) const;

private:
    void reloadConfig(SharedData&);
    void reloadLocale(SharedData&);
    void reloadTheme(SharedData&);

public:
    bool getOption(int) const noexcept;

    //==================================================================================================================
    void addReloadListener(ReloadListener*);
    void removeReloadListener(ReloadListener*);

private:
    class BackgroundBlur : public Component
    {
    public:
        void paint(Graphics &g) override
        {
            g.fillAll(Colours::black);
        }
    };

    //==================================================================================================================
    // General data
    bool initialized;
    Logger                 &logger;
    CossinAudioProcessor   &processor;
    FFAU::LevelMeterSource &sourceMetre;
    ListenerList<ReloadListener> listeners;

    // Update data
    String lastLocale;
    String lastTheme;
    jaut::Localisation locale;
    std::atomic<bool> needsUpdate;
    Array<var> options;

    // Component data
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

    // Processor sync data
    jaut::PropertyAttribute atrPanningLaw;
    jaut::PropertyAttribute atrProcessor;
    std::unique_ptr<SliderAttachment> attLevel;
    std::unique_ptr<SliderAttachment> attMix;
    std::unique_ptr<SliderAttachment> attPanning;

#if COSSIN_USE_OPENGL
    OpenGLContext glContext;
#endif

    // Painting data
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
