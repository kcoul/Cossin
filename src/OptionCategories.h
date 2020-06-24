/**
    ===============================================================
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any internal version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.
    
    Copyright (c) 2019 ElandaSunshine
    ===============================================================
    
    @author Elanda
    @file   OptionCategories.h
    @date   03, November 2019
    
    ===============================================================
 */

#pragma once

#include <jaut_provider/jaut_provider.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_opengl/juce_opengl.h>

#include "ReloadListener.h"
#include "Resources.h"
#include "CossinDef.h"
#include "SCLabel.h"

class CossinAudioProcessorEditor;
class SharedData;
class OptionCategory : public juce::Component, public ReloadListener
{
public:
    explicit OptionCategory(CossinAudioProcessorEditor &editor) : editor(editor) {}

    //==================================================================================================================
    virtual bool saveState(SharedData&) const = 0;
    virtual void loadState(const SharedData&) = 0;

    //==================================================================================================================
    void reloadConfig(const jaut::Config&)       override {}
    void reloadTheme (const jaut::ThemePointer&) override {}
    void reloadLocale(const jaut::Localisation&) override {}

protected:
    CossinAudioProcessorEditor &editor;
};

/* Holds general and default settings */
class OptionPanelGeneral final : public OptionCategory, public juce::ListBoxModel
{
public:
    explicit OptionPanelGeneral(CossinAudioProcessorEditor&);

    //==================================================================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    
    //==================================================================================================================
    bool saveState(SharedData&) const override;
    void loadState(const SharedData&) override;

private:
    class PanelDefaults final : public Component, private juce::TextEditor::InputFilter,
                                private juce::ComboBox::Listener, private juce::TextEditor::Listener
    {
    public:
        // General data
        OptionPanelGeneral &panel;
        juce::Rectangle<int> previousSize;

        // Components
        juce::ComboBox boxPanningLaw;
        juce::ComboBox boxProcessor;
        juce::ComboBox boxSize;
        juce::TextEditor boxRatio;
        juce::TextEditor boxWindowWidth;
        juce::TextEditor boxWindowHeight;
    
        // Labels
        SCLabel labelDefaultPanning;
        SCLabel labelDefaultUnit;
        SCLabel labelDefaultSize;
        
        //==============================================================================================================
        explicit PanelDefaults(OptionPanelGeneral&);

        //==============================================================================================================
        void paintOverChildren(juce::Graphics&) override;
        void resized() override;
        
        //==============================================================================================================
        juce::String filterNewText(juce::TextEditor&, const juce::String&) override;

        //==============================================================================================================
        void comboBoxChanged(juce::ComboBox*) override;
        void textEditorTextChanged(juce::TextEditor&) override;
        void textEditorFocusLost(juce::TextEditor&)   override;

        JUCE_DECLARE_NON_COPYABLE(PanelDefaults)
    };
    
    // General data
    std::vector<std::pair<juce::String, juce::String>> languages;
    int currentLanguageIndex;
    int lastSelected;

    // Components
    PanelDefaults defaultsBox;
    juce::ListBox languageList;
    
    // Labels
    SCLabel labelDefaultsTitle;
    SCLabel labelSwitchLanguage;
    
    // Theme resources
    juce::Font font;
    
    //==================================================================================================================
    void reloadLocale(const jaut::Localisation&) override;
    void reloadTheme (const jaut::ThemePointer&) override;
    void reloadConfig(const jaut::Config&)       override;

    //==================================================================================================================
    int getNumRows() override;
    void paintListBoxItem(int, juce::Graphics&, int, int, bool) override;
    void listBoxItemClicked(int, const juce::MouseEvent&)       override;
    void listBoxItemDoubleClicked(int, const juce::MouseEvent&) override;

    //==================================================================================================================
    void selectLangRow(const juce::File&);

    JUCE_DECLARE_NON_COPYABLE(OptionPanelGeneral)
};

/* Holds theme settings. */
class OptionPanelThemes final : public OptionCategory
{
public:
    explicit OptionPanelThemes(CossinAudioProcessorEditor&);

    //==================================================================================================================
    void resized() override;

    //==================================================================================================================
    bool saveState(SharedData&) const override;
    void loadState(const SharedData&) override;
    
private:
    class ThemePanel final : public Component, public juce::ListBoxModel, juce::TextButton::Listener
    {
    public:
        class ThemePreview final : public Component
        {
        public:
            ThemePanel &panel;
            jaut::ThemePointer theme;
            juce::ImageComponent screenshots[10];
            juce::Viewport gallery;
            juce::HyperlinkButton buttonWebsiteLink;
            juce::HyperlinkButton buttonLicenseLink;
            juce::Label labelNoPreview;
            juce::Label labelWebsite;
            juce::Label labelLicense;
            juce::Label labelAuthors;
            
            //==========================================================================================================
            explicit ThemePreview(ThemePanel&);

