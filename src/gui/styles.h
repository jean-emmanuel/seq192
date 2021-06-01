#ifndef SEQ24_STYLE
#define SEQ24_STYLE

#include <gtkmm.h>

const Gdk::RGBA c_color_background = Gdk::RGBA("#21252b");
const Gdk::RGBA c_color_text = Gdk::RGBA("#cccccc");

const Gdk::RGBA c_color_primary = Gdk::RGBA("rgb(117, 170, 229)");


struct color { double r; double g; double b; };

// Main window
const std::string c_mainwindow_css = "\
.mainwindow, .editwindow {background: " + c_color_background.to_string() + "; color: " + c_color_text.to_string() + "}\
.toolbar {box-shadow: inset 0 -1px 1px 0 rgba(0, 0, 0, 0.25)}\
.toolbar {background: rgb(66, 69, 74);padding:10px;}\
.toolbar > button, .toolbar > spinbutton button {border: 0; border-radius: 0; background: rgba(255, 255, 255, 0.1); box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25); color: inherit}\
.toolbar > entry {border: 0; border-radius: 0; background: rgba(0, 0, 0, 0.2); box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25); color: inherit}\
.toolbar > spinbutton entry {border: 0; border-radius: 0; background: rgba(0, 0, 0, 0.2);color: inherit; box-shadow: none}\
.toolbar > spinbutton {background:transparent; border: 0; border-radius: 0; box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25); color: inherit;}\
.toolbar > spinbutton button {box-shadow:none;}\
.toolbar > spinbutton button:last-child {margin-left:1px;}\
.panic, .play, .stop {font-size: 16px}\
button.on {color:rgba(117, 170, 229, 1.0);}\
button:hover, spinbutton button:hover {background: rgba(255, 255, 255, 0.2)}\
button:active, spinbutton button:active {opacity: 0.8}\
";

// toolbar
const int c_toolbar_spacing = 10;


// Sequence grid
const int c_grid_padding = 4;
const int c_grid_spacing = 1;

// Sequence button
const int c_sequence_padding = 4;
const int c_sequence_fontsize = 8;
const char * const c_font = "sans";


const color c_sequence_background = {0.26, 0.27, 0.29};
const color c_sequence_background_on = {0.78, 0.78, 0.79};

const color c_sequence_text = {0.9, 0.9, 0.9};
const color c_sequence_text_on = {0.1, 0.1, 0.1};
const color c_sequence_text_faded = {0.6, 0.6, 0.6};
const color c_sequence_text_faded_on = {0.4, 0.4, 0.4};

const color c_sequence_events = {0.7, 0.7, 0.7};
const color c_sequence_events_on = {0.3, 0.3, 0.3};

const color c_sequence_events_background = {0.31, 0.32, 0.34};
const color c_sequence_events_background_on = {0.70, 0.70, 0.71};

const color c_sequence_marker = {0.46, 0.67, 0.90};
const color c_sequence_marker_on = {0.26, 0.47, 0.80};




// PianoKeys / PianoRoll
const int c_key_height = 20;
const color c_key_white = {c_color_text.get_red(), c_color_text.get_green(), c_color_text.get_blue()};
const color c_key_black = {c_color_background.get_red(), c_color_background.get_green(), c_color_background.get_blue()};
const int c_key_fontsize = 8;
const int c_key_padding = 4;
// const color c_key_black = {1.0, 1.0, 1.0, 0.1};
// const color c_pianoroll_lines = {1.0, 1.0, 1.0};




#endif
