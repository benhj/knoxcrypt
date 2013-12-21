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

    bfs::MakeBFS bfs(argv[2], blocks);

    return 0;
}
