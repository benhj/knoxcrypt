#include "bfs/CoreBFSIO.hpp"
#include "bfs/MakeBFS.hpp"

#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    if(argc < 2) {
        std::cout<<"Insufficient number of arguments\n\nUsage:\n\nmake_bfs <blocks> <path>\n\n"<<std::endl;
        return 1;
    }

    int blocks = atoi(argv[1]);

    bfs::CoreBFSIO io;
    io.path = argv[2];
    io.blocks = blocks;
    io.password = "abcd1234";

    bfs::MakeBFS bfs(io);

    return 0;
}
