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
    @file   OptionCategories.h
    @date   03, November 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"
#include "ReloadListener.h"
#include "Resources.h"

#include <jaut/thememanager.h>

class CossinAudioProcessorEditor;
class SharedData;

class OptionCategory : public Component, public ReloadListener
{
public:
    OptionCategory(CossinAudioProcessorEditor &editor, jaut::Localisation &locale) : editor(editor), locale(locale) {}
    virtual ~OptionCategory() {}

    //==================================================================================================================
    virtual bool saveState(SharedData&) const = 0;
    virtual void loadState(const SharedData&) = 0;

    //==================================================================================================================
    virtual void reloadConfig(const jaut::Config&) override {}
    virtual void reloadTheme (const jaut::ThemePointer&) override {}
    virtual void reloadLocale(const jaut::Localisation&) override {}

protected:
    CossinAudioProcessorEditor &editor;
    jaut::Localisation &locale;
};

/* Holds general and default settings*/
class OptionPanelGeneral final : public OptionCategory, public ListBoxModel
{
public:
    OptionPanelGeneral(CossinAudioProcessorEditor&, jaut::Localisation&);

    //==================================================================================================================
    void paint(Graphics&) override;
    void resized() override;
    
    //==================================================================================================================
    bool saveState(SharedData&) const override;
    void loadState(const SharedData&) override;

private:
    class PanelDefaults final : public Component, private TextEditor::InputFilter, private ComboBox::Listener,
                                private TextEditor::Listener
    {
    public:
        // General data
        OptionPanelGeneral &panel;
        Rectangle<int> previousSize;

        // Components
        ComboBox boxPanningLaw;
        ComboBox boxProcessor;
        ComboBox boxSize;
        TextEditor boxRatio;
        TextEditor boxWindowWidth;
        TextEditor boxWindowHeight;

        //==============================================================================================================
        PanelDefaults(OptionPanelGeneral&);

        //==============================================================================================================
        void paint(Graphics&) override;
        void paintOverChildren(Graphics&) override;
        void resized() override;

        //==============================================================================================================
        void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

        //==============================================================================================================
        String filterNewText(TextEditor&, const String&) override;

        //==============================================================================================================
        void comboBoxChanged(ComboBox*) override;
        void textEditorTextChanged(TextEditor&) override;
        void textEditorFocusLost(TextEditor&)   override;

        JUCE_DECLARE_NON_COPYABLE(PanelDefaults)
    };
    
    // General data
    std::vector<std::pair<String, String>> languages;
    int currentLanguageIndex;
    int lastSelected;

    // Components
    PanelDefaults defaultsBox;
    ListBox languageList;

    // Theme resources
    Font font;

    //==================================================================================================================
    void reloadLocale(const jaut::Localisation&) override;
    void reloadTheme (const jaut::ThemePointer&) override;
    void reloadConfig(const jaut::Config&)       override;

    //==================================================================================================================
    int getNumRows() override;
    void paintListBoxItem(int, Graphics&, int, int, bool) override;
    void listBoxItemClicked(int, const MouseEvent&)       override;
    void listBoxItemDoubleClicked(int, const MouseEvent&) override;

    //==================================================================================================================
    void selectLangRow(const File&);

    JUCE_DECLARE_NON_COPYABLE(OptionPanelGeneral)
};

/* Holds theme settings. */
class OptionPanelThemes final : public OptionCategory
{
public:
    OptionPanelThemes(CossinAudioProcessorEditor&, jaut::Localisation&);

    //==================================================================================================================
    void resized() override;

    //==================================================================================================================
    bool saveState(SharedData&) const override;
    void loadState(const SharedData&) override;
    
private:
    class ThemePanel final : public Component, public ListBoxModel, TextButton::Listener
    {
    public:
        class ThemePreview final : public Component
        {
        public:
            ThemePanel &panel;
            jaut::ThemePointer theme;
            ImageComponent screenshots[10];
            Viewport gallery;
            HyperlinkButton buttonWebsiteLink;
            HyperlinkButton buttonLicenseLink;
            Label labelNoPreview;

