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
    @file   OptionPanel.h
    @date   18, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"
#include "OptionCategories.h"

class OptionPanel final : public Button::Listener, public Component, public ListBoxModel, public ReloadListener,
                          private LookAndFeel_V4
{
public:
    std::function<bool(const String&)> onIntercept;

    OptionPanel(CossinAudioProcessorEditor&, jaut::Localisation&);
    ~OptionPanel();

    //==================================================================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==================================================================================================================
    void addOptionCategory(const String&, OptionCategory&);

    //==================================================================================================================
    void show();
    void hide();
    
    //==================================================================================================================
    void setCloseButtonCallback(std::function<void(Button*)>) noexcept;

private:
    class OptionsContainer final : public Component
    {
    public:
        OptionsContainer(OptionPanel&);

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
    std::function<void(Button*)> closeCallback;
    int lastSelectedRow;
    Array<String> categories;

    // Components
    TextButton bttClose;
    OptionsContainer optionContainer;
    OptionPanelGeneral optionsGeneral;
    OptionPanelThemes optionsThemes;
    OptionPanelPerformance optionsPerformance;
    //OptionPanelAccount optionsAccount;
    std::unique_ptr<OptionPanelStandalone> optionsStandalone;
    ListBox optionTabs;
    HyperlinkButton linkDiscord;
    HyperlinkButton linkTumblr;
    HyperlinkButton linkTwitter;
    HyperlinkButton linkWebsite;

    // Shared Data
    jaut::Localisation &locale;

    // Paint data
    Image imgCossinAbout;
    Image imgSocialDiscord;
    Image imgSocialTumblr;
    Image imgSocialTwitter;
    Image imgSocialWebsite;
    Font font;

    //==================================================================================================================
    void buttonClicked(Button*) override;

    //==================================================================================================================
    void drawButtonBackground(Graphics&, Button&, const Colour&, bool, bool) override {}
    void drawButtonText(Graphics&, TextButton&, bool, bool) override;

    //==================================================================================================================
    int getNumRows() override;
    void paintListBoxItem(int, Graphics&, int, int, bool) override;
    void listBoxItemClicked(int, const MouseEvent&) override;
    
    //==================================================================================================================
    void reloadTheme(const jaut::ThemePointer&) override;
};
