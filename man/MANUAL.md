seq192(1) -- live MIDI sequencer
=============================

## SYNOPSIS

`seq192` [OPTION...]

## DESCRIPTION

seq192 is a MIDI sequencer for live situations that is meant to be controlled with OSC messages and sync with other applications using jack transport.

## OPTIONS

* `-h, --help`:
    Show available options

* `-f, --file` <file>:
    Load midi file on startup

* `-c, --config` <file>:
    Load config file on startup

* `-p, --osc-port` <port>:
    OSC input port (udp port number or unix socket path)

* `-j, --jack-transport`:
    Sync to jack transport

* `-n, --no-gui`:
    Enable headless mode

* `-v, --version`:
    Show version and exit

## USER INTERFACE

Seq192 allows writing and playing MIDI sequences and organizing them in screensets.

**Main window**

The main window consist in a toolbar and a sequence grid.

* The **toolbar** contains the following controls

    *Panic button*: disable all sequences<br/>
    *Stop button*: stop transport<br/>
    *Play button*: start or restart transport<br/>
    *Bpm entry*: set beats per minute<br/>
    *Screenset name entry*: set name of current screenset<br/>
    *Screenset number entry*: select current screenset<br/>

* The **sequence grid** is empty by default. Right-clicking in the grid allows creating a new sequence or editing an existing one. Sequences can be rearranged in the grid by dragging them. Left-clicking on a sequence toggles its playing state. Middle-clicking on a sequence opens its edit window. Shift+click defines the sequence as the synchronization reference for queued sequences. Ctrl+click toggles the sequence's queued state. The keyboard's arrow keys can be used to navigate accross sequences.

**Edit window**

The edit window consist in a menu, a toolbar, a pianoroll and an event editor.

* The **toolbar** contains the following controls

    *Sequence name entry*: set name of sequence<br/>
    *Sequence length entries*: set number of beats per measure, beat unit and number of measures in sequence<br/>
    *Snap button*: toggle grid snapping. Holding `alt` allows disabling snapping temporarily<br/>
    *Snap dropdown*: set snapping grid size<br/>
    *Note dropdown*: set note size  <br/>
    *Output dropdown*: set sequence's MIDI port and channel<br/>

* The **pianoroll** responds to the following interactions

    *Left click (piano area)*: send note to sequence's output<br/>
    *Left click*: select note(s)<br/>
    *Left click drag*: create a lasso selection or move selection<br/>
    *Middle click (or ctrl + left click) drag*: change note(s) duration<br/>
    *Right click + left click*: draw new notes<br/>

* The **event editor** allows editing the velocity of notes and adding MIDI events like pitch wheel and control changes to the sequences The button on the left allows selecting which event should be shown on the timeline. The timeline works like in the piano roll, except events dont have a length. The bottom part is for editing the events' values: handles can be dragged individually, or one can draw a line with a drag-and-drop motion to create a linear automation. Scrolling with the mousewheel on the event editor increments the values of all selected events.

**Important note**

Each window has its own undo/redo history:

    - main window: screnset name and sequences (position and content)
    - edit window: MIDI events, sequence name, number of measures and time signature


## JACK TRANSPORT

When `--jack-transport` is set, seq192 will

    - follow start / stop commands from other clients
    - send start / stop commands to other clients
    - use the transport master's bpm
    - set its position to 0 whenever the transport stops or restarts
    - **not** attempt to reposition within sequences


## CONFIGURATION FILE

The configration file is located in `$XDG_CONFIG_HOME/seq192/config.json` (`~/.config/seq192/config.json` by default), but can be loaded from any location using `--config`. It allows customizing the following aspects of seq192:

    - MIDI bus names and portamento settings (see Slide Notes)
    - MIDI channel names per bus
    - Sequence colors as css color strings (per bus or per channel )
    - Note names in the piano roll (per bus or per channel)
    - Control names in the event dropdown (per bus or per channel )

**Example**

<pre>
{
    "buses": {
        "0": {
            "name": "Sampler",
            "channels": {
                "0": {
                    "name": "Drums",
                    "color": "orange",
                    "notes": {
                        "64": "Kick",
                        "65": "Snare",
                        "66": "Hihat"
                    },
                    "controls": {
                        "1": "Custom cc name ",
                        "2": "Etc"
                    }
                }
            }
        },
        "1": {
            "name": "Surge XT",
            "portamento_max_time": 4000,
            "portamento_log_scale": true,
            "channels":{
                "0": {"name": "Trap bass"},
                "1": {"name": "Wobble"}
            }
        }
    }
}
</pre>

## SLIDE NOTES

Slide notes are an attempt to imitate a well-known sequencer's feature. A slide note bends its base note (the highest note that intersects the slide note's beginning) and stops when the base note ends. The duration of the slide note only defines how long it takes to reach the slide note.

In the PianoRoll, notes can be toggled from/to slide mode using the "S" key or from the "Edit" menu.

