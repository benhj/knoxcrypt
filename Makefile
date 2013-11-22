CC=clang++
CXXFLAGS=-I/usr/local/boost_1_53_0

TEST_OBJS = test.o \
            BFSEntryWriter.o \
            BFSEntryAppender.o \
            /usr/local/boost_1_53_0/stage/lib/libboost_filesystem.dylib \
            /usr/local/boost_1_53_0/stage/lib/libboost_system.dylib

.c.o:
	$(CC) -c $(CFLAGS) -arch x86_64 $*.cpp

all: test

test:  $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS)

clean:
	/bin/rm -f *.o *~ test
