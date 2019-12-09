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
#include <jaut/thememanager.h>

namespace jaut
{
    class AppData;
    class Config;
    class IThemeDefinition;
    class Localisation;
    class ThemeManager;
    class ThemePointer;
}

class CossinAudioProcessorEditor;

class SharedData final : public ActionBroadcaster
{
public:
    enum LockPriority
    {
        HIGH,
        LOW,
        NONE
    };

    class ReadLock final
    {
    public:
        ReadLock(SharedData &sharedData, LockPriority priority = HIGH)
            : sharedData(sharedData),
              lockWasSuccessful(priority == HIGH)
        {
            if(JUCEApplication::isStandaloneApp() || priority == NONE)
            {
                return;
            }

            if(priority == HIGH)
            {
                sharedData.rwLock.enterRead();
            }
            else
            {
                lockWasSuccessful = sharedData.rwLock.tryEnterRead();
            }
        }

        ~ReadLock()
        {
            if(JUCEApplication::isStandaloneApp())
            {
                return;
            }

            if(lockWasSuccessful)
            {
                sharedData.rwLock.exitRead();
            }
        }

    private:
        SharedData &sharedData;
        bool lockWasSuccessful;
    };

    class WriteLock final
    {
    public:
        WriteLock(SharedData &sharedData, LockPriority priority = HIGH)
            : sharedData(sharedData),
              lockWasSuccessful(priority == HIGH)
        {
            if(JUCEApplication::isStandaloneApp() || priority == NONE)
            {
                return;
            }

            if(priority == HIGH)
            {
                sharedData.rwLock.enterWrite();
            }
            else
            {
                lockWasSuccessful = sharedData.rwLock.tryEnterWrite();
            }
        }

        ~WriteLock()
        {
            if(JUCEApplication::isStandaloneApp())
            {
                return;
            }
            
            if(lockWasSuccessful)
            {
                sharedData.rwLock.exitWrite();
            }
        }

    private:
        SharedData &sharedData;
        bool lockWasSuccessful;
    };
    
    //==================================================================================================================
    static SharedResourcePointer<SharedData> getInstance();

    //==================================================================================================================
    SharedData() noexcept;
    ~SharedData();

    //==================================================================================================================
    jaut::AppData      &AppData()       const noexcept;
    jaut::Config       &Configuration() const noexcept;
    jaut::ThemeManager &ThemeManager()  const noexcept;
    jaut::Localisation &Localisation()  const noexcept;

    //==================================================================================================================
    const jaut::ThemePointer &getDefaultTheme()  const noexcept;
    const jaut::Localisation &getDefaultLocale() const noexcept;

    //==================================================================================================================
    void sendChangeToAllInstancesExcept(CossinAudioProcessorEditor* = nullptr) const;

private:
    friend class ReadLock;

    // Data
    std::unique_ptr<jaut::AppData>      appData;
    std::unique_ptr<jaut::Config>       appConfig;
    std::unique_ptr<jaut::ThemeManager> appThemes;
    std::unique_ptr<jaut::Localisation> appLocale;

    // Defaults
    jaut::ThemePointer defaultTheme;
    std::unique_ptr<jaut::Localisation> defaultLocale;

    // Misc
    mutable ReadWriteLock rwLock;
    bool initialized;

    //==================================================================================================================
    void initialize();
    void initAppdata();
    void initConfig();
    void initDefaults();
    void initLangs();
    void initThemeManager();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SharedData)
};
