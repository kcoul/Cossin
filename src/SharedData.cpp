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
    @file   SharedData.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "SharedData.h"

#include "PluginEditor.h"
#include "Resources.h"
#include "ThemeFolder.h"
#include <jaut/appdata.h>
#include <jaut/imetareader.h>
#include <jaut/ithemedefinition.h>
#include <jaut/localisation.h>
#include <jaut/metadatahelper.h>
#include <jaut/impl/configparser.h>

namespace
{
/**
    Main routine for initializing new theme packs!
 */
jaut::IThemeDefinition *initializeThemePack(const File &file, std::unique_ptr<jaut::IMetadata> meta)
{
    std::unique_ptr<jaut::IThemeDefinition> theme(new ThemeFolder(file));
    return theme->isValid() ? theme.release() : nullptr;
}
}

//======================================================================================================================
SharedResourcePointer<SharedData> SharedData::getInstance()
{
    return SharedResourcePointer<SharedData>();
}

//======================================================================================================================
SharedData::SharedData() noexcept
    : initialized(false)
{
    initialize();
}

SharedData::~SharedData() {}

//======================================================================================================================
jaut::AppData &SharedData::AppData() const noexcept
{
    return *appData;
}

jaut::Config &SharedData::Configuration() const noexcept
{
    return *appConfig;
}

jaut::ThemeManager &SharedData::ThemeManager() const noexcept
{
    return *appThemes;
}

jaut::Localisation &SharedData::Localisation() const noexcept
{
    return *appLocale;
}

//======================================================================================================================
const jaut::ThemePointer &SharedData::getDefaultTheme() const noexcept
{
    return defaultTheme;
}

const jaut::Localisation &SharedData::getDefaultLocale() const noexcept
{
    return *defaultLocale;
}

//======================================================================================================================
void SharedData::sendChangeToAllInstancesExcept(CossinAudioProcessorEditor *except) const
{
    sendActionMessage(except ? except->getSession().id.toDashedString() : "");
}

//======================================================================================================================
void SharedData::initialize()
{
    if(initialized)
    {
        return;
    }
    
    initialized = true;

    using jaut::MetadataHelper;
    MetadataHelper::setPlaceholder("name",    res::App_Name);
    MetadataHelper::setPlaceholder("version", res::App_Version);
    MetadataHelper::setPlaceholder("author",  res::App_Author);
    MetadataHelper::setPlaceholder("vendor",  res::App_Vendor);
    MetadataHelper::setPlaceholder("license", res::App_License);
    MetadataHelper::setPlaceholder("website", res::App_Website);
    MetadataHelper::setPlaceholder("license_url", "https://www.gnu.org/licenses/gpl-3.0.de.html");

    initAppdata();
    initConfig();       // <- depends on appdata
    initDefaults();     // <- depends on appdata
    initLangs();        // <- depends on appdata and config
    initThemeManager(); // <- depends on appdata and config
}

void SharedData::initAppdata()
{
    auto appdata = new jaut::AppData(File::getSpecialLocation(File::SpecialLocationType::userApplicationDataDirectory),
                                     "ElandaSunshine/Cossin/");
    (void) appdata->withSubFolder("Lang")
                   .withSubFolder("Logs")
                   .withSubFolder("Themes")
                   .makeDir("Data")
                       .withSubFolder("Presets")
                       .withSubFolder("Saves");
    appdata->createFolderHierarchy();
    
    appData.reset(appdata);
}

