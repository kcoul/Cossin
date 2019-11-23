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
    @file   MetreLookAndFeel.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#include "MetreLookAndFeel.h"

#include "PluginStyle.h"
#include <jaut/fontformat.h>

MetreLookAndFeel::MetreLookAndFeel(PluginStyle &pluginStyle, float infinity, int metreRefreshRateInMiliseconds) noexcept
    : pluginStyle(pluginStyle), infinity(infinity), lastUpdate(metreRefreshRateInMiliseconds)
{}

void MetreLookAndFeel::setupDefaultMeterColours()
{}

void MetreLookAndFeel::updateMeterGradients()
{
    horizontalGradient.clearColours();
}

// inner metre bounds
juce::Rectangle<float> MetreLookAndFeel::getMeterInnerBounds(const juce::Rectangle<float>,
                                                             const FFAU::LevelMeter::MeterFlags) const
{
    return {};
}

// full metre bounds outside
juce::Rectangle<float> MetreLookAndFeel::getMeterBounds(const juce::Rectangle<float> bounds,
                                                        const FFAU::LevelMeter::MeterFlags meterType,
                                                        const int, const int channel) const
{
    if (meterType & FFAU::LevelMeter::SingleChannel)
    {
        return bounds.withHeight(32.0f);
    }
    else
    {
        return bounds.withY(17.0f * static_cast<float>(channel)).withHeight(15.0f);
    }
}

// full metre bounds
juce::Rectangle<float> MetreLookAndFeel::getMeterBarBounds(const juce::Rectangle<float> bounds,
                                                           const FFAU::LevelMeter::MeterFlags) const
{
    const float margin = 2.0f;
    return {bounds.getX() + margin, bounds.getY() + margin, bounds.getWidth() - 4.0f, bounds.getHeight() - 4.0f};
}

// tick marks bounds
juce::Rectangle<float> MetreLookAndFeel::getMeterTickmarksBounds(const juce::Rectangle<float>,
                                                                 const FFAU::LevelMeter::MeterFlags) const
{
    return {};
}

// clip indicator
juce::Rectangle<float> MetreLookAndFeel::getMeterClipIndicatorBounds(const juce::Rectangle<float>,
                                                                     const FFAU::LevelMeter::MeterFlags) const
{
    return {};
}

// peak display
juce::Rectangle<float> MetreLookAndFeel::getMeterMaxNumberBounds(const juce::Rectangle<float> bounds,
                                                                 const FFAU::LevelMeter::MeterFlags) const
{
    return {0.0f, 36.0f, bounds.getWidth(), 14.0f};
}

juce::Rectangle<float> MetreLookAndFeel::drawBackground(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags,
                                                        const juce::Rectangle<float> bounds)
{
    g.setColour(pluginStyle.findColour(FFAU::LevelMeter::lmBackgroundColour));
    g.fillRect(bounds);
    
    return bounds;
}

void MetreLookAndFeel::drawMeterBars(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags meterType,
                                     const juce::Rectangle<float> bounds, const FFAU::LevelMeterSource *source,
                                     const int, const int selectedChannel)
{
    if (source)
    {
        const int numChannels        = source->getNumChannels();
        juce::Rectangle<float> maxes = getMeterMaxNumberBounds(bounds, meterType);
        float peak                   = 0.0f;

        if (meterType & FFAU::LevelMeter::SingleChannel)
        {
            const int channel    = selectedChannel < 0 || selectedChannel >= source->getNumChannels() ? 0
                                   : selectedChannel;
            const auto dest      = getMeterBounds(bounds, meterType, 0, channel);
            const auto innerDest = getMeterBarBounds(dest, meterType);
            peak                 = source->getMaxLevel(channel);
            
            lastUpdate.setCurrentChannel(channel);
            drawMeterChannel(g, meterType, innerDest, source, channel);
        }
        else
        {
            for (int channel = 0; channel < numChannels; ++channel)
            {
                const auto dest      = getMeterBounds(bounds, meterType, 0, channel);
                const auto innerDest = getMeterBarBounds(dest, meterType);
                peak                 = std::fmaxf(source->getMaxLevel(channel), peak);
                
                lastUpdate.setCurrentChannel(channel);
                drawMeterChannel(g, meterType, innerDest, source, channel);
            }
        }

        if (!maxes.isEmpty())
        {
            drawMaxNumber(g, meterType, maxes, peak);
        }
    }
}

void MetreLookAndFeel::drawMeterBarsBackground(juce::Graphics &g, FFAU::LevelMeter::MeterFlags meterType,
                                               const juce::Rectangle<float> bounds, const int numChannels, const int)
{
    if (meterType & FFAU::LevelMeter::SingleChannel)
    {
        drawMeterChannelBackground(g, meterType, getMeterBounds(bounds, meterType, 0, 0));
    }
    else
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            drawMeterChannelBackground(g, meterType, getMeterBounds(bounds, meterType, 0, channel));
        }
    }
}


void MetreLookAndFeel::drawMeterChannel(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags meterType,
                                        const juce::Rectangle<float> bounds, const FFAU::LevelMeterSource *source,
                                        const int selectedChannel)
{
    if (source)
    {
        if (!bounds.isEmpty())
        {
            drawMeterBar(g, meterType, bounds, source->getRMSLevel(selectedChannel),
                         source->getMaxLevel(selectedChannel));
        }
    }
}

