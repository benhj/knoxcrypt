UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
    CXX=g++
    FUSE=fuse
else
    CXX=clang++
    FUSE=osxfuse
endif
BOOST_LIBS=/usr/local/lib
LDFLAGS= -v -L/usr/local/lib $(BOOST_LIBS)/libboost_filesystem.a \
                             $(BOOST_LIBS)/libboost_system.a \
                             $(BOOST_LIBS)/libboost_program_options.a \
                             $(BOOST_LIBS)/libboost_random.a 
CXXFLAGS_FUSE= -v -I/usr/local/include/$(FUSE)  -DFUSE_USE_VERSION=26
CXXFLAGS= -v -ggdb -std=c++11 -Os -ffast-math -funroll-loops -Wno-ctor-dtor-privacy -I/usr/local/include/boost -Iinclude -D_FILE_OFFSET_BITS=64
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

all: $(SOURCES) $(CIPHER_SRC) directoryObj directoryObjCipher $(OBJECTS) $(OBJECTS_CIPHER) libBuilder $(TEST_SRC) $(TEST_EXECUTABLE) $(FUSE_LAYER) $(MAKETeaSafe_EXECUTABLE)

lib: $(SOURCES) $(CIPHER_SRC) directoryObj directoryObjCipher $(OBJECTS) $(OBJECTS_CIPHER) libBuilder 

$(TEST_EXECUTABLE): directoryObjTest $(OBJECTS_TEST)
	$(CXX) $(LDFLAGS) ./libteasafe.a $(OBJECTS_TEST) -o $@ 
	
$(MAKETeaSafe_EXECUTABLE): directoryObjMakeBfs $(OBJECTS_UTIL)
	$(CXX) $(LDFLAGS) ./libteasafe.a $(OBJECTS_UTIL) -o $@

$(FUSE_LAYER): directoryObjFuse $(OBJECTS_FUSE) 
	$(CXX) $(LDFLAGS) ./libteasafe.a -l$(FUSE) $(OBJECTS_FUSE) -o $@
	
clean:
	/bin/rm -fr obj obj-maketeasafe obj-test obj-fuse test maketeasafe teasafe obj-cipher libteasafe.a
	
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

libBuilder:
	/usr/bin/ar rcs libteasafe.a obj/* obj-cipher/*
