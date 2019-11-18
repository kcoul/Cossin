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
    @file   ThemeFolder.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"

#include <jaut/imetadata.h>
#include <jaut/imetareader.h>
#include <jaut/ithemedefinition.h>

class ThemeMetaReader final : public jaut::IMetaReader
{
    jaut::IMetadata *parseMetadata(InputStream &stream) override;
};

class ThemeMeta : public jaut::IMetadata
{
public:
    //=================================================================================================================
    ThemeMeta(const NamedValueSet &metadata) noexcept;
    virtual ~ThemeMeta() {}

    //=================================================================================================================
    virtual const String getName() const override;
    virtual const String getAuthor() const override;
    virtual const StringArray getAuthors() const override;
    virtual const String getDescription() const override;
    virtual const std::pair<String, String> getLicense() const override;
    virtual const String getVersion() const override;
    virtual const String getWebsite() const override;
    virtual const StringArray getExcludedImages() const override;
    virtual const StringArray getScreenshots() const override;

private:
    NamedValueSet metaData;
};

class ThemeDefinition : public jaut::IThemeDefinition
{
public:
    using ColourMap = std::unordered_map<String, std::pair<int, int>>;
    using MetaPtr   = std::unique_ptr<ThemeMeta>;

    //=================================================================================================================
    ThemeDefinition() = default;
    ThemeDefinition(ThemeMeta *metadata) noexcept;
    virtual ~ThemeDefinition() {}

    ThemeDefinition(const ThemeDefinition &other);
    ThemeDefinition(ThemeDefinition &&other);

    //=================================================================================================================
    ThemeDefinition &operator=(const ThemeDefinition &other);
    ThemeDefinition &operator=(ThemeDefinition &&other);

    //=================================================================================================================
    virtual const String getThemeRootPath() const override;
    virtual Image getThemeThumbnail() const override;
    virtual File getFile(const String &filePath) const override;
    virtual Image getImage(const String &imageName) const override;
    virtual const String getImageExtension() const override;
    virtual Font getThemeFont() const override;
    virtual Image getMissingImage() const override;
    virtual Colour getThemeColour(const String &themeColorName) const override;
    virtual Colour getThemeColourFromPixel(int x, int y) const override;
    virtual const bool fileExists(const String &filePath) const override;
    virtual const bool imageExists(const String &imagePath) const override;
    virtual const jaut::IMetadata *getThemeMeta() const override;
    virtual const bool isImageValid(const Image &image) const override;
    virtual const bool isValid() const override;

    //=================================================================================================================
    friend void swap(ThemeDefinition &left, ThemeDefinition &right)
    {
        std::swap(left.cachedFont, right.cachedFont);
        left.meta.swap(right.meta);
        left.colours.swap(right.colours);
    }

protected:
    MetaPtr meta;
    mutable Font cachedFont;
    mutable ColourMap colours;
};

class ThemeFolder final : public ThemeDefinition
{
public:
    //=================================================================================================================
    ThemeFolder() = default;
    ThemeFolder(const File &themeFolderPath) noexcept;

    //=================================================================================================================
    const String getThemeRootPath() const override;
    Image getThemeThumbnail() const override;
    File getFile(const String &filePath) const override;
    Image getImage(const String &imageName) const override;
    Image getMissingImage() const override;
    const bool fileExists(const String &filePath) const override;
    const bool imageExists(const String &imageName) const override;
    Colour getThemeColourFromPixel(int x, int y) const override;
    
private:
    File themeFolderPath;
};
