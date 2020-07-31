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
    @file   Util.cpp
    @date   18, July 2019

    ===============================================================
 */

#pragma once



#include <juce_events/juce_events.h>

namespace
{
void sendLog(const juce::String &message, const juce::String &importance = "INFO")
{
#if !JUCE_DEBUG
    if (!juce::Logger::getCurrentLogger())
    {
        return;
    }
#endif
    
    const juce::Time time = juce::Time::getCurrentTime();
    const juce::String thread_name = juce::MessageManager::getInstance()->isThisTheMessageThread() ? "MESSAGE"
                                                                                                   : "OTHER";
    
    juce::String prependix;
    prependix << "[" << time.toString(false, true) << "]" << "[" << thread_name << "/" << importance << "] ";
    
    juce::Logger::writeToLog(prependix + message);
}
}
