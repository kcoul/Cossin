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
    @file   ThemeFolder.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include <jaut_provider/jaut_provider.h>

class ThemeMetaReader final : public jaut::IMetaReader
{
    jaut::IMetadata* parseMetadata(juce::InputStream&) override;
};

class ThemeMeta : public jaut::IMetadata
{
public:
    explicit ThemeMeta(juce::NamedValueSet) noexcept;

    //==================================================================================================================
    juce::String      getName()           const override;
    juce::String      getAuthor()         const override;
    juce::StringArray getAuthors()        const override;
    juce::String      getDescription()    const override;
    License           getLicense()        const override;
    jaut::Version     getVersion()        const override;
    juce::String      getWebsite()        const override;
    juce::StringArray getExcludedImages() const override;
    juce::StringArray getScreenshots()    const override;

private:
    juce::NamedValueSet metaData;
    JUCE_DECLARE_NON_COPYABLE(ThemeMeta)
};

class ThemeDefinition : public jaut::IThemeDefinition
{
public:
    using ColourMap = std::unordered_map<juce::String, std::pair<int, int>>;
    using MetaPtr   = std::unique_ptr<jaut::IMetadata>;
    
    //==================================================================================================================
    ThemeDefinition() = default;
    explicit ThemeDefinition(jaut::IMetadata*);
    
    //==================================================================================================================
    juce::String     getThemeRootPath()                  const override;
    juce::Image      getThemeThumbnail()                 const override;
    juce::File       getFile(const juce::String&)        const override;
    juce::Image      getImage(const juce::String&)       const override;
    juce::String     getImageExtension()                 const override;
    juce::Font       getThemeFont()                      const override;
    juce::Image      getMissingImage()                   const override;
    juce::Colour     getThemeColour(const juce::String&) const override;
    juce::Colour     getThemeColourFromPixel(int, int)   const override;
    bool             fileExists(const juce::String&)     const override;
    bool             imageExists(const juce::String&)    const override;
    jaut::IMetadata* getThemeMeta()                      const override;
    bool             isImageValid(const juce::Image&)    const override;
    bool             isValid()                           const override;

protected:
    MetaPtr meta;
    ColourMap colours;
};

class ThemeFolder : public ThemeDefinition
{
public:
    //==================================================================================================================
    ThemeFolder() = default;
    ThemeFolder(const juce::File &themeFolderPath, jaut::IMetadata *metadata);
    
    //==================================================================================================================
    juce::String getThemeRootPath()                const override;
    juce::Image  getThemeThumbnail()               const override;
    juce::File   getFile(const juce::String&)      const override;
    juce::Image  getImage(const juce::String&)     const override;
    bool         fileExists(const juce::String&)   const override;
    bool         imageExists(const juce::String&)  const override;
    juce::Colour getThemeColourFromPixel(int, int) const override;
    
private:
    juce::File themeFolderPath;
    juce::Font themeFont;
    juce::Image colourMap;
    juce::Image themeThumbnail;
    
    JUCE_DECLARE_NON_COPYABLE(ThemeFolder)
};
