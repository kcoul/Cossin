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
    
    @author Elanda (elanda@elandasunshine.xyz)
    @file   PluginEditor.cpp
    @date   05, October 2019

    ===============================================================
 */

#pragma once

#include <juce_events/juce_events.h>

#if JUCE_OPENGL && !defined(COSSIN_USE_OPENGL)
    #define COSSIN_USE_OPENGL 1
#endif

#if JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
/**
 *  This comes in handy when messing with custom standalone windows and plugin versions.
 *  As you may want to return to the default standalone window throughout development,
 *  some things may not work anymore.
 *  Use this to make sure certain things only work as default standalone and as plugin.
 *
 *  Long story short, if you defined your custom standalone window, this will make sure that
 *  certain functionality runs only when your code is compiled as a plugin.
 *  If you defined the default standalone window, this will make the code run the same in the
 *  standalone version as also in the plugin version.
 *
 *  @param x The value which should be expanded if it's run as standalone (doesn't do anything if the default
 *            standalone filter is defined)
 *  @param y The value which should be expanded if it's not run as standalone (this will always be evaluated if the
 *            default standalone filter is defined)
 */
    #define COSSIN_IS_STANDALONE_INLINE(x, y) JUCEApplicationBase::isStandaloneApp() ? (x) : (y)

/**
 *  This comes in handy when messing with custom standalone windows and plugin versions.
 *  As you may want to return to the default standalone window throughout development,
 *  some things may not work anymore.
 *  Use this to make sure certain things only work as default standalone and as plugin.
 *
 *  Long story short, if you defined your custom standalone window, this will make sure that
 *  certain functionality runs only when your code is compiled as a plugin.
 *  If you defined the default standalone window, this will make the code run the same in the
 *  standalone version as also in the plugin version.
 *
 *  @param x The value which should be expanded if it's run as standalone (doesn't do anything if the default
 *            standalone filter is defined)
 */
    #define COSSIN_IS_STANDALONE(x) if(juce::JUCEApplicationBase::isStandaloneApp()) {x}

/**
 *  This comes in handy when messing with custom standalone windows and plugin versions.
 *  As you may want to return to the default standalone window throughout development,
 *  some things may not work anymore.
 *  Use this to make sure certain things only work as default standalone and as plugin.
 *
 *  Long story short, if you defined your custom standalone window, this will make sure that
 *  certain functionality runs only when your code is compiled as a plugin.
 *  If you defined the default standalone window, this will make the code run the same in the
 *  standalone version as also in the plugin version.
 *
 *  @param x The value which should be expanded if it's not run as standalone (this will always be evaluated if the
 *            default standalone filter is defined)
 */
    #define COSSIN_STANDALONE_ELSE(x) else {x}
#else
/**
     *  This comes in handy when messing with custom standalone windows and plugin versions.
     *  As you may want to return to the default standalone window throughout development,
     *  some things may not work anymore.
     *  Use this to make sure certain things only work as default standalone and as plugin.
     *
     *  Long story short, if you defined your custom standalone window, this will make sure that
     *  certain functionality runs only when your code is compiled as a plugin.
     *  If you defined the default standalone window, this will make the code run the same in the
     *  standalone version as also in the plugin version.
     *
     *  @param x The value which should be expanded if it's run as standalone (doesn't do anything if the default
     *           standalone filter is defined)
     *  @param y The value which should be expanded if it's not run as standalone (this will always be evaluated if the
     *           default standalone filter is defined)
     */
    #define COSSIN_IS_STANDALONE_INLINE(x, y) y

    /**
     *  This comes in handy when messing with custom standalone windows and plugin versions.
     *  As you may want to return to the default standalone window throughout development,
     *  some things may not work anymore.
     *  Use this to make sure certain things only work as default standalone and as plugin.
     *
     *  Long story short, if you defined your custom standalone window, this will make sure that
     *  certain functionality runs only when your code is compiled as a plugin.
     *  If you defined the default standalone window, this will make the code run the same in the
     *  standalone version as also in the plugin version.
     *
     *  @param x The value which should be expanded if it's run as standalone (doesn't do anything if the default
     *           standalone filter is defined)
     */
    #define COSSIN_IS_STANDALONE(x) if constexpr(false){x}

    /**
     *  This comes in handy when messing with custom standalone windows and plugin versions.
     *  As you may want to return to the default standalone window throughout development,
     *  some things may not work anymore.
     *  Use this to make sure certain things only work as default standalone and as plugin.
     *
     *  Long story short, if you defined your custom standalone window, this will make sure that
     *  certain functionality runs only when your code is compiled as a plugin.
     *  If you defined the default standalone window, this will make the code run the same in the
     *  standalone version as also in the plugin version.
     *
     *  @param x The value which should be expanded if it's not run as standalone (this will always be evaluated if the
     *           default standalone filter is defined)
     */
    #define COSSIN_STANDALONE_ELSE(x) else {x}
#endif

inline void sendLog(const juce::String &message, const juce::String &importance = "INFO")
{
    if (!juce::Logger::getCurrentLogger())
    {
        return;
    }
    
    const juce::Time   time        = juce::Time::getCurrentTime();
    const juce::String thread_name = juce::MessageManager::getInstance()->isThisTheMessageThread() ? "MESSAGE"
                                                                                                   : "OTHER";
    
    juce::String prependix;
    prependix << "[" << time.toString(false, true) << "]" << "[" << thread_name << "/" << importance << "] ";
    
    juce::Logger::writeToLog(prependix + message);
}
