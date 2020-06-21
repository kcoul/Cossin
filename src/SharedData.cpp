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
    @file   SharedData.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "Assets.h"
#include "SharedData.h"
#include "PluginEditor.h"
#include "ThemeFolder.h"
#include "Resources.h"

//**********************************************************************************************************************
// region Namespace
//======================================================================================================================
namespace
{
// Main routine for initializing new theme packs!
jaut::IThemeDefinition* initializeThemePack(const juce::File &file, std::unique_ptr<jaut::IMetadata> metadata)
{
    auto theme = std::make_unique<ThemeFolder>(file, metadata.release());
    return theme->isValid() ? theme.release() : nullptr;
}
}
//======================================================================================================================
// endregion Namespace
//**********************************************************************************************************************
// region SharedData
//======================================================================================================================
juce::SharedResourcePointer<SharedData> SharedData::getInstance()
{
    return juce::SharedResourcePointer<SharedData>();
}

//======================================================================================================================
SharedData::SharedData() noexcept
{
    initialize();
}

SharedData::~SharedData() = default;

//======================================================================================================================
jaut::Config& SharedData::Configuration() noexcept
{
    return *appConfig;
}

jaut::ThemeManager& SharedData::ThemeManager() noexcept
{
    return *appThemes;
}

jaut::Localisation& SharedData::Localisation() noexcept
{
    return *appLocale;
}

const SharedData::ApplicationData& SharedData::AppData() const noexcept
{
    return appData;
}

const jaut::Config& SharedData::Configuration() const noexcept
{
    return *appConfig;
}

const jaut::ThemeManager& SharedData::ThemeManager() const noexcept
{
    return *appThemes;
}

const jaut::Localisation& SharedData::Localisation() const noexcept
{
    return *appLocale;
}

//======================================================================================================================
const jaut::ThemePointer& SharedData::getDefaultTheme() const noexcept
{
    return defaultTheme;
}

const jaut::Localisation& SharedData::getDefaultLocale() const noexcept
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
    MetadataHelper::setPlaceholder("name",        res::App_Name);
    MetadataHelper::setPlaceholder("version",     res::App_Version);
    MetadataHelper::setPlaceholder("author",      res::App_Author);
    MetadataHelper::setPlaceholder("vendor",      res::App_Vendor);
    MetadataHelper::setPlaceholder("license",     res::App_License);
    MetadataHelper::setPlaceholder("website",     res::App_Website);
    MetadataHelper::setPlaceholder("license_url", "https://www.gnu.org/licenses/gpl-3.0.de.html");

    initAppdata();
    initConfig();       // <- depends on appdata
    initDefaults();     // <- depends on appdata
    initLangs();        // <- depends on appdata and config
    initThemeManager(); // <- depends on appdata and config
}

