# Changelog

## 0.12.0

- sequence: (seq32) trigger undo/redo
- sequence: (seq32) trigger value handle undo/redo
- sequence: (seq32) nudge notes with keyboard arrows
- sequence: (seq32) pitch/time menu actions
- sequence: redo on ctrl+y and ctrl+shift+z
- sequence: unselect on ctrl+shift+a
- sequence: fix start/stop keyboard shortcuts
- main: (seq32) global is_modified flag (warning when quitting with unsaved changes)
- main: remove option panel
- midi: remove midi controls, midi clock and sysex passing
- midi: always use alsa ports
- build: drop windows support
- build: drop old gtkmm version
- build: require jack
- misc: remove --priority
- misc: remove lash support
- userfile: apply user bus/channel/control definitions
- sequence: display user channel definition

## 0.11.0

- main: removed song editor
- main:keyboard shortcuts disabled
- main:scrollable main window
- engine: osc controls (see OSC.md)
- sequence: bigger keys in pianoroll
- main: slightly bigger font for small texts
- sequence: prevent name entry from grabbing focus

## 0.10.0

- starting playback when already playing restarts playback
- allow entering arbitrary numbers of beats in sequence editor