void SharedData::initConfig()
{
    // init wrapper
    jaut::Config::Options options;
    options.autoSave        = false;
    options.configNotice    = "This is the ES Cossin config file.\n"
                              "Make sure you know what you are doing while you edit these settings by hand!\n"
                              "To adopt the new settings you need to close all instances of Cossin or the DAW\n"
                              "if Cossin is currently used in an active session!";
    options.fileName        = "config.yaml";
    options.processSynced   = true;
    options.defaultCategory = "general";

    const File config_root = appData->toFile();
    const File config_file = config_root.getChildFile(options.fileName);
    auto config            = new jaut::Config(config_root.getFullPathName(), options,
                                              std::make_unique<jaut::YamlParser>());

    // setup config
    // GENERAL
    jaut::Config::Property property_theme;    // the theme identifier for the skin to use
    jaut::Config::Property property_language; // the selected language to use

    // DEFAULTS
    jaut::Config::Property property_initial_size; // the initial size the plugin window should have when new instantiated
    jaut::Config::Property property_panning;      // the default panning function
    jaut::Config::Property property_processor;    // the default processor type to be shown when a new instance was created

    // OPTIMIZATION
    jaut::Config::Property property_hardware_acceleration; // determines whether opengl should be used to draw or not
    jaut::Config::Property property_multisampling;         // OpenGL MSAA multisampling
    jaut::Config::Property property_texture_smoothing;     // OpenGL texture scaling resolution
    jaut::Config::Property property_vsync;                 // OpenGL front and back buffer swapping synchronisation
    jaut::Config::Property property_animations;            // animation properties
    jaut::Config::Property property_animations_custom;     // custom animations settings

    // STANDALONE
    jaut::Config::Property property_buffer_size; // determines the buffer size
    jaut::Config::Property property_sample_rate; // determines the sample rate
    jaut::Config::Property property_io_device;   // determines the input and output devices
    jaut::Config::Property property_mute_input;  // determines whether input should be muted due to possible feedback loops
    jaut::Config::Property property_device_type; // determines the type of device to use
    jaut::Config::Property property_log_to_file; // determines whether logging should be enabled or not

    //=================================: GENERAL
    property_theme = config->createProperty("theme", "default");
    property_theme.setComment("This is the name of the current selected theme.\n"
                              "If the theme could not be found the default theme will be used instead.");

    property_language = config->createProperty("language", "default");
    property_language.setComment("This is the language file that should be used.\n"
                                 "If the language could not be found, the default one will be used.");

    //=================================: DEFAULTS
    property_initial_size = config->createProperty("size", var(), res::Cfg_Defaults);
    property_initial_size.setComment("This is the size the plugin window should have when a new\n"
                                     "session was started.");
    (void) property_initial_size.createProperty("width", 800);
    (void) property_initial_size.createProperty("height", 500);

    property_panning = config->createProperty("panning", 1, res::Cfg_Defaults);
    property_panning.setComment("This is the default panning function which should be used when a new\n"
                                "session was started.");

    property_processor = config->createProperty("processor", 0, res::Cfg_Defaults);
    property_processor.setComment("This is the default processor category which should be used when a new\n"
                                  "session was started.");

    //=================================: OPTIMIZATION
    property_hardware_acceleration = config->createProperty("hardwareAcceleration", true, res::Cfg_Optimization);
    property_hardware_acceleration.setComment("Determines whether rendering should be hardware or software aided.\n"
                                              "In case weird problems arise, like glitches or lags,\n"
                                              "it is better to turn this off. (needs restart)");
    
    property_multisampling = config->createProperty("useMultisampling", false, res::Cfg_Optimization);
    property_multisampling.setComment("Determines whether OpenGL should use multisampling to improve textures or not.\n"
                                      "Be aware that not every device may support this and/or may suffer a massive\n"
                                      "performance hit when enabled. (needs restart)");

    property_texture_smoothing = config->createProperty("textureSmoothing", true, res::Cfg_Optimization);
    property_texture_smoothing.setComment("Determines whether textures should be smoothed out or not.\n"
                                          "This is true by default, but if you encounter problems with the "
                                          "visual appearance\n"
                                          "or stronger performance hits, turn this off. (needs restart)");

    property_vsync = config->createProperty("enableVSync", true, res::Cfg_Optimization);
    property_vsync.setComment("Determines whether your GPU's and monitor's refresh rate should be synched or not.\n"
                              "Note that this may not be supported by your hardware or may affect "
                              "performance. (needs restart)");

    property_animations = config->createProperty("animations", var(), res::Cfg_Optimization);
    property_animations.setComment("Turn on/off specific or all animations.");
    property_animations.createProperty("mode", 3).setComment("The level of activated animations:\n"
                                                             "All:     Activate all animations\n"
                                                             "Reduced: Activate only crucial animations\n"
                                                             "Custom:  Activate only custom set animations\n"
                                                             "None:    Deactivate all animations");
    property_animations_custom = property_animations.createProperty("custom", var());
    property_animations_custom.createProperty("components", true);
    property_animations_custom.createProperty("effects", true);
    
    //=================================: STANDALONE
    property_buffer_size = config->createProperty("bufferSize", 512, res::Cfg_Standalone);
    property_buffer_size.setComment("Determines the buffer size.");
    property_sample_rate = config->createProperty("sampleRate", 44100.0f, res::Cfg_Standalone);
    property_sample_rate.setComment("Determines the sample rate.");
    property_io_device   = config->createProperty("devices", var(), res::Cfg_Standalone);
    property_io_device.setComment("Determines the input/output devices.");
    property_io_device.createProperty("input", "default");
    property_io_device.createProperty("output", "default");
    property_mute_input  = config->createProperty("muteInput", true, res::Cfg_Standalone);
    property_mute_input.setComment("Determines whether input should be muted or not.\n"
                                   "This is due to a possible feedback loop!");
    property_device_type = config->createProperty("deviceType", "default", res::Cfg_Standalone);
    property_device_type.setComment("Determines what type of device to use.");
    property_log_to_file = config->createProperty("logToFile", false, res::Cfg_Standalone);
    property_log_to_file.setComment("Determines if the logger should be enabled and should output to a log file.\n"
                                    "This should only be used for debugging reasons as it could impact performance!");

    Logger::getCurrentLogger()->writeToLog("1 Hardware acceleration is:" + var(property_hardware_acceleration.getValue()).toString());

    if(!config->load())
    {
        // If there is a problem with config saving, we've got a problem!
        jassert(config_file.create().wasOk());
    }

    Logger::getCurrentLogger()->writeToLog("2 Hardware acceleration is:" + var(property_hardware_acceleration.getValue()).toString());
    
    (void) config->save();

    Logger::getCurrentLogger()->writeToLog("3 Hardware acceleration is:" + var(property_hardware_acceleration.getValue()).toString());

    appConfig.reset(config);
}

