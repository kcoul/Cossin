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

inline String getLocaleName(const jaut::Localisation &locale)
{
    return locale.getInternalLocalisation().getLanguageName()
         + locale.getInternalLocalisation().getCountryCodes().joinIntoString("-");
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
    MemoryInputStream mismd(Assets::theme_meta, Assets::theme_metaSize, false);
    defaultTheme = jaut::ThemeManager::ThemePointer("default",
                   new ThemeDefinition(new ThemeMeta(jaut::MetadataHelper::readMetaToNamedValueSet(mismd))));

    // make default locale
    MemoryInputStream mis(Assets::default_lang, Assets::default_langSize, false);
    defaultLocale.reset(new jaut::Localisation(File(), LocalisedStrings(mis.readEntireStreamAsString(), true)));

    initAppdata();
    initConfig();       // <- depends on appdata
    initLangs();        // <- depends on appdata and config
    initThemeManager(); // <- depends on appdata and config
    initPluginStyle();  // <- depends on config and thememanager

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
    jaut::Config::Options cfoptions;
    File configfile         = appData->toFile();
    cfoptions.autoSave      = false;
    cfoptions.configNotice  = "This is the ESAC Cossin config file.\n"
                              "Make sure you know what you are doing while you edit these settings by hand!\n"
                              "To adopt the new settings you need to close all instances of Cossin or the DAW\n"
                              "if Cossin is currently used in an active session!";
    cfoptions.fileName      = "config.yaml";
    cfoptions.processSynced = true;
    auto config             = new jaut::Config(configfile.getFullPathName(), cfoptions,
                                               std::make_unique<jaut::YamlParser>());
    File conffile           = configfile.getChildFile(config->getOptions().fileName);

    // setup config
    // GENERAL
    jaut::Config::Property propertytheme;    // the theme identifier for the skin to use
    jaut::Config::Property propertylanguage; // the selected language to use

    // DEFAULTS
    jaut::Config::Property propertyinitialsize; // the initial size the plugin window should have when new instantiated
    jaut::Config::Property propertypanning;     // the default panning function
    jaut::Config::Property propertyprocessor;   // the default processor type to be shown when a new instance was created

    // OPTIMIZATION
    jaut::Config::Property propertyhardwareacceleration; // determines whether opengl should be used to draw or not
    //jaut::Config::Property propertymemorysavemode;     // the value whether memory mode should be used or not
    jaut::Config::Property propertyanimations;           // animation properties
    jaut::Config::Property propertyanimationscustom;     // custom animations settings

    // STANDALONE
    jaut::Config::Property propertybuffersize; // determines the buffer size
    jaut::Config::Property propertysamplerate; // determines the sample rate
    jaut::Config::Property propertyiodevice;   // determines the input and output devices
    jaut::Config::Property propertymuteinput;  // determines whether input should be muted due to possible feedback loops
    jaut::Config::Property propertydevtype;    // determines the type of device to use
    jaut::Config::Property propertylogtofile;  // determines whether logging should be enabled or not

    //=================================: GENERAL
    propertytheme = config->createProperty("theme", "default", res::Cfg_General);
    propertytheme.setComment("Set this to the name of a theme folder in your themes directory.");

    propertylanguage = config->createProperty("language", "en_GB", res::Cfg_General);
    propertylanguage.setComment("Set the language which the app should be displayed in.");

    //=================================: DEFAULTS
    propertyinitialsize = config->createProperty("size", var(), res::Cfg_Defaults);
    propertyinitialsize.setComment("Set this to the initial size the plugin window should have when "
                                   "it's newly instantiated.");
    (void) propertyinitialsize.createProperty("width", 800);
    (void) propertyinitialsize.createProperty("height", 500);

    propertypanning = config->createProperty("panning", 1, res::Cfg_Defaults);
    propertypanning.setComment("Set this to the default panning function. (0 = linear, 1 = square, 2 = sine)");

    propertyprocessor = config->createProperty("processor", 0, res::Cfg_Defaults);
    propertyprocessor.setComment("Set this to the default processor which should be initially shown when opening a new "
                                 "instance of Cossin. (0 = solo, 1 = stack)");

    //=================================: OPTIMIZATION
    propertyhardwareacceleration = config->createProperty("hardwareAcceleration", true, res::Cfg_Optimization);
    propertyhardwareacceleration.setComment("Sets whether rendering should be hardware or software aided.\n"
                                            "In case weird problems arise, like glitches or lag, "
                                            "it is better to turn this off!");

    /*
        propertymemorysavemode = config->createProperty("memorySaveMode", false, res::Cfg_Optimization);
        propertymemorysavemode.setComment("Sets whether memory save mode should be used or not.\n"
                                          "Read the plugin docs for more information!");
    */
    
    propertyanimations = config->createProperty("animations", var(), res::Cfg_Optimization);
    propertyanimations.setComment("Change how the plugin should be animated or\nif it shouldn't be animated at all");
    (void) propertyanimations.createProperty("mode", 3).setComment("The animation mode.\n"
                                                                   "(3 = All, 2 = Effects only, 1 = Custom, 0 = None)");
    propertyanimationscustom = propertyanimations.createProperty("custom", var());
    (void) propertyanimationscustom.createProperty("components", true).setComment("Animate components like: "
                                                                                  "Smooth panel change.");
    (void) propertyanimationscustom.createProperty("effects", true).setComment("Animate effect's visuals like: "
                                                                               "Smooth EQ points moving.");
    
    //=================================: STANDALONE
    propertybuffersize = config->createProperty("bufferSize", 512, res::Cfg_Standalone);
    propertybuffersize.setComment("Sets the buffer size for the standalone Cossin app.");
    propertysamplerate = config->createProperty("sampleRate", 44100.0, res::Cfg_Standalone);
    propertysamplerate.setComment("Sets the sample rate for the standalone Cossin app.");
    propertyiodevice   = config->createProperty("devices", var(), res::Cfg_Standalone);
    propertyiodevice.setComment("Sets the input/output device that should be used in the standalone Cossin app.");
    propertyiodevice.createProperty("input", "default");
    propertyiodevice.createProperty("output", "default");
    propertymuteinput = config->createProperty("muteInput", true, res::Cfg_Standalone);
    propertymuteinput.setComment("Sets whether input should be muted or not."
                                 "This is due to a possible feedback loop!");
    propertydevtype   = config->createProperty("deviceType", "default", res::Cfg_Standalone);
    propertydevtype.setComment("Sets what type of device to use.");
    propertylogtofile = config->createProperty("logToFile", false, res::Cfg_Standalone);
    propertylogtofile.setComment("Sets if the logger should be enabled and should output to a log file.\n"
                                 "This should only be used for debugging reasons as it could impact performance!");

    if (conffile.exists())
    {
        (void) config->load();
    }
    else
    {
        (void) conffile.create();
    }

    (void) config->save();
    appConfig.reset(config);
}

void SharedData::initLangs()
{    
    String langname(appConfig->getProperty("language").getValue().toString());
    jaut::Localisation *locale(new jaut::Localisation((*appData)["Lang"].toFile(),
                                                      LocalisedStrings(defaultLocale->getInternalLocalisation())));
    locale->setCurrentLanguage(langname);
    appLocale.reset(locale);
}

void SharedData::initPluginStyle()
{
    jaut::ThemeManager::ThemePointer themetoset = defaultTheme;

    if(auto theme = appThemes->getThemePack(appConfig->getProperty("theme", res::Cfg_General).getValue()))
    {
        themetoset = theme;
    }

    appStyle.reset(themetoset);
    Desktop::getInstance().setDefaultLookAndFeel(&appStyle);
}

void SharedData::initThemeManager()
{
    jaut::ThemeManager::Options tmoptions;
    tmoptions.cacheThemes = true;
    tmoptions.themeMetaId = "theme.meta";
    auto *thememanager(new jaut::ThemeManager(appData->getDir("Themes").toFile(), ::initializeThemePack,
                       std::make_unique<ThemeMetaReader>(), tmoptions));
    thememanager->reloadThemePacks();
    appThemes.reset(thememanager);
}
