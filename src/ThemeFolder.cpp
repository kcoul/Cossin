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
    @file   ThemeFolder.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "ThemeFolder.h"
#include <jaut/appdata.h>
#include <jaut/metadatahelper.h>

//==============================================================================
//============================  ThemeMetaReader  ===============================
//==============================================================================

jaut::IMetadata *ThemeMetaReader::parseMetadata(InputStream &stream)
{
    return new ThemeMeta(jaut::MetadataHelper::readMetaToNamedValueSet(stream));
}



//==============================================================================
//===============================  ThemeMeta  ==================================
//==============================================================================

ThemeMeta::ThemeMeta(const NamedValueSet &metaData) noexcept
    : metaData(metaData)
{}

//=====================================================================================================================
const String ThemeMeta::getName() const
{
    return metaData["name"];
}

const String ThemeMeta::getAuthor() const
{
    return metaData["author"];
}

const StringArray ThemeMeta::getAuthors() const
{
    var array_authors = metaData["authors"];
    return *array_authors.getArray();
}

const String ThemeMeta::getDescription() const
{
    return metaData["description"];
}

const std::pair<String, String> ThemeMeta::getLicense() const
{
    auto objects = metaData["license"];
    return std::make_pair(objects[0].toString(), objects[1].toString());
}

const String ThemeMeta::getVersion() const
{
    return metaData["version"];
}

const String ThemeMeta::getWebsite() const
{
    return metaData["website"];
}

const StringArray ThemeMeta::getExcludedImages() const
{
    return StringArray(static_cast<Array<String>>(metaData["excludedImages"]));
}

const StringArray ThemeMeta::getScreenshots() const
{
    return StringArray(metaData["screenshots"]);
}



//==============================================================================
//============================  ThemeDefinition ================================
//==============================================================================

ThemeDefinition::ThemeDefinition(ThemeMeta *metaData) noexcept
    : meta(metaData)
{
    MemoryInputStream mis(Assets::colourmap_json, Assets::colourmap_jsonSize, false);
    var json;

    if (mis.isExhausted())
    {
        return;
    }

    if (JSON::parse(mis.readEntireStreamAsString(), json).wasOk())
    {
        DynamicObject *jsonroot = json.getDynamicObject();

        if (!jsonroot)
        {
            return;
        }

        for (auto &[key, coords] : jsonroot->getProperties())
        {
            StringArray colorCoords;
            colorCoords.addTokens(coords.toString(), ":", "\"");
            colours.emplace(key.toString().trim().toLowerCase(), std::make_pair(colorCoords[0].getIntValue(),
                                                                                colorCoords[1].getIntValue()));
        }
    }
}

ThemeDefinition::ThemeDefinition(const ThemeDefinition &other)
    : meta(new ThemeMeta(*other.meta)),
      colours(other.colours),
      cachedFont(other.cachedFont)
{}

ThemeDefinition::ThemeDefinition(ThemeDefinition &&other)
    : meta(std::move(other.meta)),
      colours(std::move(other.colours)),
      cachedFont(std::move(cachedFont))
{
    other.meta = nullptr;
}

//=====================================================================================================================
ThemeDefinition &ThemeDefinition::operator=(const ThemeDefinition &other)
{
    ThemeDefinition temp(other);
    swap(*this, temp);

    return *this;
}

ThemeDefinition &ThemeDefinition::operator=(ThemeDefinition &&other)
{
    swap(*this, other);
    other.meta = nullptr;

    return *this;
}

//=====================================================================================================================
const String ThemeDefinition::getThemeRootPath() const
{
    return "";
}

Image ThemeDefinition::getThemeThumbnail() const
{
    return ImageCache::getFromMemory(Assets::theme_png, Assets::theme_pngSize);
}

File ThemeDefinition::getFile(const String&) const
{
    return File();
}

Image ThemeDefinition::getImage(const String &imageName) const
{
    int imgSize = 0;
    String name = imageName.removeCharacters("-") + "_" + getImageExtension();
    const char *imgData = Assets::getNamedResource(name.toRawUTF8(), imgSize);
    Image img = ImageCache::getFromMemory(imgData, imgSize);

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
