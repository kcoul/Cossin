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
    @file   ThemeFolder.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "ThemeFolder.h"

//**********************************************************************************************************************
// region ThemeMetaReader
//======================================================================================================================
jaut::IMetadata* ThemeMetaReader::parseMetadata(juce::InputStream &stream)
{
    return new ThemeMeta(jaut::MetadataHelper::readMetaToNamedValueSet(stream));
}
//======================================================================================================================
// endregion ThemeMetaReader
//**********************************************************************************************************************
// region ThemeMeta
//======================================================================================================================
ThemeMeta::ThemeMeta(juce::NamedValueSet metaData) noexcept
    : metaData(std::move(metaData))
{}

//======================================================================================================================
juce::String ThemeMeta::getName() const
{
    return metaData["name"];
}

juce::String ThemeMeta::getAuthor() const
{
    return metaData["author"];
}

juce::StringArray ThemeMeta::getAuthors() const
{
    juce::var array_authors = metaData["authors"];
    return *array_authors.getArray();
}

juce::String ThemeMeta::getDescription() const
{
    return metaData["description"];
}

ThemeMeta::License ThemeMeta::getLicense() const
{
    auto objects = metaData["license"];
    return std::make_pair(objects[0].toString(), objects[1].toString());
}

jaut::Version ThemeMeta::getVersion() const
{
    return jaut::Version(metaData["version"]);
}

juce::String ThemeMeta::getWebsite() const
{
    return metaData["website"];
}

juce::StringArray ThemeMeta::getExcludedImages() const
{
    return juce::StringArray(static_cast<juce::Array<juce::String>>(metaData["excludedImages"]));
}

juce::StringArray ThemeMeta::getScreenshots() const
{
    return juce::StringArray(metaData["screenshots"]);
}



//==============================================================================
//============================  ThemeDefinition ================================
//==============================================================================

ThemeDefinition::ThemeDefinition(jaut::IMetadata *metaData) noexcept
    : meta(metaData)
{
    juce::MemoryInputStream input_stream(Assets::colourmap_json, Assets::colourmap_jsonSize, false);
    juce::var json;
    
    if (!input_stream.isExhausted() && juce::JSON::parse(input_stream.readEntireStreamAsString(), json).wasOk())
    {
        juce::DynamicObject *const json_root = json.getDynamicObject();

        if (!json_root)
        {
            return;
        }

        for (auto &[key, coords] : json_root->getProperties())
        {
            juce::StringArray color_coords;
            color_coords.addTokens(coords.toString(), ":", "\"");
            colours.emplace(key.toString().trim().toLowerCase(), std::make_pair(color_coords[0].getIntValue(),
                                                                                color_coords[1].getIntValue()));
        }
    }
}

ThemeDefinition::ThemeDefinition(ThemeDefinition &&other)
    : meta(std::move(other.meta)),
      colours(std::move(other.colours)),
      cachedFont(std::move(cachedFont))
{
    other.meta = nullptr;
}

//=====================================================================================================================
ThemeDefinition &ThemeDefinition::operator=(ThemeDefinition &&other)
{
    swap(*this, other);
    other.meta = nullptr;

    return *this;
}

//=====================================================================================================================
juce::String ThemeDefinition::getThemeRootPath() const
{
    return "";
}

juce::Image ThemeDefinition::getThemeThumbnail() const
{
    return juce::ImageCache::getFromMemory(Assets::theme_png, Assets::theme_pngSize);
}

juce::File ThemeDefinition::getFile(const juce::String&) const
{
    return {};
}

juce::Image ThemeDefinition::getImage(const juce::String &imageName) const
{
    int imgSize             = 0;
    const juce::String name = imageName.removeCharacters("-") + "_" + getImageExtension();
    const char *imgData     = Assets::getNamedResource(name.toRawUTF8(), imgSize);
    juce::Image img         = juce::ImageCache::getFromMemory(imgData, imgSize);

    if (img.isNull() || !img.isValid())
    {
        img = getMissingImage();
    }
    
    return img;
}

const String ThemeDefinition::getImageExtension() const
{
    return "png";
}

Font ThemeDefinition::getThemeFont() const
{
    if(cachedFont == Font())
    {
        cachedFont.setTypefaceName("Arial");
        File fontfile = getFile("font.ttf");

        if(fontfile.exists())
        {
            FileInputStream fis(fontfile);

            if(!fis.isExhausted())
            {
                MemoryBlock fontdata;
                fis.readIntoMemoryBlock(fontdata);

                if(Typeface::Ptr fontptr = Typeface::createSystemTypefaceFor(fontdata.getData(), fontdata.getSize()))
                {
                    Typeface::clearTypefaceCache();
                    cachedFont = std::move(Font(fontptr));
                }
            }
        }

        cachedFont.setSizeAndStyle(14.0f, 0, 1.0f, 0.0f);
    }

    return cachedFont;
}

Image ThemeDefinition::getMissingImage() const
{
    return ImageCache::getFromMemory(Assets::png000_png, Assets::png000_pngSize);
}

