UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    CXX=g++
    CXXFLAGS_FUSE=-I/usr/local/include/fuse  -DFUSE_USE_VERSION=26
    LDFLAGS=-L/usr/local/lib -lboost_filesystem -lboost_system -lboost_program_options -lfuse -lcrypto
else
    CXX=clang++
    CXXFLAGS_FUSE=-I/usr/local/include/osxfuse  -DFUSE_USE_VERSION=26
    LDFLAGS=-L/usr/local/lib -lboost_filesystem -lboost_system -lboost_program_options -losxfuse -lcrypto
endif
CXXFLAGS=-std=c++11 -Os -ffast-math -funroll-loops -Wno-ctor-dtor-privacy -I/usr/local/include/boost -Iinclude -D_FILE_OFFSET_BITS=64
SOURCES := $(wildcard src/teasafe/*.cpp)
MAKE_TeaSafe_SRC := $(wildcard src/maketeasafe/*.cpp)
TEST_SRC := $(wildcard src/test/*.cpp)
FUSE_SRC := $(wildcard src/fuse/*.cpp)
CIPHER_SRC := $(wildcard src/cipher/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS_UTIL := $(addprefix obj-maketeasafe/,$(notdir $(MAKE_TeaSafe_SRC:.cpp=.o)))
OBJECTS_TEST := $(addprefix obj-test/,$(notdir $(TEST_SRC:.cpp=.o)))
OBJECTS_FUSE := $(addprefix obj-fuse/,$(notdir $(FUSE_SRC:.cpp=.o)))
OBJECTS_CIPHER := $(addprefix obj-cipher/,$(notdir $(CIPHER_SRC:.cpp=.o)))
TEST_EXECUTABLE=test
MAKETeaSafe_EXECUTABLE=maketeasafe
FUSE_LAYER=teasafe

obj/%.o: src/teasafe/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
obj-test/%.o: src/test/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
obj-maketeasafe/%.o: src/maketeasafe/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
obj-fuse/%.o: src/fuse/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_FUSE) -c -o $@ $<
	
obj-cipher/%.o: src/cipher/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_FUSE) -c -o $@ $<

all: $(SOURCES) $(TEST_SRC) $(CIPHER_SRC) $(TEST_EXECUTABLE) $(FUSE_LAYER) $(MAKETeaSafe_EXECUTABLE)

$(TEST_EXECUTABLE): directoryObj directoryObjTest directoryObjCipher $(OBJECTS) $(OBJECTS_TEST) $(OBJECTS_CIPHER)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_TEST) $(OBJECTS_CIPHER) -o $@ 
	
$(MAKETeaSafe_EXECUTABLE): directoryObj directoryObjMakeBfs directoryObjCipher $(OBJECTS) $(OBJECTS_UTIL) $(OBJECTS_CIPHER)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_UTIL) $(OBJECTS_CIPHER) -o $@

$(FUSE_LAYER): directoryObj directoryObjFuse directoryObjCipher $(OBJECTS) $(OBJECTS_FUSE) $(OBJECTS_CIPHER)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_FUSE) $(OBJECTS_CIPHER) -o $@
	
clean:
	/bin/rm -fr obj obj-maketeasafe obj-test obj-fuse test maketeasafe teasafe obj-cipher
	
directoryObj: 
	/bin/mkdir -p obj
	
directoryObjTest: 
	/bin/mkdir -p obj-test
	
directoryObjMakeBfs: 
	/bin/mkdir -p obj-maketeasafe
	
directoryObjFuse:
	/bin/mkdir -p obj-fuse
	
directoryObjCipher:
	/bin/mkdir -p obj-cipher
