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

class OptionPanel final : public juce::Button::Listener, public juce::Component, public juce::ListBoxModel,
                          public ReloadListener, private juce::LookAndFeel_V4
{
public:
    std::function<bool(const juce::String&)> onIntercept;

    OptionPanel(CossinAudioProcessorEditor&, jaut::Localisation&);
    ~OptionPanel() override;

    //==================================================================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    //==================================================================================================================
    void addOptionCategory(const juce::String&, OptionCategory&);

    //==================================================================================================================
    void show();
    void hide();
    
    //==================================================================================================================
    void setCloseButtonCallback(std::function<void(juce::Button*)>) noexcept;

private:
    class OptionsContainer final : public Component
    {
    public:
        explicit OptionsContainer(OptionPanel&);

        //==============================================================================================================
        void resized() override;

        //==============================================================================================================
        void addOptionPanel(OptionCategory&);
        void showPanel(int, int, bool);

        //==============================================================================================================
        const std::vector<OptionCategory*> &getCategories() const noexcept;

    private:
        std::vector<OptionCategory*> categories;
        Component contentComponent;
        OptionPanel &parent;
    };

    friend class OptionsContainer;

    //==================================================================================================================
    // General
    CossinAudioProcessorEditor &editor;
    std::function<void(juce::Button*)> closeCallback;
    int lastSelectedRow;
    juce::Array<juce::String> categories;

    // Components
    juce::TextButton bttClose;
    OptionsContainer optionContainer;
    OptionPanelGeneral optionsGeneral;
    OptionPanelThemes optionsThemes;
    OptionPanelPerformance optionsPerformance;
    //OptionPanelAccount optionsAccount;
    std::unique_ptr<OptionPanelStandalone> optionsStandalone;
    juce::ListBox optionTabs;
    juce::HyperlinkButton linkDiscord;
    juce::HyperlinkButton linkTumblr;
    juce::HyperlinkButton linkTwitter;
    juce::HyperlinkButton linkWebsite;

    // Shared Data
    jaut::Localisation &locale;

    // Paint data
    juce::Image imgCossinAbout;
    juce::Image imgSocialDiscord;
    juce::Image imgSocialTumblr;
    juce::Image imgSocialTwitter;
    juce::Image imgSocialWebsite;
    juce::Font font;

    //==================================================================================================================
    void buttonClicked(juce::Button*) override;

    //==================================================================================================================
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;

    //==================================================================================================================
    int getNumRows() override;
    void paintListBoxItem(int, juce::Graphics&, int, int, bool) override;
    void listBoxItemClicked(int, const juce::MouseEvent&) override;
    
    //==================================================================================================================
    void reloadTheme(const jaut::ThemePointer&) override;
};