void SharedData::initAppdata() const
{
    if (const juce::Result result = appData.dirRoot.createDirectory(); result.failed())
    {
        throw AppDataFolderCreationException(result.getErrorMessage() + " (" + appData.dirRoot.getFullPathName() + ")");
    }
    else
    {
        (void) appData.dirLang     .createDirectory();
        (void) appData.dirPresets  .createDirectory();
        (void) appData.dirThemes   .createDirectory();
        (void) appData.dirDataLogs .createDirectory();
        (void) appData.dirDataSaves.createDirectory();
    }
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

    const juce::File config_file = appData.dirRoot.getChildFile(options.fileName);
    auto config                  = new jaut::Config(appData.dirRoot.getFullPathName(), options,
                                                    std::make_unique<jaut::YamlParser>());

    // setup config
    // GENERAL
    jaut::Config::Property prop_theme;    // the theme identifier for the skin to use
    jaut::Config::Property prop_language; // the selected language to use

    // DEFAULTS
    jaut::Config::Property prop_initial_size; // the initial size the plugin window should have when new instantiated
    jaut::Config::Property prop_panning;      // the default panning function
    jaut::Config::Property prop_processor;    // the default processor type to be shown when a new instance was created

    // OPTIMIZATION
    jaut::Config::Property prop_hardware_acceleration; // determines whether opengl should be used to draw or not
    jaut::Config::Property prop_multisampling;         // OpenGL MSAA multisampling
    jaut::Config::Property prop_texture_smoothing;     // OpenGL texture scaling resolution
    jaut::Config::Property prop_vsync;                 // OpenGL front and back buffer swapping synchronisation
    jaut::Config::Property prop_animations;            // animation properties
    jaut::Config::Property prop_animations_custom;     // custom animations settings

    // STANDALONE
    jaut::Config::Property prop_buffer_size;      // determines the buffer size
    jaut::Config::Property prop_sample_rate;      // determines the sample rate
    jaut::Config::Property prop_io_device;        // determines the input and output devices
    jaut::Config::Property prop_mute_input;       // determines whether input should be muted due to possible feedback loops
    jaut::Config::Property prop_device_type;      // determines the type of device to use
    jaut::Config::Property prop_log_to_file;      // determines whether logging should be enabled or not
    jaut::Config::Property prop_double_precision; // determines whether samples should be processed with double precision

    //=================================: GENERAL
    prop_theme = config->createProperty(res::Prop_GeneralTheme, "default");
    prop_theme.setComment("This is the name of the current selected theme.\n"
                          "If the theme could not be found the default theme will be used instead.");

    prop_language = config->createProperty(res::Prop_GeneralLanguage, "default");
    prop_language.setComment("This is the language file that should be used.\n"
                             "If the language could not be found, the default one will be used.");

    //=================================: DEFAULTS
    prop_initial_size = config->createProperty(res::Prop_DefaultsSize, {}, res::Cfg_Defaults);
    prop_initial_size.setComment("This is the size the plugin window should have when a new\n"
                                 "session was started.");
    (void) prop_initial_size.createProperty(res::Prop_DefaultsSizeWidth,  800);
    (void) prop_initial_size.createProperty(res::Prop_DefaultsSizeHeight, 500);

    prop_panning = config->createProperty(res::Prop_DefaultsPanningMode, 1, res::Cfg_Defaults);
    prop_panning.setComment("This is the default panning function which should be used when a new\n"
                            "session was started.");

    prop_processor = config->createProperty(res::Prop_DefaultsProcessMode, 0, res::Cfg_Defaults);
    prop_processor.setComment("This is the default processor category which should be used when a new\n"
                              "session was started.");

    //=================================: OPTIMIZATION
    prop_hardware_acceleration = config->createProperty(res::Prop_OptHardwareAcceleration, true, res::Cfg_Optimization);
    prop_hardware_acceleration.setComment("Determines whether rendering should be hardware or software aided.\n"
                                          "In case weird problems arise, like glitches or lags,\n"
                                          "it is better to turn this off. (needs restart)");
    
    prop_multisampling = config->createProperty(res::Prop_OptMultisampling, false, res::Cfg_Optimization);
    prop_multisampling.setComment("Determines whether OpenGL should use multisampling to improve textures or not.\n"
                                  "Be aware that not every device may support this and/or may suffer a massive\n"
                                  "performance hit when enabled. (needs restart)");

    prop_texture_smoothing = config->createProperty(res::Prop_OptTextureSmoothing, true, res::Cfg_Optimization);
    prop_texture_smoothing.setComment("Determines whether textures should be smoothed out or not.\n"
                                      "This is true by default, but if you encounter problems with the "
                                      "visual appearance\n"
                                      "or stronger performance hits, turn this off. (needs restart)");

    prop_animations = config->createProperty(res::Prop_OptAnimations, {}, res::Cfg_Optimization);
    prop_animations.setComment("Turn on/off specific or all animations.");
    prop_animations.createProperty(res::Prop_OptAnimationsMode, 3)
                   .setComment("The level of activated animations:\n"
                               "3: Activate all animations\n"
                               "2: Activate only crucial animations\n"
                               "1: Activate only custom set animations\n"
                               "0: Deactivate all animations");
    
    prop_animations_custom = prop_animations.createProperty(res::Prop_OptAnimationsCustom, {});
    prop_animations_custom.createProperty(res::Prop_OptAnimationsEffects,    true);
    prop_animations_custom.createProperty(res::Prop_OptAnimationsComponents, true);
    
    //=================================: STANDALONE
    prop_buffer_size = config->createProperty(res::Prop_StandaloneBufferSize, 512, res::Cfg_Standalone);
    prop_buffer_size.setComment("Determines the buffer size.");
    prop_sample_rate = config->createProperty(res::Prop_StandaloneSampleRate, 44100.0f, res::Cfg_Standalone);
    prop_sample_rate.setComment("Determines the sample rate.");
    
    prop_io_device = config->createProperty(res::Prop_StandaloneDevices, {}, res::Cfg_Standalone);
    prop_io_device.setComment("Determines the input/output devices.");
    prop_io_device.createProperty(res::Prop_StandaloneDevicesInput,  "default");
    prop_io_device.createProperty(res::Prop_StandaloneDevicesOutput, "default");
    
    prop_mute_input = config->createProperty(res::Prop_StandaloneMuteInput, true, res::Cfg_Standalone);
    prop_mute_input.setComment("Determines whether input should be muted or not.\n"
                               "This is due to a possible feedback loop!");
    prop_device_type = config->createProperty(res::Prop_StandaloneDeviceType, "default", res::Cfg_Standalone);
    prop_device_type.setComment("Determines what type of device to use.");
    prop_log_to_file = config->createProperty(res::Prop_StandaloneLogToFile, false, res::Cfg_Standalone);
    prop_log_to_file.setComment("Determines if the logger should be enabled and should output to a log file.\n"
                                "This should only be used for debugging reasons as it could impact performance!");
    prop_double_precision = config->createProperty(res::Prop_StandaloneDoublePrecision, false, res::Cfg_Standalone);
    prop_double_precision.setComment("Determines whether to use single or double precision sample processing.");

    if(config->load() == jaut::Config::ErrorCodes::FileNotFound)
    {
        // If there is a problem with config saving, we've got a problem in general!
        // Maybe you don't have any access rights?
        jassert(config_file.create().wasOk());
    }
    
    (void) config->save();
    appConfig.reset(config);
}

