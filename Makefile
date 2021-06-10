CXX = g++
CXXFLAGS = -g -O0 $(shell pkg-config --cflags liblo jack alsa gtkmm-3.0) -Wall
LDFLAGS = $(shell pkg-config --libs liblo jack alsa  gtkmm-3.0)
SOURCES = $(wildcard src/core/*.cpp) $(wildcard src/gui/*.cpp) src/seq192.cpp
OBJ = $(SOURCES:.cpp=.o)
PROG = src/seq192

all: $(PROG)

$(PROG): $(OBJ)
	$(info Linking)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(info Compilation from $^ to $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $^

clean:
	@rm -f $(OBJ)
	@rm -f $(PROG)