void SharedData::initDefaults()
{
    // make default locale
    MemoryInputStream locale_stream(Assets::default_lang, Assets::default_langSize, false);
    LocalisedStrings  locale_def(locale_stream.readEntireStreamAsString(), true);
    defaultLocale.reset(new jaut::Localisation(appData->getDir("Lang").toFile(), locale_def));

    // make default theme
    MemoryInputStream theme_stream(Assets::theme_meta, Assets::theme_metaSize, false);
    auto theme_def = new ThemeDefinition(new ThemeMeta(jaut::MetadataHelper::readMetaToNamedValueSet(theme_stream)));
    defaultTheme   = jaut::ThemePointer("default", theme_def);
}

void SharedData::initLangs()
{    
    String language_name       = appConfig->getProperty("language").getValue().toString();
    jaut::Localisation *locale = new jaut::Localisation(appData->getDir("Lang").toFile(),
                                                        defaultLocale->getInternalLocalisation());
    
    if(!language_name.equalsIgnoreCase("default"))
    {
        if(!locale->setCurrentLanguage(language_name))
        {
            Logger::getCurrentLogger()->writeToLog("Language '" + language_name + "' is not valid, keeping default.");
        }
    }

    appLocale.reset(locale);
}

void SharedData::initThemeManager()
{
    jaut::ThemeManager::Options options;
    options.cacheThemes        = true;
    options.themeMetaId        = "theme.meta";
    options.duplicateBehaviour = jaut::ThemeManager::Options::KeepLatest;
    options.defaultTheme       = defaultTheme;
    
    String theme_name = appConfig->getProperty("theme").toString();
    auto *thememanager (new jaut::ThemeManager(appData->getDir("Themes").toFile(), ::initializeThemePack,
                        std::make_unique<ThemeMetaReader>(), options));
    
    thememanager->reloadThemes();

    if(!theme_name.equalsIgnoreCase("default"))
    {
        if(!thememanager->setCurrentTheme(theme_name))
        {
            Logger::getCurrentLogger()->writeToLog("Theme '" + theme_name + "' is not valid, keeping default.");
        }
    }

    appThemes.reset(thememanager);
}
