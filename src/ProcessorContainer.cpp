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
    @file   ProcessorContainer.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "ProcessorContainer.h"
#include "PluginEditor.h"

ProcessorContainer::ProcessorContainer(AudioProcessor &processor, AudioProcessorValueTreeState &vts,
                                       UndoManager &undoManager)
    : mainProcessor(processor), currentProcessor(0)
{
    // TODO stack processor
    // FUTURE processors (stack, graph)
    processors.add(new jaut::DspUnitManager(processor, vts, &undoManager, 0, false)); // frame processor (0)
    //processors.add(new jaut::DspStackManager(processor, vts, &undoManager, 0, false)); // stack processor (1)
}

ProcessorContainer::~ProcessorContainer() {}

//======================================================================================================================
void ProcessorContainer::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    for(auto &processor : processors)
    {
        processor->prepareToPlay(sampleRate, samplesPerBlock);
    }
}

void ProcessorContainer::releaseResources()
{
    for(auto &processor : processors)
    {
        processor->releaseResources();
    }
}

void ProcessorContainer::processBlock(AudioBuffer<float> &buffer, MidiBuffer &midiBuffer)
{
    processors.getUnchecked(currentProcessor)->processBlock(buffer, midiBuffer);
}

//======================================================================================================================
const OwnedArray<jaut::DspUnit> &ProcessorContainer::getProcessors() const noexcept
{
    return processors;
}

//======================================================================================================================
void ProcessorContainer::readData(const ValueTree data)
{
    if(data.isValid())
    {
        var currproc = data.getProperty("SelectedIndex");

        if(!currproc.isVoid() && JT_FIX(currproc) < processors.size())
        {
            currentProcessor = currproc;
        }

        for(ValueTree processorstate : data)
        {
            if(processorstate.isValid() && processorstate.hasType("Processor") && processorstate.hasProperty("Index")
               &&processorstate.hasProperty("Name"))
            {
                int index = processorstate.getProperty("Index");
                jaut::DspUnit *proc = processors.getUnchecked(index);

                if(proc->getName() == processorstate.getProperty("Name").toString())
                {
                    proc->readData(processorstate);
                }
            }
        }
    }
}

void ProcessorContainer::writeData(ValueTree data) const
{
    if(!data.isValid())
    {
        return;
    }

    data.setProperty("SelectedIndex", currentProcessor, nullptr);
    data.removeAllChildren(nullptr);

    for(int i = 0; i < processors.size(); ++i)
    {
        jaut::DspUnit *proc = processors.getUnchecked(i);
        ValueTree processorstate("Processor");
        processorstate.setProperty("Index", i, nullptr);
        processorstate.setProperty("Name", proc->getName(), nullptr);
        proc->writeData(processorstate);
        data.appendChild(processorstate, nullptr);
    }
}
