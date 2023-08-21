# seq192

## Changes by Zig:
### OSC API:
- [X] midi through as a toggle
- [X] Create new clip?
- [X] Increase or decrease the clip's length, see https://github.com/jean-emmanuel/seq192/pull/16/commits/3aed2b02ffa898f5d8ed2ed29dd6a17ccd06c80e
- [X] Change the bus and channel

### 3rd-party app to connect my MIDI controller to Seq192:
Written probably first in Pd, then eventually in Python or mididings:
- [ ] Create new clip, or copy an existing empty clip and past it where I select
- [ ] one single button to create a new clip and activate record through
- [ ] ask for naming the sequence as soon as it is created
- [ ] MIDI leds out to controller


## Upstream info:

MIDI sequencer based on seq24 with less features and more swag.

Read the [Changelog](CHANGELOG.md) for more details.

![Seq192 main window](https://user-images.githubusercontent.com/5261671/215058107-55ec762c-a9c7-488b-aff8-70c26bea93b7.png)

![Seq192 edit window](https://user-images.githubusercontent.com/5261671/215058105-f825167b-2d37-4296-a8d8-da17b280ee66.png)



## Build

**Dependencies**
```
# as debian packages
libjack-jackd2-dev liblo-dev libgtkmm-3.0-dev libasound2-dev nlohmann-json3-dev

# as fedora packages
jack-audio-connection-kit-devel alsa-lib-devel liblo-devel gtkmm30-devel json-devel
```

**Build**
```
make clean && make -j8
```

**Build options**

Jack and Gtk can be stripped out at compile time with `USE_JACK=0` and `USE_GTK=0` options.
```
# build headless and jackless binary
make -j8 USE_JACK=0 USE_GTK=0
```

**Run**

```
usage: ./src/seq192 [options]

options:
  -h, --help              show available options
  -f, --file <filename>   load midi file on startup
  -c, --config <filename> load config file on startup
  -p, --osc-port <port>   osc input port (udp port number or unix socket path)
  -j, --jack-transport    sync to jack transport
  -n, --no-gui            enable headless mode
  -v, --version           show version and exit
```

**Install**

```bash
sudo make install
```

Append `PREFIX=/usr` to override the default installation path (`/usr/local`)

**Uninstall**

```bash
sudo make uninstall
```

Append `PREFIX=/usr` to override the default uninstallation path (`/usr/local`)

## Documentation

See [seq192.ammd.net](https://seq192.ammd.net/) or run `man seq192` after installing.

## Web UI

A proof-of-concept web UI built with [Open Stage Control](https://openstagecontrol.ammd.net/) is available at https://github.com/jean-emmanuel/seq192-control
