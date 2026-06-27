# Makefile for machipc -- Mach-style IPC over UNIX sockets
#
# Usage:
#   make / make all   build the central server and every demo into build/
#   make clean        remove the build/ directory
#
# Binaries produced (in build/):
#   cserver    central / bootstrap server (TCP :3333)
#   send       minimal sender demo        (key 1 -> key 2)
#   recv       minimal receiver demo      (key 2)
#   endserver  sends the shutdown control message to the server
#   shm_send   sender demo using System V shared memory
#   shm_recv   receiver demo using System V shared memory

CXX      ?= g++
CXXFLAGS ?= -Wall -O2
LDLIBS   := -lpthread

BUILD    := build

LIB_SRC  := $(wildcard lib/*.cpp)
LIB_OBJ  := $(patsubst lib/%.cpp,$(BUILD)/%.o,$(LIB_SRC))
HEADERS  := $(wildcard include/*.hpp mach/*.hpp)

BINS := \
	$(BUILD)/cserver \
	$(BUILD)/send \
	$(BUILD)/recv \
	$(BUILD)/endserver \
	$(BUILD)/shm_send \
	$(BUILD)/shm_recv

.PHONY: all clean
all: $(BINS)

# Shared socket-helper library objects.
$(BUILD)/%.o: lib/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Central / bootstrap server.
$(BUILD)/cserver: mach/mach_central_server.cpp $(LIB_OBJ) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) $(LDLIBS) -o $@

# Demos (each is a single translation unit linked against the helper objects).
$(BUILD)/send: send.cpp $(LIB_OBJ) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) $(LDLIBS) -o $@

$(BUILD)/recv: recv.cpp $(LIB_OBJ) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) $(LDLIBS) -o $@

$(BUILD)/endserver: process_endserver.cpp $(LIB_OBJ) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) $(LDLIBS) -o $@

$(BUILD)/shm_send: process1_send.cpp $(LIB_OBJ) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) $(LDLIBS) -o $@

$(BUILD)/shm_recv: process2_recv.cpp $(LIB_OBJ) $(HEADERS) | $(BUILD)
	$(CXX) $(CXXFLAGS) $< $(LIB_OBJ) $(LDLIBS) -o $@

$(BUILD):
	mkdir -p $(BUILD)

clean:
	rm -rf $(BUILD)
