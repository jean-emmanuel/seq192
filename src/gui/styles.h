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
const double c_min_zoom = 0.35;
const int c_max_zoom = 16;
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

#endif
