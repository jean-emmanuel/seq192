# Changelog

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
