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
    @file   MetreLookAndFeel.h
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include "JuceHeader.h"

class PluginStyle;
class MetreLookAndFeel : public FFAU::LevelMeter::LookAndFeelMethods
{
public:
    using MeterFlags       = FFAU::LevelMeter::MeterFlags;
    using LevelMeterSource = FFAU::LevelMeterSource;

    MetreLookAndFeel(PluginStyle &pluginStyle, float infinity = -80.0f, int metreRefreshRateInMiliseconds = 50) noexcept;

    void setupDefaultMeterColours() override;
    void updateMeterGradients() override;

    juce::Rectangle<float> getMeterInnerBounds(const juce::Rectangle<float> bounds,
                                               const MeterFlags meterType) const override;
                                               
    juce::Rectangle<float> getMeterBounds(const juce::Rectangle<float> bounds, const MeterFlags meterType,
                                          const int numChannels, const int channel) const override;
                                          
    juce::Rectangle<float> getMeterBarBounds(const juce::Rectangle<float> bounds,
                                             const MeterFlags meterType) const override;
                                             
    juce::Rectangle<float> getMeterTickmarksBounds(const juce::Rectangle<float> bounds,
                                                   const MeterFlags meterType) const override;
                                                   
    juce::Rectangle<float> getMeterClipIndicatorBounds(const juce::Rectangle<float> bounds,
                                                       const MeterFlags meterType) const override;
                                                       
    juce::Rectangle<float> drawBackground(juce::Graphics &, const MeterFlags meterType,
                                          const juce::Rectangle<float> bounds) override;
                                          
    juce::Rectangle<float> getMeterMaxNumberBounds(const juce::Rectangle<float> bounds,
                                                   const MeterFlags meterType) const override;

    int hitTestClipIndicator(const juce::Point<int> position, const MeterFlags meterType,
                             const juce::Rectangle<float> bounds, const LevelMeterSource *source) const override;
                             
    int hitTestMaxNumber(const juce::Point<int> position, const MeterFlags meterType,
                         const juce::Rectangle<float> bounds, const LevelMeterSource *source) const override;

    void drawMeterBars(juce::Graphics &, const MeterFlags meterType, const juce::Rectangle<float> bounds,
                       const LevelMeterSource *source, const int fixedNumChannels = -1,
                       const int selectedChannel = -1) override;
                       
    void drawMeterBarsBackground(juce::Graphics &, const MeterFlags meterType, const juce::Rectangle<float> bounds,
                                 const int numChannels, const int fixedNumChannels = -1) override;
                                 
    void drawMeterChannel(juce::Graphics &, const MeterFlags meterType, const juce::Rectangle<float> bounds,
                          const LevelMeterSource *source, const int selectedChannel) override;
                          
    void drawMeterChannelBackground(juce::Graphics &, const MeterFlags meterType,
                                    const juce::Rectangle<float> bounds) override;
                                    
    void drawMeterBar(juce::Graphics &, const MeterFlags meterType, const juce::Rectangle<float> bounds,
                      const float rms, const float peak) override;
                      
    void drawMeterReduction(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags meterType,
                            const juce::Rectangle<float> bounds, const float reduction) override;
                            
    void drawMeterBarBackground(juce::Graphics &, const MeterFlags meterType,
                                const juce::Rectangle<float> bounds) override;
                                
    void drawTickMarks(juce::Graphics &, const MeterFlags meterType, const juce::Rectangle<float> bounds) override;
    
    void drawClipIndicator(juce::Graphics &, const MeterFlags meterType, const juce::Rectangle<float> bounds,
                           const bool hasClipped) override;
                           
    void drawClipIndicatorBackground(juce::Graphics &, const MeterFlags meterType,
                                     const juce::Rectangle<float> bounds) override;
                                     
    void drawMaxNumber(juce::Graphics &, const MeterFlags meterType, const juce::Rectangle<float> bounds,
                       const float maxGain) override;
                       
    void drawMaxNumberBackground(juce::Graphics &, const MeterFlags meterType,
                                 const juce::Rectangle<float> bounds) override;

    //==================================================================================================================
    void reloadResources() noexcept;

private:
    class LastUpdate final
    {
    public:
        LastUpdate(int refreshTicks) noexcept
            : currchannel(0),
              lastValue{0, 0},
              peak{0.0f, 0.0f},
              refreshTick{0, 0},
              refreshTicks(refreshTicks),
              speedGain{1.0f, 1.0f}
        {}

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
            return jmax(peak[0], peak[1]);
        }

    private:
        int currchannel;
        float lastValue[2];
        float peak[2];
        int refreshTick[2];
        const int refreshTicks;
        float speedGain[2];
    };

    PluginStyle &pluginStyle;
    const float infinity;
    Font font;
    ColourGradient horizontalGradient;
    LastUpdate lastUpdate;

};
