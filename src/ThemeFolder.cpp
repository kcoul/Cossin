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
    @file   ThemeFolder.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "ThemeFolder.h"

#include "Assets.h"
#include "Util.h"

//**********************************************************************************************************************
// region Namespace
//======================================================================================================================
namespace
{
juce::Font getDefaultFont()
{
    return juce::Font("<Sans-Serif>", 14.0, 0);
}

juce::Font getFontFromFile(const juce::File &themesDir)
{
    const juce::File font_file = themesDir.getChildFile("font.ttf");
    juce::Font default_font    = getDefaultFont();
    
    DBG(juce::Font::findAllTypefaceNames().joinIntoString(";"));
    
    if (font_file.exists())
    {
        juce::FileInputStream file_stream(font_file);
        
        if (!file_stream.isExhausted())
        {
            juce::MemoryBlock font_data;
            file_stream.readIntoMemoryBlock(font_data);
            
            if (const juce::Typeface::Ptr font_ptr = juce::Typeface::createSystemTypefaceFor(font_data.getData(),
                                                                                             font_data.getSize()))
            {
                default_font = juce::Font(font_ptr);
            }
        }
    }
    
    return default_font;
}
}
//======================================================================================================================
// endregion Namespace
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
//======================================================================================================================
// endregion ThemeMeta
//**********************************************************************************************************************
// region ThemeDefinition
//======================================================================================================================
ThemeDefinition::ThemeDefinition(jaut::IMetadata *metaData)
    : meta(metaData)
{
    juce::MemoryInputStream memory_stream(Assets::colourmap_json, Assets::colourmap_jsonSize, false);
    juce::var json;
    
    if (juce::JSON::parse(memory_stream.readEntireStreamAsString(), json).wasOk())
    {
        juce::DynamicObject *const json_root = json.getDynamicObject();
        
        for (auto &[key, point] : json_root->getProperties())
        {
            juce::StringArray color_point;
            color_point.addTokens(point.toString(), ":", "\"");
            colours.emplace(key.toString().trim().toLowerCase(), std::make_pair(color_point[0].getIntValue(),
                                                                                color_point[1].getIntValue()));
        }
    }
    else
    {
        JAUT_ASSERTFALSE("Invalid format of default colour mapping file: Cossin/src/assets/colourmap.json");
    }
}