            //==========================================================================================================
            ThemePreview(ThemePanel&);

            //==========================================================================================================
            void paint(Graphics&) override;
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
        TextButton buttonApply;
        ThemePreview previewBox;
        ListBox themeList;

        //==============================================================================================================
        ThemePanel(OptionPanelThemes&);

        //==============================================================================================================
        void paint(Graphics&) override;
        void resized() override;

        //==============================================================================================================
        int getNumRows() override;
        void paintListBoxItem(int, Graphics&, int, int, bool) override;
        void listBoxItemClicked(int, const MouseEvent&) override;
        void listBoxItemDoubleClicked(int, const MouseEvent&) override;

        //==============================================================================================================
        void buttonClicked(Button*) override;
        void changeButtonState();

        //==============================================================================================================
        void selectThemeRow(const jaut::ThemePointer&);
        void resetThemeList(const std::vector<jaut::ThemePointer>&);

        JUCE_DECLARE_NON_COPYABLE(ThemePanel)
    };

    ThemePanel themePanel;
    Font font;

    //==================================================================================================================
    void reloadLocale(const jaut::Localisation&) override;
    void reloadTheme (const jaut::ThemePointer&) override;

    //==================================================================================================================
    void selectThemeRow(const jaut::ThemePointer&);

    JUCE_DECLARE_NON_COPYABLE(OptionPanelThemes)
};

class OptionPanelPerformance final : public OptionCategory, private ToggleButton::Listener
{
public:
    OptionPanelPerformance(CossinAudioProcessorEditor&, jaut::Localisation&);
    ~OptionPanelPerformance();

    //==================================================================================================================
    void resized() override;
    void paint(Graphics&) override;

    //==================================================================================================================
    bool saveState(SharedData&) const override;
    void loadState(const SharedData&) override;

private:
    ComboBox boxAnimationMode;
    ToggleButton tickControls;
    ToggleButton tickEffects;

#if COSSIN_USE_OPENGL
    ToggleButton tickHardwareAcceleration;
    ToggleButton tickMultisampling;
    ToggleButton tickSmoothing;
#endif

    Font font;

    //==================================================================================================================
    void reloadTheme(const jaut::ThemePointer&) override;
    void reloadConfig(const jaut::Config&) override;
    void reloadLocale(const jaut::Localisation&) override;

    //==================================================================================================================
    void buttonClicked (Button*) override {}
    void buttonStateChanged(Button*) override;
};

class CossinPluginWrapper;

class OptionPanelStandalone final : public OptionCategory
{
public:
    OptionPanelStandalone(CossinAudioProcessorEditor&, jaut::Localisation&);
    ~OptionPanelStandalone();

    //==================================================================================================================
    void resized() override;
    void paint(Graphics&) override;

    //==================================================================================================================
    bool saveState(SharedData&) const override;
    void loadState(const SharedData&) override {}

private:
    class DevicePanel final : public Component, private ChangeListener
    {
    public:
        AudioDeviceManager &deviceManager;
        OptionPanelStandalone &panel;
        String audioDeviceSettingsCompType;
        
        TextButton buttonControlPanel;
        ComboBox boxBufferSize;
        ComboBox boxDevice;
        ComboBox boxInput;
        ComboBox boxOutput;
        ComboBox boxSampleRate;
        Label labelLatency;

        //==============================================================================================================
        DevicePanel(OptionPanelStandalone&);
        ~DevicePanel();

        //==============================================================================================================
        void resized() override;
        void paint(Graphics&) override;

        //==============================================================================================================
        void changeListenerCallback(ChangeBroadcaster*) override;
    };

    class DeviceIOSelector;

    CossinPluginWrapper &plugin;
    DevicePanel devicePanel;
    std::unique_ptr<DeviceIOSelector> ioSelector;

    ToggleButton tickMuteInput;

    Font font;

    //==================================================================================================================
    void updateAllData();

    //==================================================================================================================
    void reloadTheme(const jaut::ThemePointer&) override;
    void reloadLocale(const jaut::Localisation&) override;
};
