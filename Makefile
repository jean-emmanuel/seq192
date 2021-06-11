CXX = g++
CXXFLAGS = -g -O0 $(shell pkg-config --cflags liblo jack alsa gtkmm-3.0) -Wall
LDFLAGS = $(shell pkg-config --libs liblo jack alsa  gtkmm-3.0)
SOURCES = $(wildcard src/core/*.cpp) $(wildcard src/gui/*.cpp) src/seq192.cpp
OBJ = $(SOURCES:.cpp=.o)
PROG = src/seq192

all: $(PROG)

bold := $(shell tput bold)
sgr0 := $(shell tput sgr0)

$(PROG): $(OBJ)
	@printf '\n$(bold)Linking$(sgr0)\n'
	$(CXX) -o $@ $^ $(LDFLAGS)
	@printf '\n'

%.o: %.cpp
	@printf '\n$(bold)Compilation from $^ to $@ $(sgr0)\n'
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	@rm -f $(OBJ)
	@rm -f $(PROG)
