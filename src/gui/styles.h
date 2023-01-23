// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#ifndef SEQ192_STYLE
#define SEQ192_STYLE

#include <gtkmm.h>
#include "../core/globals.h"

const Gdk::RGBA c_color_background = Gdk::RGBA("#21252b");
const Gdk::RGBA c_color_foreground = Gdk::RGBA("#42454A");
const Gdk::RGBA c_color_raised = Gdk::RGBA("#55575c");
const Gdk::RGBA c_color_text = Gdk::RGBA("#cccccc");
const Gdk::RGBA c_color_text_hilight = Gdk::RGBA("#ffffff");

const Gdk::RGBA c_color_primary = Gdk::RGBA("rgb(117, 170, 229)");
const Gdk::RGBA c_color_secondary = Gdk::RGBA("rgb(229, 170, 117)");
const Gdk::RGBA c_color_tertiary = Gdk::RGBA("rgb(154, 117, 229)");


struct color { double r; double g; double b; };

// toolbar
const int c_toolbar_spacing = 10;


// Sequence grid
const int c_grid_padding = 0;
const int c_grid_spacing = 1;

// Sequence button
const int c_sequence_padding = 4;
const int c_sequence_fontsize = 8;
const int c_sequence_signature_size = 6;

const char * const c_font = "sans";

extern color global_user_instrument_colors[c_max_instruments];
const color c_sequence_default_instrument_color = {0.4, 0.4, 0.4};

const color c_sequence_background = {c_color_foreground.get_red(), c_color_foreground.get_green(), c_color_foreground.get_blue()};
const color c_sequence_background_on = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};

const color c_sequence_syncref = {c_color_secondary.get_red(), c_color_secondary.get_green(), c_color_secondary.get_blue()};
const color c_sequence_syncref_bg = {c_color_background.get_red(), c_color_background.get_green(), c_color_background.get_blue()};

const color c_sequence_text = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};
const color c_sequence_text_on = {c_color_background.get_red(), c_color_background.get_green(), c_color_background.get_blue()};
const color c_sequence_text_record = {1.0, 0.0, 0.0};

const color c_sequence_marker = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};
const color c_sequence_marker_on = {c_color_background.get_red(), c_color_background.get_green(), c_color_background.get_blue()};



// PianoKeys
const int c_key_height = 20;
const int c_keys_height = c_key_height * c_num_keys;
const int c_keys_width =  100;
const color c_key_white = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};
const color c_key_black = {c_color_background.get_red(), c_color_background.get_green(), c_color_background.get_blue()};
const int c_key_fontsize = 8;
const int c_key_padding = 4;

// TimeRoll

const int c_timeroll_height = c_key_height;

// PianoRoll
const int c_default_zoom = 2;
const int c_disabled_snap = c_min_note_length;
const int c_default_snap = c_ppqn / 4;
const double c_min_zoom = 0.5;
const int c_max_zoom = 32;
const color c_color_grid = {1.0, 1.0, 1.0};
const double c_alpha_grid_measure = 0.8;
const double c_alpha_grid_beat = 0.3;
const double c_alpha_grid_snap = 0.1;
const double c_alpha_grid_off = 0.01;
const double c_alpha_grid_key = 0.1;
const color c_color_event = {c_color_primary.get_red(), c_color_primary.get_green(), c_color_primary.get_blue()};
const color c_color_event_selected = {c_color_secondary.get_red(), c_color_secondary.get_green(), c_color_secondary.get_blue()};
const color c_color_event_alt = {c_color_tertiary.get_red(), c_color_tertiary.get_green(), c_color_tertiary.get_blue()};
const double c_alpha_event = 0.75;
const double c_alpha_event_alt = 0.5;

const color c_color_lasso = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};
const double c_alpha_lasso_stroke = 0.8;
const double c_alpha_lasso_fill = 0.2;

// EventRoll
const int c_eventroll_height =  c_key_height;
const int c_event_width =  3;
const double c_alpha_grid_separator = 0.3;

// DataRoll
const int c_dataroll_height =  150;
const int c_dataroll_padding =  4;
const int c_data_handle_radius = 6;
const int c_data_text_height = 35;
const int c_data_y0 = c_dataroll_padding + c_data_handle_radius;
const int c_data_y1 = c_dataroll_height - c_dataroll_padding - c_data_handle_radius - c_data_text_height;
const int c_data_text_width = 6;
const double c_alpha_handle = 0.25;
const double c_alpha_bottom_line = 0.25;
const color c_color_data_background = {c_color_background.get_red(), c_color_background.get_green(), c_color_background.get_blue()};

// Main window
const std::string c_mainwindow_css = "\
@define-color color_bg " + c_color_background.to_string() + ";\
@define-color color_fg " + c_color_foreground.to_string() + ";\
@define-color color_raised " + c_color_raised.to_string() + ";\
@define-color color_text " + c_color_text.to_string() + ";\
@define-color color_text_hilight " + c_color_text_hilight.to_string() + ";\
@define-color color_primary " + c_color_primary.to_string() + ";\
@define-color color_secondary " + c_color_secondary.to_string() + ";\
" + R"CSS(

/* reset */
*:not(decoration), window * {
    box-shadow: none;
    border: 0;
    border-radius: 0;
    text-shadow: none;
    color: inherit;
}

