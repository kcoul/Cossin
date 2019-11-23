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

#include "Resources.h"
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
jaut::IThemeDefinition *initializeThemePack(const File &file, jaut::IMetadata *meta)
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
    : initState(InitializationState::CREATING)
{
    initialize();
}

SharedData::~SharedData()
{
    if(appConfig)
    {
        appConfig->save();
    }
}

//======================================================================================================================
jaut::AppData &SharedData::App() const noexcept
{
    return *appData;
}

jaut::Config &SharedData::Configuration() const noexcept
{
    return *appConfig;
}

jaut::ThemeManager &SharedData::Themes() const noexcept
{
    return *appThemes;
}

jaut::Localisation &SharedData::Locale() const noexcept
{
    return *appLocale;
}

PluginStyle &SharedData::Style() noexcept
{
    return appStyle;
}

//======================================================================================================================
const jaut::ThemeManager::ThemePointer SharedData::getDefaultTheme() const noexcept
{
    return defaultTheme;
}

const jaut::Localisation &SharedData::getDefaultLocale() const noexcept
{
    return *defaultLocale;
}

//======================================================================================================================
std::shared_ptr<SharedData::SharedDataLock<true>> SharedData::setReading() const noexcept
{
    return !JUCEApplicationBase::isStandaloneApp() ? std::make_shared<SharedDataLock<true>>(*this) : nullptr;
}

std::shared_ptr<SharedData::SharedDataLock<false>> SharedData::setWriting() const noexcept
{
    return !JUCEApplicationBase::isStandaloneApp() ? std::make_shared<SharedDataLock<false>>(*this) : nullptr;
}

//======================================================================================================================
void SharedData::sendUpdate() const
{
    sendActionMessage(String());
}

//======================================================================================================================
const SharedData::InitializationState &SharedData::getInitializationState() const noexcept
{
    return initState;
}

