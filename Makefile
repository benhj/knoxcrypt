CXX=clang++
CXXFLAGS=-ggdb -std=c++11 -I/usr/local/include/boost -Iinclude -D_FILE_OFFSET_BITS=64
CXXFLAGS_FUSE=-I/usr/local/include/osxfuse  -DFUSE_USE_VERSION=26
LDFLAGS=-L/usr/local/lib -lboost_filesystem -lboost_system -lboost_program_options -losxfuse
SOURCES := $(wildcard src/bfs/*.cpp)
MAKE_BFS_SRC := $(wildcard src/makebfs/*.cpp)
TEST_SRC := $(wildcard src/test/*.cpp)
FUSE_SRC := $(wildcard src/fuse/*.cpp)
CIPHER_SRC := $(wildcard src/cipher/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS_UTIL := $(addprefix obj-makebfs/,$(notdir $(MAKE_BFS_SRC:.cpp=.o)))
OBJECTS_TEST := $(addprefix obj-test/,$(notdir $(TEST_SRC:.cpp=.o)))
OBJECTS_FUSE := $(addprefix obj-fuse/,$(notdir $(FUSE_SRC:.cpp=.o)))
OBJECTS_CIPHER := $(addprefix obj-cipher/,$(notdir $(CIPHER_SRC:.cpp=.o)))
TEST_EXECUTABLE=test
MAKEBFS_EXECUTABLE=makebfs
FUSE_LAYER=bfs

obj/%.o: src/bfs/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
obj-test/%.o: src/test/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
obj-makebfs/%.o: src/makebfs/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
obj-fuse/%.o: src/fuse/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_FUSE) -c -o $@ $<
	
obj-cipher/%.o: src/cipher/%.cpp
	$(CXX) $(CXXFLAGS) $(CXXFLAGS_FUSE) -c -o $@ $<

all: $(SOURCES) $(TEST_SRC) $(CIPHER_SRC) $(TEST_EXECUTABLE) $(FUSE_LAYER) $(MAKEBFS_EXECUTABLE)

$(TEST_EXECUTABLE): directoryObj directoryObjTest directoryObjCipher $(OBJECTS) $(OBJECTS_TEST) $(OBJECTS_CIPHER)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_TEST) $(OBJECTS_CIPHER) -o $@ 
	
$(MAKEBFS_EXECUTABLE): directoryObj directoryObjMakeBfs directoryObjCipher $(OBJECTS) $(OBJECTS_UTIL) $(OBJECTS_CIPHER)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_UTIL) $(OBJECTS_CIPHER) -o $@

$(FUSE_LAYER): directoryObj directoryObjFuse directoryObjCipher $(OBJECTS) $(OBJECTS_FUSE) $(OBJECTS_CIPHER)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_FUSE) $(OBJECTS_CIPHER) -o $@
	
clean:
	/bin/rm -fr obj obj-makebfs obj-test obj-fuse test makebfs bfs obj-cipher
	
directoryObj: 
	/bin/mkdir -p obj
	
directoryObjTest: 
	/bin/mkdir -p obj-test
	
directoryObjMakeBfs: 
	/bin/mkdir -p obj-makebfs
	
directoryObjFuse:
	/bin/mkdir -p obj-fuse
	
directoryObjCipher:
	/bin/mkdir -p obj-cipher
