CXX=clang++
CXXFLAGS=-ggdb -std=c++11 -I/usr/local/boost_1_53_0

BFS_OBJS =  BFSImageStream.o \
            FileBlock.o \
            FileEntry.o \
            FolderEntry.o \
            EntryInfo.o \
            /usr/local/boost_1_53_0/stage/lib/libboost_filesystem.dylib \
            /usr/local/boost_1_53_0/stage/lib/libboost_system.dylib

TEST_OBJS = test.o

MAKE_BFS_OBJS = make_bfs.o

.c.o:
	$(CXX) -c $(CXXFLAGS) -arch x86_64 $*.cpp

all: test

test:  $(TEST_OBJS) $(BFS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) $(BFS_OBJS)

make_bfs:  $(MAKE_BFS_OBJS) $(BFS_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(MAKE_BFS_OBJS) $(BFS_OBJS)

clean:
	/bin/rm -f *.o *~ test make_bfs
