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
    @file   OptionPanel.h
    @date   18, October 2019
    
    ===============================================================
 */

#pragma once

#include "OptionCategories.h"

class OptionPanel final : public juce::Component, public juce::ListBoxModel, private juce::LookAndFeel_V4
{
public:
    using CategoryList = jaut::TypeArray<
        OptionPanelGeneral,
        OptionPanelThemes,
        OptionPanelPerformance,
        OptionPanelStandalone
    >;
    
    using CategoryArray = CategoryList::toArray;
    using NameArray     = std::array<juce::String, CategoryList::size>;
    
    //==================================================================================================================
    std::function<bool(const juce::String&)> onIntercept;
    
    //==================================================================================================================
    explicit OptionPanel(CossinAudioProcessorEditor&);
    ~OptionPanel() override;

    //==================================================================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    
    //==================================================================================================================
    void load();
    
    //==================================================================================================================
    void setCloseButtonCallback(std::function<void(juce::Button*)>) noexcept;
    
    //==================================================================================================================
    void reloadTheme (const jaut::ThemePointer&);
    void reloadLocale(const jaut::Localisation&);
    void reloadConfig(const jaut::Config&);
    
private:
    class OptionsContainer final : public Component
    {
    public:
        explicit OptionsContainer(OptionPanel&);
        ~OptionsContainer() override;
        
        //==============================================================================================================
        void resized() override;
        
        //==============================================================================================================
        void addOptionPanel(OptionCategory&);
        void removeOptionPanel(OptionCategory&);
        void showPanel(int, int, bool);
    
    private:
        Component contentComponent;
        OptionPanel &parent;
    };
    
    struct PanelRegisterer;
    struct NameGetter;
    template<class> struct DataReloader;
    template<bool>  struct StateManager;
    
    //==================================================================================================================
    // General
    CossinAudioProcessorEditor &editor;
    std::function<void(juce::Button*)> closeCallback;
    int lastSelectedRow { 0 };

    // Components
    juce::TextButton buttonApply;
    juce::TextButton buttonCancel;
    juce::TextButton buttonOk;
    OptionsContainer optionsContainer;
    juce::ListBox listBoxOptionTabs;
    juce::HyperlinkButton linkDiscord;
    juce::HyperlinkButton linkTumblr;
    juce::HyperlinkButton linkTwitter;
    juce::HyperlinkButton linkWebsite;
    
    // Categories
    CategoryArray categories;
    NameArray     categoryNames;
    
    // Locale data
    SCLabel labelTitleOptions;
    
    // Paint data
    juce::Image imgCossinAbout;
    juce::Image imgSocialDiscord;
    juce::Image imgSocialTumblr;
    juce::Image imgSocialTwitter;
    juce::Image imgSocialWebsite;
    juce::Font font;
    
    //==================================================================================================================
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    
    //==================================================================================================================
    int getNumRows() override;
    void paintListBoxItem(int, juce::Graphics&, int, int, bool) override;
    void listBoxItemClicked(int, const juce::MouseEvent&) override;
};
