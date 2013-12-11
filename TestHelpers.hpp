#ifndef BFS_TEST_HELPERS_HPP__
#define BFS_TEST_HELPERS_HPP__

#include <string>

int const HELLO_IT = 5000;
int const BIG_SIZE = HELLO_IT * 13;


std::string createLargeStringToWrite()
{
    std::string theString("");
    for(int i = 0; i < HELLO_IT; ++i) {
        theString.append("Hello, World!");
    }
    return theString;
}

#define ASSERT_EQUAL(A, B, C) \
    if(A == B) { \
        std::cout<<C<<"\tpassed"<<std::endl; \
    } else { \
        std::cout<<C<<"\tfailed"<<std::endl; \
    }

#endif
