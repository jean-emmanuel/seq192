CXX = g++
CXXFLAGS = -g -O0 $(shell pkg-config --cflags liblo jack alsa gtkmm-3.0) -Wall
LDFLAGS = $(shell pkg-config --libs liblo jack alsa gtkmm-3.0)
SOURCES = $(wildcard src/core/*.cpp) $(wildcard src/gui/*.cpp) src/seq192.cpp
OBJ = $(SOURCES:.cpp=.o)
DEPENDS := $(SOURCES:.cpp=.d)
PROG = src/seq192

.PHONY: all clean

all: $(PROG)

bold := $(shell tput bold)
sgr0 := $(shell tput sgr0)

$(PROG): $(OBJ)
	@printf '\n$(bold)Linking$(sgr0)\n'
	$(CXX) -o $@ $^ $(LDFLAGS)
	@printf '\n'

%.o: %.cpp Makefile
	@printf '\n$(bold)Compilation from $< to $@ $(sgr0)\n'
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPENDS)

clean:
	@rm -f $(OBJ) $(DEPENDS) $(PROG)
