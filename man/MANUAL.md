seq192(1) -- live MIDI sequencer
=============================

## SYNOPSIS

`seq192` [OPTION...]

## DESCRIPTION

seq192 is a MIDI sequencer for live situations that is meant to be be controlled with OSC messages and sync with other applications using jack transport.

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

*TODO*

## JACK TRANSPORT

When `--jack-transport` is set, seq192 will

    - follow start / stop commands from other clients
    - send start / stop commands to other clients
    - use the transport master's bpm
    - set its position to 0 whenever the transport stops or restarts
    - **not** attempt to reposition within sequences


## CONFIGURATION FILE

The configration file is located in `$XDG_CONFIG_HOME/seq192/config.json` (`~/.config/seq192/config.json` by default), but can be loaded from any location using `--config`. It allows customizing the following aspects of seq192:

    - MIDI bus names
    - MIDI channel names per bus
    - Note names in the piano roll (per channel)
    - Control names in the event dropdown (per channel)

**Example**

<pre>
{
    "buses": {
        "0": {
            "name": "Sampler",
            "channels": {
                "0": {
                    "name": "Drums",
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
            "name": "Bass synth",
            "channels":{
                "0": {"name": "Trap bass"},
                "1": {"name": "Wobble"}
            }
        }
    }
}
</pre>

## OSC CONTROLS

* `/play`:
    Start playback or restart if already playing

* `/stop`:
    Stop playback

* `/screenset` <int: screen>:
    Change active screen set

* `/panic`:
    Disable all sequences and cancel queued sequences

* `/sequence` <string: mode> <int: column> <int: row>:
    Set sequence(s) state<br/>
    _mode_: "solo", "on", "off" or "toggle"<br/>
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
            "timesPlayed": <int>
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


## AUTHORS

seq192 is written by Jean-Emmanuel Doucet and based on

* seq24, written by:
    Rob C. Buse, Ivan Hernandez, Guido Scholz, Dana Olson, Jaakko Sipari,
    Peter Leigh, Anthony Green, Daniel Ellis, Sebastien Alaiwan, Kevin Meinert,
    Andrea delle Canne
* seq32, written by:
    Stazed



## COPYRIGHT

Copyright © 2021 Jean-Emmanuel Doucet <jean-emmanuel@ammd.net>

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

<style type='text/css' media='all'>
/* style: toc */
.man-navigation {display:block !important;position:fixed;top:0;left:113ex;height:100%;width:100%;padding:48px 0 0 0;border-left:1px solid #dbdbdb;background:#eee}
.man-navigation a,.man-navigation a:hover,.man-navigation a:link,.man-navigation a:visited {display:block;margin:0;padding:5px 2px 5px 30px;color:#999;text-decoration:none}
.man-navigation a:hover {color:#111;text-decoration:underline}
</style>
