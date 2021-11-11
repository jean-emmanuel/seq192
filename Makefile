CXX = g++
CXXFLAGS = -g -O0 $(shell pkg-config --cflags liblo jack alsa gtkmm-3.0) -Wall
LDFLAGS = $(shell pkg-config --libs liblo jack alsa gtkmm-3.0)
SOURCES = $(wildcard src/core/*.cpp) $(wildcard src/gui/*.cpp) src/seq192.cpp
OBJ = $(SOURCES:.cpp=.o)
DEPENDS := $(SOURCES:.cpp=.d)
BIN = seq192
PREFIX = /usr/local

.PHONY: all clean install uninstall

all: src/$(BIN)

bold := $(shell tput bold)
sgr0 := $(shell tput sgr0)

src/$(BIN): $(OBJ)
	@printf '\n$(bold)Linking$(sgr0)\n'
	$(CXX) -o $@ $^ $(LDFLAGS)
	@printf '\n'

%.o: %.cpp Makefile
	@printf '\n$(bold)Compilation from $< to $@ $(sgr0)\n'
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDS)

manual:
	ronn man/MANUAL.md --manual='User manual' --roff
	mv man/MANUAL.1 man/seq192.1

manual-html:
	ronn man/MANUAL.md --manual='User manual' --html

clean:
	@rm -f $(OBJ) $(DEPENDS) src/$(BIN)

install: src/$(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/pixmaps
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	mkdir -p $(DESTDIR)$(PREFIX)/share/man/man1
	cp $< $(DESTDIR)$(PREFIX)/bin/$(BIN)
	cp src/xpm/seq192_32.xpm $(DESTDIR)$(PREFIX)/share/pixmaps/seq192.xpm
	cp desktop/seq192.desktop $(DESTDIR)$(PREFIX)/share/applications/seq192.desktop
	cp man/seq192.1 $(DESTDIR)$(PREFIX)/share/man/man1/seq192.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/seq192.xpm
	rm -f $(DESTDIR)$(PREFIX)/share/applications/seq192.desktop
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/seq192.1
