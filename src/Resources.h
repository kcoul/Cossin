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
    @file   Resources.cpp
    @date   05, October 2019
    
    ===============================================================
 */

#pragma once

#include <array>

namespace res
{
using CString = const char*;


/** App info */
inline constexpr CString App_Name    = "Cossin";
inline constexpr CString App_Version = "0.1.0";
inline constexpr CString App_Author  = "Elanda";
inline constexpr CString App_Vendor  = "ElandaSunshine";
inline constexpr CString App_Website = "iamelsa.xyz";
inline constexpr CString App_License = "GPL v3";


/** Image dictionary */
inline constexpr CString Png_Empty       = "png-000";
inline constexpr CString Png_KnobSmall   = "png-001";
inline constexpr CString Png_KnobBig     = "png-002";
inline constexpr CString Png_TlistNone   = "png-003";
inline constexpr CString Png_ContBack    = "png-004";
inline constexpr CString Png_HeadCover   = "png-005";
inline constexpr CString Png_Title       = "png-006";
inline constexpr CString Png_MetreH      = "png-007";
inline constexpr CString Png_Tabs        = "png-008";
inline constexpr CString Png_TabOpts     = "png-009";
inline constexpr CString Png_CheckBox    = "png-010";
inline constexpr CString Png_CossinAbout = "png-011";
inline constexpr CString Png_AlertIcons  = "png-012";
inline constexpr CString Png_Play        = "png-013";
inline constexpr CString Png_PanLaw      = "png-019";
inline constexpr CString Png_Lock        = "png-020";
inline constexpr CString Png_Add         = "png-021";
inline constexpr CString Png_ViewModel   = "png-022";
inline constexpr CString Png_ListEntry   = "png-023";
inline constexpr CString Png_FxIconx32   = "png-024";


/** Colour dictionary */
inline constexpr CString Col_ContainerBg   = "container_bg";
inline constexpr CString Col_ContainerFg   = "container_fg";
inline constexpr CString Col_ComponentBg   = "component_bg";
inline constexpr CString Col_ComponentFg   = "component_fg";
inline constexpr CString Col_Font          = "font";
inline constexpr CString Col_HeaderBg      = "header_bg";
inline constexpr CString Col_TooltipBorder = "tooltip_border";
inline constexpr CString Col_TooltipBg     = "tooltip_bg";
inline constexpr CString Col_TooltipFont   = "tooltip_font";
inline constexpr CString Col_Format0       = "font_colour_0";
inline constexpr CString Col_Format1       = "font_colour_1";
inline constexpr CString Col_Format2       = "font_colour_2";
inline constexpr CString Col_Format3       = "font_colour_3";
inline constexpr CString Col_Format4       = "font_colour_4";
inline constexpr CString Col_Format5       = "font_colour_5";
inline constexpr CString Col_Format6       = "font_colour_6";
inline constexpr CString Col_Format7       = "font_colour_7";
inline constexpr CString Col_Format8       = "font_colour_8";
inline constexpr CString Col_Format9       = "font_colour_9";
inline constexpr CString Col_FormatA       = "font_colour_a";
inline constexpr CString Col_FormatB       = "font_colour_b";
inline constexpr CString Col_FormatC       = "font_colour_c";
inline constexpr CString Col_FormatD       = "font_colour_d";
inline constexpr CString Col_FormatE       = "font_colour_e";
inline constexpr CString Col_FormatF       = "font_colour_f";


/** Config dictionary */
inline constexpr CString Cfg_General      = "general";
inline constexpr CString Cfg_Themes       = "themes";
inline constexpr CString Cfg_Defaults     = "defaults";
inline constexpr CString Cfg_Optimization = "optimization";
inline constexpr CString Cfg_Standalone   = "standalone";


/** Config properties */
// General
inline constexpr CString Prop_GeneralTheme    = "theme";
inline constexpr CString Prop_GeneralLanguage = "language";

// Defaults
inline constexpr CString Prop_DefaultsSize        = "size";
inline constexpr CString Prop_DefaultsSizeWidth   = "width";
inline constexpr CString Prop_DefaultsSizeHeight  = "height";
inline constexpr CString Prop_DefaultsPanningMode = "panningMode";
inline constexpr CString Prop_DefaultsProcessMode = "processMode";

// Optimisation
inline constexpr CString Prop_OptHardwareAcceleration = "hardwareAcceleration";
inline constexpr CString Prop_OptMultisampling        = "useMultisampling";
inline constexpr CString Prop_OptTextureSmoothing     = "textureSmoothing";
inline constexpr CString Prop_OptAnimations           = "animations";
inline constexpr CString Prop_OptAnimationsMode       = "mode";
inline constexpr CString Prop_OptAnimationsCustom     = "custom";
inline constexpr CString Prop_OptAnimationsEffects    = "effects";
inline constexpr CString Prop_OptAnimationsComponents = "components";

// Standalone
inline constexpr CString Prop_StandaloneBufferSize      = "bufferSize";
inline constexpr CString Prop_StandaloneSampleRate      = "sampleRate";
inline constexpr CString Prop_StandaloneMuteInput       = "muteInput";
inline constexpr CString Prop_StandaloneDeviceType      = "deviceType";
inline constexpr CString Prop_StandaloneLogToFile       = "logToFile";
inline constexpr CString Prop_StandaloneDoublePrecision = "useDoublePrecisionProcessing";
inline constexpr CString Prop_StandaloneDevices         = "devices";
inline constexpr CString Prop_StandaloneDevicesInput    = "input";
inline constexpr CString Prop_StandaloneDevicesOutput   = "output";


/** Pan Modes */
inline constexpr JAUT_DECLARE_AUTOMATIC_STD_ARRAY(List_PanningModes,
    "Linear",
    "Square",
    "Sinusoidal"
);

/** Processors */
inline constexpr JAUT_DECLARE_AUTOMATIC_STD_ARRAY(List_ProcessModes,
    "Solo"/*,
    "Stack",
    "Graph"
    */
);
}
