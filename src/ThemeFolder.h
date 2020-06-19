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
    ThemeDefinition(jaut::IMetadata*) noexcept;
    ThemeDefinition(ThemeDefinition&&);
    
    //==================================================================================================================
    ThemeDefinition& operator=(ThemeDefinition&&);
    
    //==================================================================================================================
    virtual juce::String     getThemeRootPath()                  const override;
    virtual juce::Image      getThemeThumbnail()                 const override;
    virtual juce::File       getFile(const juce::String&)        const override;
    virtual juce::Image      getImage(const juce::String&)       const override;
    virtual juce::String     getImageExtension()                 const override;
    virtual juce::Font       getThemeFont()                      const override;
    virtual juce::Image      getMissingImage()                   const override;
    virtual juce::Colour     getThemeColour(const juce::String&) const override;
    virtual juce::Colour     getThemeColourFromPixel(int, int)   const override;
    virtual bool             fileExists(const juce::String&)     const override;
    virtual bool             imageExists(const juce::String&)    const override;
    virtual jaut::IMetadata* getThemeMeta()                      const override;
    virtual bool             isImageValid(const juce::Image&)    const override;
    virtual bool             isValid()                           const override;
    
    //==================================================================================================================
    friend void swap(ThemeDefinition &left, ThemeDefinition &right)
    {
        std::swap(left.cachedFont, right.cachedFont);
        left.meta.swap(right.meta);
        left.colours.swap(right.colours);
    }

protected:
    MetaPtr meta;
    juce::Font cachedFont;
    ColourMap colours;
    
    JUCE_DECLARE_NON_COPYABLE(ThemeDefinition)
};

class ThemeFolder final : public ThemeDefinition
{
public:
    //==================================================================================================================
    ThemeFolder() = default;
    ThemeFolder(const juce::File &themeFolderPath, jaut::IMetadata *metadata) noexcept;
    
    //==================================================================================================================
    const juce::String getThemeRootPath() const override;
    juce::Image getThemeThumbnail() const override;
    juce::File getFile(const juce::String &filePath) const override;
    juce::Image getImage(const juce::String &imageName) const override;
    juce::Image getMissingImage() const override;
    const bool fileExists(const juce::String &filePath) const override;
    const bool imageExists(const juce::String &imageName) const override;
    juce::Colour getThemeColourFromPixel(int x, int y) const override;
    
private:
    juce::File themeFolderPath;
};