void MetreLookAndFeel::drawMeterChannelBackground(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags meterType,
                                                  const juce::Rectangle<float> bounds)
{
    juce::Rectangle<float> clip = getMeterClipIndicatorBounds(bounds, meterType);
    juce::Rectangle<float> ticks = getMeterTickmarksBounds(bounds, meterType);
    juce::Rectangle<float> maxes = getMeterMaxNumberBounds(bounds, meterType);

    drawMeterBarBackground(g, meterType, bounds);

    if (!clip.isEmpty())
    {
        drawClipIndicatorBackground(g, meterType, clip);
    }

    if (!ticks.isEmpty())
    {
        drawTickMarks(g, meterType, ticks);
    }

    if (!maxes.isEmpty())
    {
        drawMaxNumberBackground(g, meterType, maxes);
    }
}

void MetreLookAndFeel::drawMeterBar(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags,
                                    const juce::Rectangle<float> bounds, const float, const float peak)
{
    const float skew      = 0.5f;
    const float skewedfac = bounds.getWidth() * std::pow(peak, skew);
    const auto dest       = bounds.withRight(jmin(bounds.getWidth(), skewedfac));

    if (horizontalGradient.getNumColours() < 2)
    {
        horizontalGradient = juce::ColourGradient(pluginStyle.findColour(FFAU::LevelMeter::lmMeterGradientLowColour),
                                                  bounds.getX(), bounds.getY(),
                                                  pluginStyle.findColour(FFAU::LevelMeter::lmMeterGradientMaxColour),
                                                  bounds.getRight(), bounds.getY(), false);
        horizontalGradient.addColour(0.5, pluginStyle.findColour(FFAU::LevelMeter::lmMeterGradientLowColour));
        horizontalGradient.addColour(0.75, pluginStyle.findColour(FFAU::LevelMeter::lmMeterGradientMidColour));
    }

    g.setGradientFill(horizontalGradient);
    g.fillRect(dest);

    float value = jlimit(2.0f, bounds.getWidth(), lastUpdate.getUpdateMultiplier(jmin(bounds.getWidth(), skewedfac)
                                                                                 + 2.0f, peak, bounds.getWidth(),
                                                                                 skew));
    
    if (value > 2.0f)
    {
        g.setColour(pluginStyle.findColour(FFAU::LevelMeter::lmTextColour));
    }
    else
    {
        g.setColour(pluginStyle.findColour(FFAU::LevelMeter::lmTextDeactiveColour));
    }

    g.drawRect(value, bounds.getY(), 2.0f, bounds.getHeight());
}

void MetreLookAndFeel::drawMeterReduction(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags meterType,
                                          const juce::Rectangle<float> bounds, const float reduction)
{}

void MetreLookAndFeel::drawMeterBarBackground(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags,
                                              const juce::Rectangle<float> bounds)
{
    g.setColour(pluginStyle.findColour(FFAU::LevelMeter::lmMeterBackgroundColour));
    g.fillRect(bounds);
    g.setColour(pluginStyle.findColour(FFAU::LevelMeter::lmMeterOutlineColour));
    g.drawRect(bounds, 1.0f);
}

void MetreLookAndFeel::drawTickMarks(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags meterType,
                                     const juce::Rectangle<float> bounds)
{}

void MetreLookAndFeel::drawClipIndicator(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags,
                                         const juce::Rectangle<float> bounds, const bool hasClipped)
{}

void MetreLookAndFeel::drawClipIndicatorBackground(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags,
                                                   const juce::Rectangle<float> bounds)
{}

void MetreLookAndFeel::drawMaxNumber(juce::Graphics &g, const FFAU::LevelMeter::MeterFlags,
                                     const juce::Rectangle<float> bounds, const float maxGain)
{
    const float maxDb = juce::Decibels::gainToDecibels(lastUpdate.getPeak(), infinity);
    g.setFont(font);
    g.setColour(pluginStyle.findColour(FFAU::LevelMeter::lmTextColour));
    jaut::FontFormat::drawSmallCaps(g, "Peak", bounds, Justification::bottomLeft);
    g.drawText((maxDb <= infinity ? "-INF" : juce::String(maxDb, 2) + " ") + "dB", bounds, Justification::bottomRight);
}

void MetreLookAndFeel::drawMaxNumberBackground(juce::Graphics&, const FFAU::LevelMeter::MeterFlags,
                                               const juce::Rectangle<float>)
{}

int MetreLookAndFeel::hitTestClipIndicator(const juce::Point<int> position,
                                           const FFAU::LevelMeter::MeterFlags meterType,
                                           const juce::Rectangle<float> bounds,
                                           const FFAU::LevelMeterSource *source) const
{
    if (source)
    {
        const int numChannels = source->getNumChannels();
        
        for (int i = 0; i < numChannels; ++i)
        {
            if (getMeterClipIndicatorBounds(getMeterBounds (bounds, meterType, source->getNumChannels(), i), meterType)
                .contains(position.toFloat()))
            {
                return i;
            }
        }
    }
    
    return -1;
}

int MetreLookAndFeel::hitTestMaxNumber(const juce::Point<int> position, const FFAU::LevelMeter::MeterFlags meterType,
                                       const juce::Rectangle<float> bounds, const FFAU::LevelMeterSource *source) const
{
    if (source)
    {
        const int numChannels = source->getNumChannels();
        
        for (int i = 0; i < numChannels; ++i)
        {
            if (getMeterMaxNumberBounds(getMeterBounds(bounds, meterType, source->getNumChannels(), i),meterType)
                .contains(position.toFloat()))
            {
                return i;
            }
        }
    }
    
    return -1;
}

//======================================================================================================================
void MetreLookAndFeel::reloadResources() noexcept
{
    setupDefaultMeterColours();
    updateMeterGradients();

    font = pluginStyle.getFont(14.0f, 0, 1.0f, 0.1f);
}
