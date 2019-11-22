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
    @file   SharedData.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"

#include "PluginStyle.h"
#include "ThemeFolder.h"
#include <jaut/config.h>
#include <jaut/thememanager.h>

namespace jaut
{
    class AppData;
    class Config;
    class IThemeDefinition;
    class Localisation;
}

class SharedData final : public ActionBroadcaster
{
public:
    template<bool Mode>
    class SharedDataLock;

    enum InitializationState
    {
        CREATING,
        DONE
    };
    
    //==================================================================================================================
    static SharedResourcePointer<SharedData> getInstance();

    //==================================================================================================================
    SharedData() noexcept;
    ~SharedData();

    //==================================================================================================================
    jaut::AppData      &App()           const noexcept;
    jaut::Config       &Configuration() const noexcept;
    jaut::ThemeManager &Themes()        const noexcept;
    jaut::Localisation &Locale()        const noexcept;
    PluginStyle        &Style()               noexcept;

    //==================================================================================================================
    const jaut::ThemeManager::ThemePointer getDefaultTheme() const noexcept;
    const jaut::Localisation &getDefaultLocale() const noexcept;

    //==================================================================================================================
    std::shared_ptr<SharedDataLock< true>> setReading() const noexcept;
    std::shared_ptr<SharedDataLock<false>> setWriting() const noexcept;

    //==================================================================================================================
    void sendUpdate() const;

    //==================================================================================================================
    const InitializationState &getInitializationState() const noexcept;

private:
    template<bool>
    friend class SharedDataLock;

    std::unique_ptr<jaut::AppData> appData;
    std::unique_ptr<jaut::Config> appConfig;
    std::unique_ptr<jaut::ThemeManager> appThemes;
    std::unique_ptr<jaut::Localisation> appLocale;
    PluginStyle appStyle;
    jaut::ThemeManager::ThemePointer defaultTheme;
    std::unique_ptr<jaut::Localisation> defaultLocale;
    mutable ReadWriteLock rwLock;
    InitializationState initState;

    //==================================================================================================================
    void initialize();
    void initAppdata();
    void initConfig();
    void initLangs();
    void initPluginStyle();
    void initThemeManager();
};


template<>
class SharedData::SharedDataLock<true>
{
    friend class SharedData;
public:
    const SharedData &sharedData;

    SharedDataLock(const SharedData &sharedData) noexcept
        : sharedData(sharedData)
    {
        sharedData.rwLock.enterRead();
    }

    ~SharedDataLock() noexcept
    {
        sharedData.rwLock.exitRead();
    }

    JUCE_DECLARE_NON_COPYABLE(SharedDataLock)
};

template<>
class SharedData::SharedDataLock<false>
{
    friend class SharedData;
public:
    SharedDataLock(const SharedData &sharedData) noexcept
        : sharedData(sharedData)
    {
        sharedData.rwLock.enterWrite();
    }

    ~SharedDataLock()
    {
        sharedData.rwLock.exitWrite();
    }

private:
    const SharedData &sharedData;

    JUCE_DECLARE_NON_COPYABLE(SharedDataLock)
};
