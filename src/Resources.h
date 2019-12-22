/**
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
    
 */

#pragma once

#if JUCE_OPENGL
  #define COSSIN_USE_OPENGL 1
#endif

namespace res
{
/**
 * App info
 */
inline constexpr const char *App_Name    = "Cossin";
inline constexpr const char *App_Version = "1.0.0 Alpha";
inline constexpr const char *App_Author  = "Elanda";
inline constexpr const char *App_Vendor  = "ElandaSunshine";
inline constexpr const char *App_Website = "iamelsa.xyz";
inline constexpr const char *App_License = "GPL v3";


/**
 * Image dictionary
 */
inline constexpr const char *Png_Empty        = "png-000";
inline constexpr const char *Png_Knob_Small   = "png-001";
inline constexpr const char *Png_Knob_Big     = "png-002";
inline constexpr const char *Png_Tlist_None   = "png-003";
inline constexpr const char *Png_Cont_Back    = "png-004";
inline constexpr const char *Png_Head_Cover   = "png-005";
inline constexpr const char *Png_Title        = "png-006";
inline constexpr const char *Png_Metre_H      = "png-007";
inline constexpr const char *Png_Tabs         = "png-008";
inline constexpr const char *Png_Tab_Opts     = "png-009";
inline constexpr const char *Png_Check_Box    = "png-010";
inline constexpr const char *Png_Cossin_About = "png-011";
inline constexpr const char *Png_Alert_Icons  = "png-012";
inline constexpr const char *Png_Play         = "png-013";
inline constexpr const char *Png_Pan_Law      = "png-019";
inline constexpr const char *Png_Lock         = "png-020";
inline constexpr const char *Png_Add          = "png-021";
inline constexpr const char *Png_View_Model   = "png-022";
inline constexpr const char *Png_List_Entry   = "png-023";
inline constexpr const char *Png_Fx_Icon_x32  = "png-024";


/**
 * Colour dictionary
 */
inline constexpr const char *Col_Container_Bg   = "container_bg";
inline constexpr const char *Col_Container_Fg   = "container_fg";
inline constexpr const char *Col_Component_Bg   = "component_bg";
inline constexpr const char *Col_Component_Fg   = "component_fg";
inline constexpr const char *Col_Font           = "font";
inline constexpr const char *Col_Header_Bg      = "header_bg";
inline constexpr const char *Col_Tooltip_Border = "tooltip_border";
inline constexpr const char *Col_Tooltip_Bg     = "tooltip_bg";
inline constexpr const char *Col_Tooltip_Font   = "tooltip_font";
inline constexpr const char *Col_Format_0       = "font_colour_0";
inline constexpr const char *Col_Format_1       = "font_colour_1";
inline constexpr const char *Col_Format_2       = "font_colour_2";
inline constexpr const char *Col_Format_3       = "font_colour_3";
inline constexpr const char *Col_Format_4       = "font_colour_4";
inline constexpr const char *Col_Format_5       = "font_colour_5";
inline constexpr const char *Col_Format_6       = "font_colour_6";
inline constexpr const char *Col_Format_7       = "font_colour_7";
inline constexpr const char *Col_Format_8       = "font_colour_8";
inline constexpr const char *Col_Format_9       = "font_colour_9";
inline constexpr const char *Col_Format_a       = "font_colour_a";
inline constexpr const char *Col_Format_b       = "font_colour_b";
inline constexpr const char *Col_Format_c       = "font_colour_c";
inline constexpr const char *Col_Format_d       = "font_colour_d";
inline constexpr const char *Col_Format_e       = "font_colour_e";
inline constexpr const char *Col_Format_f       = "font_colour_f";


/**
 * Config dictionary
 */
inline constexpr const char *Cfg_General      = "general";
inline constexpr const char *Cfg_Defaults     = "defaults";
inline constexpr const char *Cfg_Optimization = "optimization";
inline constexpr const char *Cfg_Standalone   = "standalone";


/**
 * Panning Modes
 */
inline constexpr const char *Pan_Modes[] =
{
    "Linear",
    "Square",
    "Sinusoidal"
};
inline constexpr int Pan_Modes_Num = sizeof(Pan_Modes) / sizeof(Pan_Modes[0]);


/**
 * Processors
 */
inline constexpr const char *Proc_Types[] =
{
    "Solo"/*,
    "Stack",
    "Graph"
    */
};
inline constexpr int Proc_Types_Num = sizeof(Proc_Types) / sizeof(Proc_Types[0]);
}
