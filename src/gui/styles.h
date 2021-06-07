#ifndef SEQ24_STYLE
#define SEQ24_STYLE

#include <gtkmm.h>

const Gdk::RGBA c_color_background = Gdk::RGBA("#21252b");
const Gdk::RGBA c_color_foreground = Gdk::RGBA("#42454A");
const Gdk::RGBA c_color_raised = Gdk::RGBA("#55575c");
const Gdk::RGBA c_color_text = Gdk::RGBA("#cccccc");
const Gdk::RGBA c_color_text_hilight = Gdk::RGBA("#eeeeee");

const Gdk::RGBA c_color_primary = Gdk::RGBA("rgb(117, 170, 229)");
const Gdk::RGBA c_color_secondary = Gdk::RGBA("rgb(229, 170, 117)");


struct color { double r; double g; double b; };

// Main window
const std::string c_mainwindow_css = "\
window * {box-shadow: none; border: 0; border-radius: 0; text-shadow: none; color: inherit;}\
window {background: " + c_color_background.to_string() + "; color: " + c_color_text.to_string() + "}\
window menu {background: " + c_color_foreground.to_string() + "; border: 1px solid " + c_color_background.to_string() + "; padding: 1px;}\
window menubar {background: " + c_color_foreground.to_string() + "; border-bottom: 1px solid " + c_color_background.to_string() + "}\
window menuitem:hover {background: " + c_color_primary.to_string() + "; color: " + c_color_text_hilight.to_string() + "}\
window separator {background: " + c_color_background.to_string() + "; opacity: 0.5;}\
window scrollbar {background: " + c_color_background.to_string() + "; min-width: 12px; min-height: 12px;}\
window scrollbar slider {background: " + c_color_raised.to_string() + "; min-width: 10px; min-height: 10px;}\
window scrollbar slider:active {background: " + c_color_primary.to_string() + ";}\
.toolbar {background: " + c_color_foreground.to_string() + ";padding:10px; border-bottom: 1px solid " + c_color_background.to_string() + "}\
window button {background: " + c_color_raised.to_string() + "; box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25);}\
window button.on {color: " + c_color_primary.to_string() + ";}\
window button:hover {opacity: 0.8}\
window button:active {opacity: 0.6}\
window entry {background: rgba(0, 0, 0, 0.15); box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25); color: inherit}\
window entry:focus {background: rgba(0, 0, 0, 0.2);}\
window spinbutton entry {box-shadow: none}\
window spinbutton {box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25); color: inherit;}\
window spinbutton button {box-shadow:none;}\
window spinbutton button:last-child {margin-left:1px;}\
window overshoot, undershoot {background: none;}\
window :disabled {opacity: 0.75}\
window .nomargin {margin-right: -9px;}\
combobox {min-width: 0;}\
";

// toolbar
const int c_toolbar_spacing = 10;


// Sequence grid
const int c_grid_padding = 0;
const int c_grid_spacing = 1;

// Sequence button
const int c_sequence_padding = 4;
const int c_sequence_fontsize = 8;
const char * const c_font = "sans";


const color c_sequence_background = {c_color_foreground.get_red(), c_color_foreground.get_green(), c_color_foreground.get_blue()};
const color c_sequence_background_on = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};

const color c_sequence_text = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};
const color c_sequence_text_on = {c_color_background.get_red(), c_color_background.get_green(), c_color_background.get_blue()};

const color c_sequence_marker = {c_color_primary.get_red(), c_color_primary.get_green(), c_color_primary.get_blue()};
const color c_sequence_marker_on = {c_color_primary.get_red(), c_color_primary.get_green(), c_color_primary.get_blue()};




// PianoKeys
const int c_key_height = 20;
const int c_keys_height = c_key_height * c_num_keys;
const int c_keys_width =  100;
const color c_key_white = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};
const color c_key_black = {c_color_background.get_red(), c_color_background.get_green(), c_color_background.get_blue()};
const int c_key_fontsize = 8;
const int c_key_padding = 4;

// PianoRoll
const int c_default_zoom = 2;
const int c_min_zoom = 1;
const int c_max_zoom = 32;
const color c_color_grid = {1.0, 1.0, 1.0};
const double c_alpha_grid_measure = 0.8;
const double c_alpha_grid_beat = 0.3;
const double c_alpha_grid_snap = 0.1;
const double c_alpha_grid_off = 0.01;
const double c_alpha_grid_key = 0.1;
const color c_color_event = {c_color_primary.get_red(), c_color_primary.get_green(), c_color_primary.get_blue()};
const color c_color_event_selected = {c_color_secondary.get_red(), c_color_secondary.get_green(), c_color_secondary.get_blue()};
const double c_alpha_event = 0.75;

const color c_color_lasso = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};
const double c_alpha_lasso_stroke = 0.8;
const double c_alpha_lasso_fill = 0.2;

// EventRoll
const int c_eventroll_height =  c_key_height;
const int c_event_width =  4;
const double c_alpha_grid_separator = 0.3;

// EventData
const int c_dataroll_height =  150;

#endif