//=====================================================================================================================
juce::String ThemeDefinition::getThemeRootPath() const
{
    return {};
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

juce::String ThemeDefinition::getImageExtension() const
{
    return "png";
}

juce::Font ThemeDefinition::getThemeFont() const
{
    return ::getDefaultFont();
}

juce::Image ThemeDefinition::getMissingImage() const
{
    return juce::ImageCache::getFromMemory(Assets::missing_png, Assets::missing_pngSize);
}

juce::Colour ThemeDefinition::getThemeColour(const juce::String &colourMappingKey) const
{
    if (colours.find(colourMappingKey) != colours.end())
    {
        const auto &[x, y] = colours.at(colourMappingKey);
        return getThemeColourFromPixel(x, y);
    }

    return juce::Colours::transparentBlack;
}

juce::Colour ThemeDefinition::getThemeColourFromPixel(int x, int y) const
{
    const juce::Image colour_map = juce::ImageCache::getFromMemory(Assets::colourmap_png, Assets::colourmap_pngSize);
    return colour_map.getPixelAt(x, y);
}

jaut::IMetadata* ThemeDefinition::getThemeMeta() const
{
    return meta.get();
}

bool ThemeDefinition::isImageValid(const juce::Image &image) const
{
    return image.isValid() && image != getMissingImage();
}

bool ThemeDefinition::isValid() const
{
    return meta != nullptr && !meta->getName().isEmpty();
}

bool ThemeDefinition::fileExists(const juce::String &filePath) const
{
    return Assets::getNamedResourceOriginalFilename(filePath.toRawUTF8());
}

bool ThemeDefinition::imageExists(const juce::String &imageName) const
{
    return Assets::getNamedResourceOriginalFilename((imageName + "_" + getImageExtension()).toRawUTF8());
}
//======================================================================================================================
// endregion ThemeDefinition
//**********************************************************************************************************************
// region ThemeFolder
//======================================================================================================================
ThemeFolder::ThemeFolder(const juce::File &themeFolderPath, jaut::IMetadata *metadata)
    : ThemeDefinition(metadata),
      themeFolderPath(themeFolderPath),
      themeFont(::getFontFromFile(themeFolderPath)),
      colourMap(juce::ImageFileFormat::loadFrom(Assets::colourmap_png, Assets::colourmap_pngSize)),
      themeThumbnail(juce::ImageFileFormat::loadFrom(Assets::missing_png, Assets::missing_pngSize))
{
    {
        const juce::File colour_map_file = themeFolderPath.getChildFile("colourmap.json");
        
        if (colour_map_file.existsAsFile())
        {
            juce::FileInputStream file_stream(colour_map_file);
            juce::var json;
            
            if (!file_stream.isExhausted() && juce::JSON::parse(file_stream.readEntireStreamAsString(), json).wasOk())
            {
                juce::DynamicObject *const json_root = json.getDynamicObject();
                
                if (!json_root)
                {
                    return;
                }
                
                for (const auto &[key, point] : json_root->getProperties())
                {
                    juce::StringArray colour_point;
                    colour_point.addTokens(point.toString(), ":", "\"");
                    
                    const juce::String colour_id = key.toString().trim().toLowerCase();
                    
                    if (colours.find(colour_id) != colours.end() && colour_point.size() == 2 &&
                        colour_point[0].containsOnly("0123456789") && colour_point[1].containsOnly("0123456789"))
                    {
                        colours.emplace(colour_id, std::make_pair(colour_point[0].getIntValue(),
                                                                  colour_point[1].getIntValue()));
                    }
                    else
                    {
                        sendLog("Invalid colour '" + key + "': Either is not a valid colour-id or "
                                "mapped value is invalid, will be skipped.", "WARNING");
                    }
                }
            }
            else
            {
                sendLog("Problem loading colourmap.json for theme '" + themeFolderPath.getFileName() +
                        "': Invalid format, using default colourmap.json instead.", "ERROR");
            }
        }
    }
    
    {
        const juce::File colour_map_file = themeFolderPath.getChildFile("colourmap.png");
        juce::Image      colour_map      = juce::ImageFileFormat::loadFrom(colour_map_file);
    
        if (colour_map.isValid())
        {
            std::swap(colourMap, colour_map);
        }
        else
        {
            sendLog("Problem loading colourmap.png for theme '" + themeFolderPath.getFileName() +
                    "': Either not found or not an image, using default colour map image instead.", "ERROR");
        }
    
        const juce::File thumbnail_file = themeFolderPath.getChildFile("theme.png");
        juce::Image      thumbnail      = juce::ImageFileFormat::loadFrom(colour_map_file);
    
        if (thumbnail.isValid())
        {
            std::swap(themeThumbnail, thumbnail);
        }
        else
        {
            sendLog("Problem loading theme thumbnail for theme '" + themeFolderPath.getFileName() +
                    "': Either not found or not an image, empty image will be used instead.", "ERROR");
        }
    }
}

juce::String ThemeFolder::getThemeRootPath() const
{
    return themeFolderPath.getFullPathName();
}

juce::Image ThemeFolder::getThemeThumbnail() const
{
    return themeThumbnail;
}

juce::File ThemeFolder::getFile(const juce::String &filePath) const
{
    return themeFolderPath.getChildFile(filePath);
}

juce::Image ThemeFolder::getImage(const juce::String &imageName) const
{
    const juce::File image_file = themeFolderPath.getChildFile("object/" + imageName + "." + getImageExtension());
    juce::Image image           = juce::ImageFileFormat::loadFrom(image_file);

    if (image.isNull())
    {
        return ThemeDefinition::getImage(imageName);
    }

    return image;
}

bool ThemeFolder::fileExists(const juce::String &filePath) const
{
    return themeFolderPath.getChildFile(filePath).exists();
}

bool ThemeFolder::imageExists(const juce::String &imageName) const
{
    return fileExists("object/" + imageName + "." + getImageExtension());
}

juce::Colour ThemeFolder::getThemeColourFromPixel(int x, int y) const
{
    return colourMap.getPixelAt(x, y);
}
//======================================================================================================================
// endregion ThemeFolder
//**********************************************************************************************************************
