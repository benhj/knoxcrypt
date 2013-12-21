CXX=clang++
CXXFLAGS=-ggdb -std=c++11 -stdlib=libc++ -I/usr/local/include/boost -Iinclude
CXXFLAGS_FUSE=-I/usr/local/include/osxfuse  -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26
LDFLAGS=-L/usr/local/lib -stdlib=libc++ -lboost_filesystem -lboost_system -losxfuse
SOURCES := $(wildcard src/bfs/*.cpp)
MAKE_BFS_SRC := $(wildcard src/makebfs/*.cpp)
TEST_SRC := $(wildcard src/test/*.cpp)
FUSE_SRC := $(wildcard src/fuse/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS_UTIL := $(addprefix obj-makebfs/,$(notdir $(MAKE_BFS_SRC:.cpp=.o)))
OBJECTS_TEST := $(addprefix obj-test/,$(notdir $(TEST_SRC:.cpp=.o)))
OBJECTS_FUSE := $(addprefix obj-fuse/,$(notdir $(FUSE_SRC:.cpp=.o)))
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

all: $(SOURCES) $(TEST_SRC) $(TEST_EXECUTABLE)

makebfs: $(SOURCES) $(MAKE_BFS_SRC) $(MAKEBFS_EXECUTABLE)

bfs: $(SOURCES) $(FUSE_SRC) $(FUSE_LAYER)

$(TEST_EXECUTABLE): directoryObj directoryObjTest $(OBJECTS) $(OBJECTS_TEST)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_TEST) -o $@
	
$(MAKEBFS_EXECUTABLE): directoryObj directoryObjMakeBfs $(OBJECTS) $(OBJECTS_UTIL)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_UTIL) -o $@

$(FUSE_LAYER): directoryObj directoryObjFuse $(OBJECTS) $(OBJECTS_FUSE)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_FUSE) -o $@
clean:
	/bin/rm -fr obj obj-makebfs obj-test obj-fuse test makebfs
	
directoryObj: 
	/bin/mkdir -p obj
	
directoryObjTest: 
	/bin/mkdir -p obj-test
	
directoryObjMakeBfs: 
	/bin/mkdir -p obj-makebfs
	
directoryObjFuse:
	/bin/mkdir -p obj-fuse
    
    
    
