# seq192

WORK IN PROGRESS

MIDI sequencer based on seq24 with less features and more swag.

**Less features**
- No song editor
- No keyboard controls  
- No midi controls
- Linux only

**More swag**
- Interface rewritten with GTK3
- OSC controls
- almost 192 patterns per set

## Build

**Dependencies** (as debian packages)
```
libjack-jackd2-dev liblo-dev libgtkmm-3.0-dev
```

**Build**
```
make clean && make -j8
```

**Run**

```
./src/seq192
```