/* window */

window {
    background: @color_bg;
    color: @color_text
}

/* menus */

menu {
    background: @color_fg;
    border: 1px solid @color_bg;
    padding: 1px;
}

menu window arrow:not(.right) {
    background: @color_fg;
}

menubar {
    background: @color_fg;
    border-bottom: 1px solid @color_bg
}

menuitem:hover {
    background: @color_primary;
    color: @color_text_hilight
}

separator {
    background: @color_bg;
    opacity: 0.5;
}

menubar > menuitem {
    margin-left: 1px;
}

menuitem.recording {
    box-shadow: 0 -2px 0 0 rgba(255, 0, 0, 0.5) inset;
}

menuitem.playing {
    box-shadow: 0 -2px 0 0 rgba(255, 255, 255, 0.5) inset;
}

/* tooltips */

popover, tooltip {
    background: @color_fg;
    border: 1px solid @color_bg;
}

tooltip {
    opacity: 0.9;
}

/* scrollbars */

scrollbar {
    background: @color_bg;
    min-width: 12px;
    min-height: 12px;
}

scrollbar slider {
    background: @color_raised;
    min-width: 10px;
    min-height: 10px;
}

scrollbar slider:active {
    background: @color_primary;
}

overshoot, undershoot {
    background: none;
}

/* toolbar */

.toolbar {
    background: @color_fg;
    padding: 10px;
    border-bottom: 1px solid @color_bg
}

#sset .down, #sset .up {
-gtk-icon-transform: scale(0);
background: -gtk-icontheme("pan-start-symbolic") center no-repeat, @color_raised;
background-size: 20px;
}

#sset .up {
background-image: -gtk-icontheme("pan-end-symbolic");
}

/* widgets */

button {
    background: @color_raised;
    box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25);
    min-width: 20px;
    padding: 2px 10px;
}

button:hover {
    opacity: 0.8
}

button:active {
    opacity: 0.6
}

.togglebutton:checked, button.on {
    color: @color_primary;
}

button.bypass.on,
.togglebutton.bypass:checked {
    color: @color_text;
}

entry {
    background: rgba(0, 0, 0, 0.15);
    box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25);
    color: inherit
}

entry:focus {
    background: rgba(0, 0, 0, 0.2);
}

selection {
    background: @color_primary;
    color: @color_text_hilight
}

spinbutton entry {
    box-shadow: none
}

spinbutton {
    box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25);
    color: inherit;
    background: transparent
}

spinbutton button {
    box-shadow: none;
    padding: 2px 6px;
}

spinbutton button:last-child {
    margin-left: 1px;
}

combobox {
    min-width: 0;
}

combobox window arrow {
    background: @color_fg;
}

check {
    background: transparent;
    -gtk-icon-source: none;
    border: 1px solid @color_text;
    background-clip: content-box;
    min-height: 12px;
    min-width: 12px;
}

check:hover {
    border-color: @color_text_hilight
}

check:checked, .checked check {
    background: @color_text;
    box-shadow: inset 0 0 0 1px;
    color: @color_fg;
}

check:checked:hover, .checked check:hover {
    background: @color_text_hilight;
    color: @color_primary;
}

:disabled {
    opacity: 0.75
}

.nomargin {
    margin-right: -9px;
}

/* edit window */

.editwindow-vscrollbar {
    padding-top: 2px;
    padding-bottom: 2px;
}

.editwindow-hscrollbar {
    padding-top: 2px;
    opacity: 0;
    transition: none;
}

.editwindow-hscrollbar.show {
    opacity: 1;
}

.editwindow-eventbutton {
    font-size: 8pt;
    font-weight: bold;
    box-shadow: none;
    padding-right: 4px;
    min-height: 0;
    border-top: 1px solid @color_bg;
    border-right: 1px solid @color_bg;
}

/* dialogs */

dialog {
    background: @color_fg;
    color: @color_text;
    box-shadow: 0 0 0 1px @color_bg;
}

messagedialog {
    box-shadow: 0 0 0 1px @color_bg;
    background: @color_fg;
    color: @color_text;
}

messagedialog .titlebar {
    background: @color_fg;
    box-shadow: 0 0 0 1px @color_bg, 0 1px 0 0 @color_fg;
}

messagedialog button {
    padding: 8px;
}

dialog headerbar {
    background: @color_fg;
    box-shadow: 0 0 0 1px @color_bg;
    padding: 0 10px;
}

dialog toolbar,
dialog treeview,
dialog #pathbarbox > stack,
dialog placesview list,
filechooser box paned box box stack,
dialog actionbar > revealer > box,
#pathbarbox {
    background: @color_fg
}

dialog actionbar > revealer > box {
    border-top: 1px solid @color_bg
}

dialog #pathbarbox {
    border-bottom: 1px solid @color_bg
}

dialog scrollbar {
    padding-top: 2px;
    padding-bottom: 2px
}

dialog placessidebar {
    background: @color_bg
}

dialog :selected,
dialog treeview :selected,
dialog placessidebar :active,
dialog modelbutton:hover {
    background: @color_primary;
    color: @color_text_hilight;
    opacity: 1;
}

)CSS";

#endif