//======================================================================================================================
void SharedData::initialize()
{
    if (initState != InitializationState::CREATING)
    {
        return;
    }

    rwLock.enterWrite();

    // make default theme
    MemoryInputStream theme_stream(Assets::theme_meta, Assets::theme_metaSize, false);
    defaultTheme = jaut::ThemeManager::ThemePointer("default",
                       new ThemeDefinition(new ThemeMeta(jaut::MetadataHelper::readMetaToNamedValueSet(theme_stream))));

    // make default locale
    MemoryInputStream locale_stream(Assets::default_lang, Assets::default_langSize, false);
    defaultLocale.reset(new jaut::Localisation(File(),
                                               LocalisedStrings(locale_stream.readEntireStreamAsString(), true)));

    initAppdata();
    initConfig();       // <- depends on appdata
    initLangs();        // <- depends on appdata and config
    initThemeManager(); // <- depends on appdata and config
    initPluginStyle();  // <- depends on config and thememanager

    using jaut::MetadataHelper;
    MetadataHelper::setPlaceholder("name",    res::App_Name);
    MetadataHelper::setPlaceholder("version", res::App_Version);
    MetadataHelper::setPlaceholder("author",  res::App_Author);
    MetadataHelper::setPlaceholder("vendor",  res::App_Vendor);
    MetadataHelper::setPlaceholder("license", res::App_License);
    MetadataHelper::setPlaceholder("website", res::App_Website);
    MetadataHelper::setPlaceholder("license_url", "https://www.gnu.org/licenses/gpl-3.0.de.html");

    initState = InitializationState::DONE;

    rwLock.exitWrite();
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
    options.autoSave      = false;
    options.configNotice  = "This is the ES Cossin config file.\n"
                            "Make sure you know what you are doing while you edit these settings by hand!\n"
                            "To adopt the new settings you need to close all instances of Cossin or the DAW\n"
                            "if Cossin is currently used in an active session!";
    options.fileName      = "config.yaml";
    options.processSynced = true;

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
    jaut::Config::Property property_panning;     // the default panning function
    jaut::Config::Property property_processor;   // the default processor type to be shown when a new instance was created

    // OPTIMIZATION
    jaut::Config::Property property_hardware_acceleration; // determines whether opengl should be used to draw or not
    //jaut::Config::Property property_memory_save_mode;     // the value whether memory mode should be used or not
    jaut::Config::Property property_animations;           // animation properties
    jaut::Config::Property property_animations_custom;     // custom animations settings

    // STANDALONE
    jaut::Config::Property property_buffer_size; // determines the buffer size
    jaut::Config::Property property_sample_rate; // determines the sample rate
    jaut::Config::Property property_io_device;   // determines the input and output devices
    jaut::Config::Property property_mute_input;  // determines whether input should be muted due to possible feedback loops
    jaut::Config::Property property_device_type;    // determines the type of device to use
    jaut::Config::Property property_log_to_file;  // determines whether logging should be enabled or not

    //=================================: GENERAL
    property_theme = config->createProperty("theme", "default", res::Cfg_General);
    property_theme.setComment("Set this to the name of a theme folder in your themes directory.");

    property_language = config->createProperty("language", "en_GB", res::Cfg_General);
    property_language.setComment("Set the language which the app should be displayed in.");

    //=================================: DEFAULTS
    property_initial_size = config->createProperty("size", var(), res::Cfg_Defaults);
    property_initial_size.setComment("Set this to the initial size the plugin window should have when "
                                     "it's newly instantiated.");
    (void) property_initial_size.createProperty("width", 800);
    (void) property_initial_size.createProperty("height", 500);

    property_panning = config->createProperty("panning", 1, res::Cfg_Defaults);
    property_panning.setComment("Set this to the default panning function. (0 = linear, 1 = square, 2 = sine)");

    property_processor = config->createProperty("processor", 0, res::Cfg_Defaults);
    property_processor.setComment("Set this to the default processor which should be initially shown when opening a new"
                                  " instance of Cossin. (0 = solo, 1 = stack)");

    //=================================: OPTIMIZATION
    property_hardware_acceleration = config->createProperty("hardwareAcceleration", true, res::Cfg_Optimization);
    property_hardware_acceleration.setComment("Sets whether rendering should be hardware or software aided.\n"
                                              "In case weird problems arise, like glitches or lag, "
                                              "it is better to turn this off!");

    /*
        property_memory_save_mode = config->createProperty("memorySaveMode", false, res::Cfg_Optimization);
        property_memory_save_mode.setComment("Sets whether memory save mode should be used or not.\n"
                                             "Read the plugin docs for more information!");
    */
    
    property_animations = config->createProperty("animations", var(), res::Cfg_Optimization);
    property_animations.setComment("Change how the plugin should be animated or\nif it shouldn't be animated at all");
    property_animations.createProperty("mode", 3).setComment("The animation mode.\n"
                                                             "(3 = All, 2 = Effects only, 1 = Custom, 0 = None)");
    property_animations_custom = property_animations.createProperty("custom", var());
    property_animations_custom.createProperty("components", true).setComment("Animate components like: "
                                                                             "Smooth panel change.");
    property_animations_custom.createProperty("effects", true).setComment("Animate effect's visuals like: "
                                                                          "Smooth EQ points moving.");
    
    //=================================: STANDALONE
    property_buffer_size = config->createProperty("bufferSize", 512, res::Cfg_Standalone);
    property_buffer_size.setComment("Sets the buffer size for the standalone Cossin app.");
    property_sample_rate = config->createProperty("sampleRate", 44100.0, res::Cfg_Standalone);
    property_sample_rate.setComment("Sets the sample rate for the standalone Cossin app.");
    property_io_device   = config->createProperty("devices", var(), res::Cfg_Standalone);
    property_io_device.setComment("Sets the input/output device that should be used in the standalone Cossin app.");
    property_io_device.createProperty("input", "default");
    property_io_device.createProperty("output", "default");
    property_mute_input  = config->createProperty("muteInput", true, res::Cfg_Standalone);
    property_mute_input.setComment("Sets whether input should be muted or not."
                                   "This is due to a possible feedback loop!");
    property_device_type = config->createProperty("deviceType", "default", res::Cfg_Standalone);
    property_device_type.setComment("Sets what type of device to use.");
    property_log_to_file = config->createProperty("logToFile", false, res::Cfg_Standalone);
    property_log_to_file.setComment("Sets if the logger should be enabled and should output to a log file.\n"
                                    "This should only be used for debugging reasons as it could impact performance!");

    if (config_file.exists())
    {
        (void) config->load();
    }
    else
    {
        (void) config_file.create();
    }

    (void) config->save();
    appConfig.reset(config);
}

void SharedData::initLangs()
{    
    String language_name       = appConfig->getProperty("language").getValue().toString();
    jaut::Localisation *locale = new jaut::Localisation(appData->getDir("Lang").toFile(),
                                                        defaultLocale->getInternalLocalisation());
    
    if(!language_name.equalsIgnoreCase("default") && !language_name.equalsIgnoreCase("en_gb"))
    {
        (void) locale->setCurrentLanguage(language_name);
    }

    appLocale.reset(locale);
}

void SharedData::initPluginStyle()
{
    jaut::ThemeManager::ThemePointer theme_to_apply = defaultTheme;

    if(auto theme = appThemes->getThemePack(appConfig->getProperty("theme", res::Cfg_General).getValue()))
    {
        theme_to_apply = theme;
    }

    appStyle.reset(theme_to_apply);
    Desktop::getInstance().setDefaultLookAndFeel(&appStyle);
}

void SharedData::initThemeManager()
{
    jaut::ThemeManager::Options options;
    options.cacheThemes = true;
    options.themeMetaId = "theme.meta";
    auto *thememanager(new jaut::ThemeManager(appData->getDir("Themes").toFile(), ::initializeThemePack,
                       std::make_unique<ThemeMetaReader>(), options));
    thememanager->reloadThemePacks();
    appThemes.reset(thememanager);
}