Colour ThemeDefinition::getThemeColour(const String &colorMappingKey) const
{
    if (colours.find(colorMappingKey) != colours.end())
    {
        std::tuple<int, int> paircolour = colours.at(colorMappingKey);
        return getThemeColourFromPixel(std::get<0>(paircolour), std::get<1>(paircolour));
    }

    return Colours::black;
}

Colour ThemeDefinition::getThemeColourFromPixel(int x, int y) const
{
    const Colour colour = getImage("colourmap").getPixelAt(x, y);
    return colour != Colours::transparentBlack ? colour : Colours::black;
}

const jaut::IMetadata *ThemeDefinition::getThemeMeta() const
{
    return meta.get();
}

const bool ThemeDefinition::isImageValid(const Image &image) const
{
    return image.isValid() && image != getMissingImage();
}

const bool ThemeDefinition::isValid() const
{
    return meta != nullptr && !meta->getName().isEmpty() && !meta->getVersion().isEmpty();
}

const bool ThemeDefinition::fileExists(const String &filePath) const
{
    return Assets::getNamedResourceOriginalFilename(filePath.toRawUTF8());
}

const bool ThemeDefinition::imageExists(const String &imageName) const
{
    return Assets::getNamedResourceOriginalFilename((imageName + "_" + getImageExtension()).toRawUTF8());
}



//==============================================================================
//===============================  ThemeFolder  ================================
//==============================================================================

ThemeFolder::ThemeFolder(const File &themeFolderPath) noexcept
    : themeFolderPath(themeFolderPath)
{
    if (themeFolderPath.exists())
    {
        File metadata_file   = themeFolderPath.getChildFile("theme.meta");
        File colour_map_file = themeFolderPath.getChildFile("colourmap.json");

        if (metadata_file.exists())
        {
            FileInputStream input_stream(metadata_file);

            if (input_stream.openedOk())
            {
                meta.reset(new ThemeMeta(jaut::MetadataHelper::readMetaToNamedValueSet(input_stream)));
            }
        }
        
        String colour_data;
        var json;

        if (colour_map_file.exists())
        {
            FileInputStream input_stream(colour_map_file);

            if (input_stream.openedOk() && !input_stream.isExhausted())
            {
                colour_data = input_stream.readEntireStreamAsString();
            }
        }

        if(colour_data.isEmpty())
        {
            MemoryInputStream input_stream(Assets::colourmap_json, Assets::colourmap_jsonSize, false);
            colour_data = input_stream.readEntireStreamAsString();
        }

        if (JSON::parse(colour_data, json).wasOk())
        {
            DynamicObject *root = json.getDynamicObject();

            if (!root)
            {
                return;
            }

            for (auto &[key, coords] : root->getProperties())
            {
                StringArray colour_coords;
                colour_coords.addTokens(coords.toString(), ":", "\"");
                colours.emplace(key.toString().trim().toLowerCase(),
                                std::make_pair(colour_coords[0].getIntValue(), colour_coords[1].getIntValue()));
            }
        }
    }
}

const String ThemeFolder::getThemeRootPath() const
{
    return themeFolderPath.getFullPathName();
}

Image ThemeFolder::getThemeThumbnail() const
{
    File thumbnail_file = themeFolderPath.getChildFile("theme." + getImageExtension());

    if (thumbnail_file.exists())
    {
        Image thumbnail = ImageCache::getFromFile(thumbnail_file);

        if(thumbnail.isValid())
        {
            return thumbnail;
        }
    }

    return getImage("png003");
}

File ThemeFolder::getFile(const String &filePath) const
{
    return themeFolderPath.getChildFile(filePath);
}

Image ThemeFolder::getImage(const String &imageName) const
{
    File image_file = themeFolderPath.getChildFile("object/" + imageName + "." + getImageExtension());
    Image image;

    if (image_file.exists())
    {
        image = ImageFileFormat::loadFrom(image_file);
    }

    if (image.isNull())
    {
        image = ThemeDefinition::getImage(imageName);
    }

    return image;
}

Image ThemeFolder::getMissingImage() const
{
    File image_file = themeFolderPath.getChildFile("object/png-000." + getImageExtension());
    Image image;

    if (image_file.exists())
    {
        image = ImageCache::getFromFile(image_file);
    }

    if (image.isNull())
    {
        image = ThemeDefinition::getMissingImage();
    }

    return image;
}

const bool ThemeFolder::fileExists(const String &filePath) const
{
    return themeFolderPath.getChildFile(filePath).exists();
}

const bool ThemeFolder::imageExists(const String &imageName) const
{
    return fileExists("object/" + imageName + "." + getImageExtension());
}

Colour ThemeFolder::getThemeColourFromPixel(int x, int y) const
{
    const File colour_map_file = getFile("colourmap.png");
    Colour colour;

    if(colour_map_file.exists())
    {
        Image colour_map = ImageFileFormat::loadFrom(colour_map_file);

        if(!colour_map.isValid())
        {
            colour_map = ThemeDefinition::getImage("colourmap");
        }

        colour = colour_map.getPixelAt(x, y);
    }

    return colour != Colours::transparentBlack ? colour : Colours::black;
}
