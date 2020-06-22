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
    @file   MetreLookAndFeel.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include <ff_meters/ff_meters.h>
#include <juce_gui_basics/juce_gui_basics.h>

class PluginStyle;
class MetreLookAndFeel : public foleys::LevelMeter::LookAndFeelMethods
{
public:
    using MeterFlags       = foleys::LevelMeter::MeterFlags;
    using LevelMeterSource = foleys::LevelMeterSource;
    
    //==================================================================================================================
    explicit MetreLookAndFeel(PluginStyle&, float = -80.0f, int = 50) noexcept;
    
    //==================================================================================================================
    void setupDefaultMeterColours() override;
    void updateMeterGradients() override;
    
    //==================================================================================================================
    void drawMeterReduction(juce::Graphics&, foleys::LevelMeter::MeterFlags, juce::Rectangle<float>, float) override {}
    void drawTickMarks(juce::Graphics&, MeterFlags, juce::Rectangle<float>) override {}
    void drawClipIndicator(juce::Graphics&, MeterFlags, juce::Rectangle<float>, bool) override {}
    void drawClipIndicatorBackground(juce::Graphics&, MeterFlags, juce::Rectangle<float>) override {}
    void drawMaxNumberBackground(juce::Graphics&, MeterFlags, juce::Rectangle<float>) override {}
    
    //==================================================================================================================
    juce::Rectangle<float> getMeterInnerBounds(juce::Rectangle<float>, MeterFlags) const override;
    juce::Rectangle<float> getMeterBounds(juce::Rectangle<float>, MeterFlags, int, int) const override;
    juce::Rectangle<float> getMeterBarBounds(juce::Rectangle<float>, MeterFlags) const override;
    juce::Rectangle<float> getMeterTickmarksBounds(juce::Rectangle<float>, MeterFlags) const override;
    juce::Rectangle<float> getMeterClipIndicatorBounds(juce::Rectangle<float>, MeterFlags) const override;
    juce::Rectangle<float> drawBackground(juce::Graphics&, MeterFlags, juce::Rectangle<float>) override;
    juce::Rectangle<float> getMeterMaxNumberBounds(juce::Rectangle<float>, MeterFlags) const override;
    int hitTestClipIndicator(juce::Point<int>, MeterFlags, juce::Rectangle<float>, const LevelMeterSource*) const override;
    int hitTestMaxNumber(juce::Point<int>, MeterFlags, juce::Rectangle<float>, const LevelMeterSource*) const override;
    void drawMeterBars(juce::Graphics&, MeterFlags, juce::Rectangle<float>, const LevelMeterSource*, int, int) override;
    void drawMeterBarsBackground(juce::Graphics&, MeterFlags, juce::Rectangle<float>, int, int) override;
    void drawMeterChannel(juce::Graphics&, MeterFlags, juce::Rectangle<float>, const LevelMeterSource*, int) override;
    void drawMeterChannelBackground(juce::Graphics&, MeterFlags, juce::Rectangle<float>) override;
    void drawMeterBar(juce::Graphics&, MeterFlags, juce::Rectangle<float>, float, float) override;
    void drawMeterBarBackground(juce::Graphics&, MeterFlags, juce::Rectangle<float>) override;
    void drawMaxNumber(juce::Graphics&, MeterFlags, juce::Rectangle<float>, float) override;

    //==================================================================================================================
    void reloadResources() noexcept;

private:
    class LastUpdate final
    {
    public:
        explicit LastUpdate(int refreshTicks) noexcept
            : currchannel(0),
              lastValue{0, 0},
              peak{0.0f, 0.0f},
              refreshTick{0, 0},
              refreshTicks(refreshTicks),
              speedGain{1.0f, 1.0f}
        {}
    
        //==============================================================================================================
        float getUpdateMultiplier(float value, float peakVal, float maxVal, float skew) noexcept
        {
            if (value > lastValue[currchannel])
            {
                lastValue[currchannel]   = value;
                refreshTick[currchannel] = 0;
                speedGain[currchannel]   = 1.0f;
                peak[currchannel]        = peakVal;
            }
            else
            {
                if (refreshTick[currchannel] >= refreshTicks)
                {
                    ++speedGain[currchannel];
                    lastValue[currchannel] -= 1.0f * speedGain[currchannel];
                    peak[currchannel]       = std::pow(lastValue[currchannel] / maxVal, 1.0f/skew);
                }
                else
                {
                    ++refreshTick[currchannel];
                }
            }

            return lastValue[currchannel];
        }

        void setCurrentChannel(int channel)
        {
            currchannel = channel;
        }

        float getPeak() const noexcept
        {
            return juce::jmax(peak[0], peak[1]);
        }

    private:
        int currchannel;
        float lastValue[2];
        float peak[2];
        int refreshTick[2];
        const int refreshTicks;
        float speedGain[2];
    };
    
    //==================================================================================================================
    PluginStyle &pluginStyle;
    const float infinity;
    juce::Font font;
    juce::ColourGradient horizontalGradient;
    LastUpdate lastUpdate;

};
