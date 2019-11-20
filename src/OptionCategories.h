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
#include <jaut/thememanager.h>

class CossinAudioProcessorEditor;

class OptionPanelGeneral final : public Component
{
    
};

class OptionPanelThemes final : public Component, public ReloadListener
{
public:
    OptionPanelThemes(CossinAudioProcessorEditor &editor);
    ~OptionPanelThemes();

    //==============================================================================================================
    void resized() override;

    //==============================================================================================================
    void selectThemeRow(const jaut::ThemeManager::ThemePointer &theme);
    void resetThemeList(const jaut::ThemeManager::ThemePointer &defaultTheme,
                        const std::vector<jaut::ThemeManager::ThemePointer> &themes);

private:
    class ThemePanel final : public Component, public ListBoxModel, TextButton::Listener
    {
    public:
        class ThemePreview final : public Component
        {
        public:
            ThemePanel &panel;
            jaut::ThemeManager::ThemePointer theme;
            ImageComponent screenshots[10];
            Viewport gallery;
            HyperlinkButton buttonWebsiteLink;
            HyperlinkButton buttonLicenseLink;
            Label labelNoPreview;

            //==========================================================================================================
            ThemePreview(ThemePanel &panel) noexcept;

            //==========================================================================================================
            void paint(Graphics&) override;
            void resized() override;
            
            //==========================================================================================================
            void updateContent(const jaut::ThemeManager::ThemePointer&) noexcept;
        };

        //==============================================================================================================
        // General
        int selectedTheme;
        int selectedRow;
        std::vector<jaut::ThemeManager::ThemePointer> themes;
        
        // Controls
        TextButton buttonApply;
        ThemePreview previewBox;
        ListBox themeList;

        // Theme resources
        Image imageApply;
        Font font;

        //==============================================================================================================
        ThemePanel() noexcept;

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
        void changeButtonState() noexcept;
    };

    CossinAudioProcessorEditor &editor;
    ThemePanel themePanel;

    //==================================================================================================================
    void reloadLocale(const jaut::Localisation&) override;
    void reloadTheme(const jaut::ThemeManager::ThemePointer &theme) override;
};
