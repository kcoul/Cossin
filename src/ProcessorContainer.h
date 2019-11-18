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
    @file   ProcessorContainer.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include <jaut/dspunitmanager.h>

class ProcessorContainer final : public AudioProcessor
{
public:
    ProcessorContainer(AudioProcessor &processor, AudioProcessorValueTreeState &vts, UndoManager &undoManager);
    ~ProcessorContainer();

    //==================================================================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(AudioBuffer<float>&, MidiBuffer&) override;

    //=================================================================================================================
    bool hasEditor() const override { return false; }
    AudioProcessorEditor *createEditor() override { return nullptr; }

    //=================================================================================================================
    const String getName() const override { return "ProcessorContainer"; }

#if(1) // unused
private:
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }

    //=================================================================================================================
    int getNumPrograms() override { return 0; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override {}
    const String getProgramName (int index) override { return ""; }
    void changeProgramName (int index, const String &newName) override {}

    //=================================================================================================================
    void getStateInformation (MemoryBlock &destData) override {}
    void setStateInformation (const void *data, int sizeInBytes) override {}

public:
#endif

    //=================================================================================================================
    const OwnedArray<jaut::DspUnit> &getProcessors() const noexcept;

    //=================================================================================================================
    void readData(const ValueTree data);
    void writeData(ValueTree data) const;

private:
    friend class CossinAudioProcessor;

    AudioProcessor &mainProcessor;
    OwnedArray<jaut::DspUnit> processors;
    int currentProcessor;
};
