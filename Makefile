PKG_CONFIG ?= pkg-config

# discover the liklihood of what version of FUSE we're using
# also set the compiler type; clang if on mac, gcc if on linux
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    CXX=g++
    FUSE=fuse
    LIB_EXT=so
else
    CXX=clang++
    FUSE=osxfuse
    LIB_EXT=dylib
endif
FUSE_LIBS = $(shell $(PKG_CONFIG) --libs fuse 2>/dev/null || echo "-l$(FUSE)")

# standard library search paths
LDFLAGS +=  -L/usr/local/lib -L/usr/lib

BOOST_LD= -lboost_filesystem \
          -lboost_system \
          -lboost_program_options \
          -lboost_random \
          -lboost_regex \
          -lboost_timer

# compilation flags
CXXFLAGS_FUSE= $(shell $(PKG_CONFIG) --cflags fuse 2>/dev/null || echo "-I/usr/local/include/$(FUSE)")  -DFUSE_USE_VERSION=26
CXXFLAGS ?= -O2 \
            -funroll-loops \
            -Wno-ctor-dtor-privacy \
            -Wall \
            -ggdb
CXXFLAGS += -std=c++14 \
            -Icryptostreampp \
            -I/usr/include -I/usr/local/include \
            -Iinclude -D_FILE_OFFSET_BITS=64 \
            -march=native \
            -D STATIC_CRYPTOSTREAMPP_VAR

# specify locations of all source files
SOURCES := $(wildcard src/knoxcrypt/*.cpp)
MAKE_knoxcrypt_SRC := $(wildcard src/makeknoxcrypt/*.cpp)
TEST_SRC := $(wildcard src/test/*.cpp)
FUSE_SRC := $(wildcard src/fuse/*.cpp)
UTILITY_SRC := $(wildcard src/utility/*.cpp)

# specify object locations; they will be dumped in several directories
# obj, obj-makeknoxcrypt, obj-test, obj-fuse and obj-cipher
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS_MAKEBIN := $(addprefix obj-makeknoxcrypt/,$(notdir $(MAKE_knoxcrypt_SRC:.cpp=.o)))
OBJECTS_TEST := $(addprefix obj-test/,$(notdir $(TEST_SRC:.cpp=.o)))
OBJECTS_FUSE := $(addprefix obj-fuse/,$(notdir $(FUSE_SRC:.cpp=.o)))
OBJECTS_UTILITY := $(addprefix obj-utility/,$(notdir $(UTILITY_SRC:.cpp=.o)))

# the executable used for running the test harness
TEST_EXECUTABLE=test_$(UNAME)

# the executable used for creating a knoxcrypt image
MAKEknoxcrypt_EXECUTABLE=makeknoxcrypt_$(UNAME)

# the fuse-layered executable
FUSE_LAYER=knoxcrypt_$(UNAME)

# simple utility programs
SHELL_BIN=teashell_$(UNAME)

# build the different object files
obj/%.o: src/knoxcrypt/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj-test/%.o: src/test/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj-makeknoxcrypt/%.o: src/makeknoxcrypt/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj-utility/%.o: src/utility/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj-fuse/%.o: src/fuse/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_FUSE) -c -o $@ $<

all: $(SOURCES) $(CIPHER_SRC) directoryObj \
     $(OBJECTS) $(OBJECTS_CIPHER) libknoxcrypt.a \
     $(TEST_SRC) $(TEST_EXECUTABLE) $(FUSE_LAYER) $(MAKEknoxcrypt_EXECUTABLE) \
     $(SHELL_BIN)

lib: $(SOURCES) directoryObj $(OBJECTS) libknoxcrypt.a

$(TEST_EXECUTABLE): directoryObjTest $(OBJECTS_TEST) libknoxcrypt.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS_TEST) ./libknoxcrypt.a -lcryptopp $(BOOST_LD) -o $@

$(MAKEknoxcrypt_EXECUTABLE): directoryObjMakeBfs $(OBJECTS_MAKEBIN) libknoxcrypt.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS_MAKEBIN) ./libknoxcrypt.a -lcryptopp $(BOOST_LD) -o $@

$(SHELL_BIN): directoryObjUtility $(OBJECTS_UTILITY) libknoxcrypt.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJECTS_UTILITY) ./libknoxcrypt.a -lcryptopp $(BOOST_LD) -o $@

$(FUSE_LAYER): directoryObjFuse $(OBJECTS_FUSE) libknoxcrypt.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(FUSE_LIBS) $(OBJECTS_FUSE) ./libknoxcrypt.a -lcryptopp $(FUSE_LIBS) $(BOOST_LD) -o $@

shell:  $(SOURCES) directoryObj \
        $(OBJECTS) libknoxcrypt.a \
        $(SHELL_BIN)

makeknoxcrypt: $(SOURCES) directoryObj \
             $(OBJECTS) libknoxcrypt.a \
             $(MAKEknoxcrypt_EXECUTABLE)

clean:
	/bin/rm -fr obj obj-makeknoxcrypt obj-test obj-fuse test_$(UNAME) makeknoxcrypt_$(UNAME) knoxcrypt_$(UNAME) teashell_$(UNAME) obj-utility libknoxcrypt.a

directoryObj:
	/bin/mkdir -p obj

directoryObjTest:
	/bin/mkdir -p obj-test

directoryObjMakeBfs:
	/bin/mkdir -p obj-makeknoxcrypt

directoryObjFuse:
	/bin/mkdir -p obj-fuse

directoryObjUtility:
	/bin/mkdir -p obj-utility

libknoxcrypt.a: $(OBJECTS)
	/usr/bin/ar rcs libknoxcrypt.a obj/*

check: $(TEST_EXECUTABLE)
	./$(TEST_EXECUTABLE)


.PHONY: all check clean lib
