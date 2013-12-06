CXX=clang++
CXXFLAGS=-I/usr/local/boost_1_53_0

TEST_OBJS = test.o \
            FileEntry.o \
            FolderEntry.o \
            /usr/local/boost_1_53_0/stage/lib/libboost_filesystem.dylib \
            /usr/local/boost_1_53_0/stage/lib/libboost_system.dylib

.c.o:
	$(CXX) -c $(CXXFLAGS) -arch x86_64 $*.cpp

all: test

test:  $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS)

clean:
	/bin/rm -f *.o *~ test
