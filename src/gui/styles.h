#ifndef SEQ24_STYLE
#define SEQ24_STYLE

struct color { float r; float g; float b; };

// Main window
const char * const c_mainwindow_css = ".MainWindow {background: #21252b}";

// Sequence grid
const int c_grid_padding = 4;
const int c_grid_spacing = 1;

// Sequence button
const int c_sequence_padding = 4;
const int c_sequence_fontsize = 8;
const char * const c_sequence_font = "sans";


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


#endif
