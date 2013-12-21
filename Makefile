CXX=clang++
CXXFLAGS=-ggdb -std=c++11 -stdlib=libc++ -I/usr/local/include/boost -Iinclude 
LDFLAGS=-L/usr/local/lib -stdlib=libc++ -lboost_filesystem -lboost_system
SOURCES := $(wildcard src/bfs/*.cpp)
MAKE_BFS_SRC := $(wildcard src/makebfs/*.cpp)
TEST_SRC := $(wildcard src/test/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))
OBJECTS_UTIL := $(addprefix obj-makebfs/,$(notdir $(MAKE_BFS_SRC:.cpp=.o)))
OBJECTS_TEST := $(addprefix obj-test/,$(notdir $(TEST_SRC:.cpp=.o)))
TEST_EXECUTABLE=test
MAKEBFS_EXECUTABLE=makebfs

obj/%.o: src/bfs/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
obj-test/%.o: src/test/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
obj-makebfs/%.o: src/makebfs/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
all: $(SOURCES) $(TEST_SRC) $(TEST_EXECUTABLE)

makebfs: $(SOURCES) $(MAKE_BFS_SRC) $(MAKEBFS_EXECUTABLE)

$(TEST_EXECUTABLE): directoryObj directoryObjTest $(OBJECTS) $(OBJECTS_TEST)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_TEST) -o $@
	
$(MAKEBFS_EXECUTABLE): directoryObj directoryObjMakeBfs $(OBJECTS) $(OBJECTS_UTIL)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(OBJECTS_UTIL) -o $@

clean:
	/bin/rm -fr obj obj-makebfs obj-test test makebfs
	
directoryObj: 
	/bin/mkdir -p obj
	
directoryObjTest: 
	/bin/mkdir -p obj-test
	
directoryObjMakeBfs: 
	/bin/mkdir -p obj-makebfs
	
    
    
    
