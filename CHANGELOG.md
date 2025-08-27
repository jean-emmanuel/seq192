# Changelog

## 1.8.3

- main window
    - add record and playback submenu to sequence buttons

- main
    - add `-o / --ouptuts` cli option to set the number of output busses

## 1.8.2

- bug fixes
    - more issues when opening midi files from other softwares

- main
    - exporting sequences now use midi format 0 for better interoperability


## 1.8.1

- bug fixes
    - memory error when importing midi files from Musescore

- main
    - allow opening/importing midi file formats 0 (e.g clips from Ardour) and 2

## 1.8.0

- edit window
    - smooth horizontal scrolling & zooming
    - support trackpad horizontal scrolling
    - add `q` shortcut to quantize selection
    - allow setting selected event's value by typing a number
    - add cut to grid (ctrl+u) and join (ctrl+j) note actions

## 1.7.0

- main window
    - append "*" to window title when there are unsaved changes

- edit window
    - add `ctrl+s` shortcut to save the session from this window
    - fix shrinking / moving / pasting accross the sequence's end line
    - pianoroll visual tweaks

- sequence
    - add (FL-ish) slide notes

- config file
    - per bus "portamento_max_time" and "portamento_log_scale" settings for slide notes

## 1.6.1

- bug fixes
    - pitch wheel chasing not working when value LSB is 0 (always the case when set from pianoroll ui)

## 1.6.0

- edit window
    - save and restore grid unit / note length per sequence

- misc
    - now requires liblo 0.32

## 1.5.1

- nsm
  - fix save command in headless mode
  - don't announce optional gui capability in headless mode

## 1.5.0

- edit window
    - add measures and time signature to undo history
    - fix some unconsistent undo/redo behaviors

- misc
    - add osc controllable swing option

## 1.4.0

- main window
    - add undo/redo history for sequence grid and screenset names

- edit window
    - add a toggle button and a keyboard shortcut for switching between draw and select modes
    - fix some regressions with editing actions
    - prevent adding overlapping events when drawing multiple events in a single drag of mouse
    - add shift + click range selection
    - include sequence name changes in undo/redo history

- nsm
    - return correct error code when failing to save

## 1.3.0

- main window
    - add keyboard shortcuts for sequence menu actions
    - add `rename` action to sequence menu
    - add keyboard navigation in the sequence grid and a visual hint on the focused sequence
- edit window
    - when no event is selected, hitting Del or Backspace removes the event under the pointer
    - prevent crash when opening event menu with a config file loaded
    - fix display of user-defined instrument names in bus menu
- osc
    - add `/cursor` method
- sequence
    - synchronize the sequence when /sequence/queue is received, even if wanted playing state is the same than the current one
- misc
    - add `ctrl+space` shortcut to stop playback

## 1.2.0

- main window
    - re-enable mousewheel support on bpm and screenset entries and buttons
- edit window
    - display user-defined bus/channel color under the output menu button
- config file
    - allow setting colors, controls and note names per bus, not only per channel
- misc
    - various style tweaks

## 1.1.0

- main window
    - support user instrument colors (set in config file)
    - display user bus name and instrument name in sequence grid
    - add ctrl+click for queuing, shift+click for chosing sync reference
    - open edit window with middle-click on sequence

- perform
    - add the possibility to choose a sequence as sync reference for queued sequences (allows proper metric changes)

- sequence
    - add simple chasing option for control changes and pitch wheel events when sequence is disabled and when playback restarts : reset their value to 0 (or 8192 for pitch wheel) if the sequence has sent a different value while playing.

- edit window
    - fix upper bound of pitch wheel events: skip step 8064 to reach 8192 when displayed value is 127
    - change pitch wheel event slider's origin
    - chase option (under playback)
    - session not seen as modified when editing sequence
    - fix Aftertouch editing
    - center verticall scroll around present notes when the window opens
    - scroll vertically to follow transposition

- misc
    - new icons
    - seq192 can now be built without gtk (headless only)

## 1.0.0

- removed song editor
- interface rewritten with GTK3
- removed MIDI controls
- removed keyboard controls
- added OSC controls
- dropped support for systems other than GNU/Linux
- removed non-ALSA MIDI support
- starting transport while already playing resets position to 0
- added screenset export
- imported a bunch of features from seq32
- refactored config file system using JSON
- added resume playback mode to sequences
- added NSM support (with "optional-gui" and "dirty" capabilities)
- toggleable alt-control display in edit window
- 182 sequences per screenset
