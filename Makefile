CXX=clang++
CXXFLAGS=-ggdb -std=c++11 -I/usr/local/boost_1_53_0 -Iinclude
LDFLAGS=-L/usr/local/boost_1_53_0/stage/lib
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(addprefix obj/,$(notdir $(SOURCES:.cpp=.o)))
TEST_EXECUTABLE=test

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
all: $(SOURCES) $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@

clean:
	/bin/rm -fr obj test make_bfs