void SharedData::initDefaults()
{
    // make default locale
    juce::MemoryInputStream locale_stream(Assets::default_lang, Assets::default_langSize, false);
    juce::LocalisedStrings  locale_def(locale_stream.readEntireStreamAsString(), true);
    defaultLocale = std::make_unique<jaut::Localisation>(appData.dirLang, locale_def);

    // make default theme
    juce::MemoryInputStream theme_stream(Assets::theme_meta, Assets::theme_metaSize, false);
    auto theme_def = new ThemeDefinition(new ThemeMeta(jaut::MetadataHelper::readMetaToNamedValueSet(theme_stream)));
    defaultTheme   = jaut::ThemePointer("default", theme_def);
}

void SharedData::initLangs()
{
    const juce::String language_name = appConfig->getProperty("language").getValue().toString();
    auto *const        locale        = new jaut::Localisation(appData.dirLang,
                                                              defaultLocale->getInternalLocalisation());
    
    if(!language_name.equalsIgnoreCase("default"))
    {
        if(!locale->setCurrentLanguage(language_name))
        {
            juce::Logger::writeToLog("Language '" + language_name + "' is not valid, keeping default.");
        }
    }

    appLocale.reset(locale);
}

void SharedData::initThemeManager()
{
    jaut::ThemeManager::Options options;
    options.cacheThemes        = true;
    options.themeMetaId        = "theme.meta";
    options.duplicateBehaviour = jaut::ThemeManager::Options::DuplicateMode::KeepLatest;
    options.defaultTheme       = defaultTheme;
    
    juce::String theme_name = appConfig->getProperty("theme").toString();
    auto *theme_manager     = new jaut::ThemeManager(appData.dirThemes, ::initializeThemePack,
                                                     std::make_unique<ThemeMetaReader>(), options);
    
    theme_manager->reloadThemes();

    if(!theme_name.equalsIgnoreCase("default"))
    {
        if(!theme_manager->setCurrentTheme(theme_name))
        {
            juce::Logger::writeToLog("Theme '" + theme_name + "' is not valid, keeping default.");
        }
    }

    appThemes.reset(theme_manager);
}
//======================================================================================================================
// endregion SharedData
//**********************************************************************************************************************
