## OSC commands

#### /play
Start playback or restart if already playing

#### /stop
Stop playback

#### /screenset <int: screen>
Change active screen set

#### /panic
Disable all sequences and cancel queued sequences

#### /sequence <string: mode> <int: column> <int: row>
Set sequence(s) state
- mode: "solo", "on", "off" or "toggle"
- column: column number on screen set (zero indexed)
- row: row number; if omitted, all rows are affected; multiple rows can be specified

#### /sequence <string: mode> <string: name>
Set sequence(s) state
- name: sequence name or osc pattern (can match multiple sequence names); multiple names can be specified

#### /sequence_queued <string: mode> <int: column> <int: row>
Same as /sequence but sequences will change state only on next cycle

#### /sequence_and_play <string: mode> <int: column> <int: row>
Same as /sequence and (re)start playback


#### /status <string: address>
Send sequencer's status as json, without sequences informations
- address: osc.udp://ip:port ; if omitted the response will be sent to the sender

#### /status/extended <string: address>
Send sequencer's status as json, including sequences informations
- address: osc.udp://ip:port ; if omitted the response will be sent to the sender


## OSC status formatting


```
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
            "on": <int>
        },
        ...
    ]
}
```

*Sequencer status*

- screenset: current screenset
- screensetName: current screenset's name
- playing: playback state
- bpm: current bpm
- tick: playback tick (192 ticks = 1 quarter note)

*Sequences statuses* (1 per active sequence in current screenset)

- col: column position
- row: row position
- name: sequence name
- time: sequence time signature (eg "4/4")
- bars: number of bars in sequence
- ticks: sequence length
- queued: sequence's queued state
- on: sequence's playing state