            //==========================================================================================================
            void paint(juce::Graphics&) override;
            void resized() override;
            
            //==========================================================================================================
            void updateContent(const jaut::ThemePointer&);

            JUCE_DECLARE_NON_COPYABLE(ThemePreview)
        };

        //==============================================================================================================
        // General
        OptionPanelThemes &panel;
        std::vector<jaut::ThemePointer> themes;
        int selectedTheme;
        int selectedRow;
        
        // Components
        juce::TextButton buttonApply;
        ThemePreview previewBox;
        juce::ListBox themeList;

        //==============================================================================================================
        explicit ThemePanel(OptionPanelThemes&);

        //==============================================================================================================
        void paint(juce::Graphics&) override;
        void resized() override;

        //==============================================================================================================
        int getNumRows() override;
        void paintListBoxItem(int, juce::Graphics&, int, int, bool) override;
        void listBoxItemClicked(int, const juce::MouseEvent&) override;
        void listBoxItemDoubleClicked(int, const juce::MouseEvent&) override;

        //==============================================================================================================
        void buttonClicked(juce::Button*) override;
        void changeButtonState();

        JUCE_DECLARE_NON_COPYABLE(ThemePanel)
    };

    ThemePanel themePanel;
    juce::Font font;

    //==================================================================================================================
    void reloadLocale(const jaut::Localisation&) override;
    void reloadTheme (const jaut::ThemePointer&) override;

    //==================================================================================================================
    void selectThemeRow(const jaut::ThemePointer&);

    JUCE_DECLARE_NON_COPYABLE(OptionPanelThemes)
};

class OptionPanelPerformance final : public OptionCategory, private juce::ToggleButton::Listener
{
public:
    explicit OptionPanelPerformance(CossinAudioProcessorEditor&);
    ~OptionPanelPerformance() override;

    //==================================================================================================================
    void resized() override;
    void paint(juce::Graphics&) override;

    //==================================================================================================================
    bool saveState(SharedData&) const override;
    void loadState(const SharedData&) override;

private:
    juce::ComboBox boxAnimationMode;
    juce::ToggleButton tickControls;
    juce::ToggleButton tickEffects;
    SCLabel labelAnimations;
    SCLabel labelAnimationMode;
    
#if COSSIN_USE_OPENGL
    juce::ToggleButton tickHardwareAcceleration;
    juce::ToggleButton tickMultisampling;
    juce::ToggleButton tickSmoothing;
    SCLabel labelQuality;
#endif
    
    juce::Font font;

    //==================================================================================================================
    void reloadTheme(const jaut::ThemePointer&) override;
    void reloadConfig(const jaut::Config&) override;
    void reloadLocale(const jaut::Localisation&) override;

    //==================================================================================================================
    void buttonClicked(juce::Button*) override {}
    void buttonStateChanged(juce::Button*) override;
};

class CossinPluginWrapper;
class OptionPanelStandalone final : public OptionCategory
{
public:
    OptionPanelStandalone(CossinAudioProcessorEditor&, jaut::Localisation&);
    ~OptionPanelStandalone() override;

    //==================================================================================================================
    void resized() override;

    //==================================================================================================================
    bool saveState(SharedData&) const override;
    void loadState(const SharedData&) override {}

private:
    class DevicePanel final : public Component, private juce::ChangeListener
    {
    public:
        juce::TextButton buttonControlPanel;
        juce::ComboBox boxBufferSize;
        juce::ComboBox boxDevice;
        juce::ComboBox boxInput;
        juce::ComboBox boxOutput;
        juce::ComboBox boxSampleRate;
        juce::Label labelLatency;
        
        SCLabel labelDeviceType;
        SCLabel labelDeviceOutput;
        SCLabel labelDeviceInput;
        SCLabel labelSampleRate;
        SCLabel labelBufferSize;
        
        OptionPanelStandalone &panel;
        juce::AudioDeviceManager &deviceManager;
        juce::String audioDeviceSettingsCompType;

        //==============================================================================================================
        explicit DevicePanel(OptionPanelStandalone&);
        ~DevicePanel() override;

        //==============================================================================================================
        void resized() override;

        //==============================================================================================================
        void changeListenerCallback(juce::ChangeBroadcaster*) override;
    };

    class DeviceIOSelector;

    CossinPluginWrapper &plugin;
    DevicePanel devicePanel;
    std::unique_ptr<DeviceIOSelector> ioSelector;
    
    juce::String transAlertWindow;
    
    SCLabel labelTitleAudio;
    SCLabel labelTitleDevice;
    
    juce::ToggleButton tickMuteInput;
    juce::Font font;

    //==================================================================================================================
    void updateAllData();

    //==================================================================================================================
    void reloadTheme(const jaut::ThemePointer&) override;
    void reloadLocale(const jaut::Localisation&) override;
};