Slide notes rely on CC5 (Portamento MSB) and CC37 (Portamento LSB) to tell controlled synths their slide duration, this has the following implications
- the synth must have portamento enabled
- the synth should most likely be monophonic although it may work in some polyphonic scenarios
- the synth must bind at least CC5 to its portamento time setting (ideally CC5 and CC37 should be combined to obtain a 14-bit control)
- seq192 must be aware of the synth's maximum portamento time in order to compute correct portamento values

Setting the maximum portamento time in seq192 is done using seq192's config file. Each output bus has a "portamento_max_time" setting (integer, millisecond, defaults to `16383`) and a "portamento_log_scale" (boolean, defaults to `false`).

**Examples**

- `Fluidsynth`: the default portamento settings matches Fluidsynth's portamento implementation. One should add a CC65 with value 127 (enable Portamento) and a CC67 with value 127 (enable Legato to make it monophonic)
- `Surge XT`: one should set "portamento_max_time" to `4000` and "portamento_log_scale" to `true` to desired bus in the config file, bind CC5 to Portamento time and choose a monophonic mode in Surge

## OSC CONTROLS

* `/play`:
    Start playback or restart if already playing

* `/stop`:
    Stop playback

* `/bpm` <float_or_int: bpm>:
    Set bpm

* `/swing` <float_or_int: position>:
    Set swing strength (0: no swing, >0: swing, <0: anti-swing)

* `/swing/reference` <float_or_int: position>:
    Set swing reference (8: 8ths will swing, 16: 16th will swing, etc)

* `/cursor` <float_or_int: position>:
    Set playhead position (affects all sequences). Position is >= 0 and expressed in quarter notes (0 = first beat)

* `/screenset` <int: screen>:
    Change active screen set

* `/panic`:
    Disable all sequences and cancel queued sequences

* `/sequence` <string: mode> <int: column> <int: row>:
    Set sequence(s) state<br/>
    _mode_: "solo", "on", "off", "toggle", "record", "record_on", "record_off", "sync", "clear", "copy", "cut", "paste", "delete"; only one sequence can be recording at a time; "record_off" mode doesn't require any argument<br/>
    _column_: column number on screen set (zero indexed)<br/>
    _row_: row number; if omitted, all rows are affected; multiple rows can be specified


* `/sequence` <string: mode> <string: name>:
    Set sequence(s) state<br/>
    _name_: sequence name or osc pattern (can match multiple sequence names); multiple names can be specified

* `/sequence/queue` <string: mode> <int: column> <int: row>:
    Same as /sequence but affected sequences will change state only on next cycle

* `/sequence/trig` <string: mode> <int: column> <int: row>:
    Same as /sequence and (re)start playback


* `/status` <string: address>:
    Send sequencer's status as json, without sequences informations<br/>
    _address_: *osc.udp://ip:port* or *osc.unix:///path/to/socket* ; if omitted the response will be sent to the sender

* `/status/extended` <string: address>:
    Send sequencer's status as json, including sequences informations<br/>

## OSC STATUS

<pre>
{
    "screenset": <int>,
    "screensetName": "<string>",
    "playing": <int>,
    "bpm": <int>,
    "tick": <int>,
    "sequences": [
        {
            "col": <int>,
            "row": <int>,
            "name": "<string>",
            "time": "<string>",
            "bars": <int>,
            "ticks": <int>,
            "queued": <int>,
            "playing": <int>,
            "timesPlayed": <int>,
            "recording": <int>
        },
        ...
    ]
}
</pre>


**Sequencer status**

    screenset: current screenset
    screensetName: current screenset's name
    playing: playback state
    bpm: current bpm
    tick: playback tick (192 ticks = 1 quarter note)

**Sequences statuses** (1 per active sequence in current screenset)

    col: column position
    row: row position
    name: sequence name
    time: sequence time signature (eg "4/4")
    bars: number of bars in sequence
    ticks: sequence length
    queued: sequence's queued state
    playing: sequence's playing state
    timesPlayed: number of times the sequence played since last enabled
    recording: sequence's recording state


## AUTHORS

seq192 is written by Jean-Emmanuel Doucet and based on

* seq24, written by:
    Rob C. Buse, Ivan Hernandez, Guido Scholz, Dana Olson, Jaakko Sipari,
    Peter Leigh, Anthony Green, Daniel Ellis, Sebastien Alaiwan, Kevin Meinert,
    Andrea delle Canne
* seq32, written by:
    Stazed



## COPYRIGHT

Copyright © 2021-2023 Jean-Emmanuel Doucet <jean-emmanuel@ammd.net>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

## LINKS

Sources: <a href="https://github.com/jean-emmanuel/seq192">https://github.com/jean-emmanuel/seq192</a>

<style type='text/css' media='all'>
/* style: toc */
.man-navigation {display:block !important;position:fixed;top:0;left:113ex;height:100%;width:100%;padding:48px 0 0 0;border-left:1px solid #dbdbdb;background:#eee}
.man-navigation a,.man-navigation a:hover,.man-navigation a:link,.man-navigation a:visited {display:block;margin:0;padding:5px 2px 5px 30px;color:#999;text-decoration:none}
.man-navigation a:hover {color:#111;text-decoration:underline}
</style>
