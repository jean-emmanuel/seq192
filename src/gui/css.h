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


#ifndef SEQ192_CSS
#define SEQ192_CSS

#include "styles.h"

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

/* widgets */

button {
    background: @color_raised;
    min-width: 20px;
    padding: 2px 8px;
}

button, entry, .buttonbox {
    border: 1px solid mix(@color_fg, @color_bg, 0.5);
    transition: opacity 0.25s, background-color 0.25s;
}

button:hover, .buttonbox.hover {
    opacity: 0.8
}

button:hover image {
    -gtk-icon-effect: none;
}

button:active {
    opacity: 0.6
}

.buttonbox {
    background: @color_raised;
}
.buttonbox button:hover image {
    opacity: 0.8;
}


.buttonbox button {
    background: transparent;
    opacity: 1;
    border: 0;
}

.togglebutton:checked, button.on {
    background: alpha(@color_primary, 0.5);
}

button.bypass.on,
.togglebutton.bypass:checked {
    background: @color_raised;
}

entry {
    background: mix(@color_fg, @color_bg, 0.3);
    color: inherit
}

entry:focus {
    border-color: mix(@color_primary, @color_bg, 0.4);
    background: mix(@color_fg, @color_bg, 0.4);
}

selection {
    background: @color_primary;
    color: @color_text_hilight
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
    margin-right: -11px;
}

.group > :not(:first-child) {
    border-left:0;
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
    padding-right: 3px;
    min-height: 0;
    border:0;
    margin-right: 1px;
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

dialog button {
    border: 0;
    box-shadow: 0 0 0 1px mix(@color_fg, @color_bg, 0.5);
}

)CSS";

#endif
