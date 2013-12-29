#ifndef BFS_CORE_BFS_IO_HPP__
#define BFS_CORE_BFS_IO_HPP__

#include <string>

namespace bfs
{

    struct CoreBFSIO {
        std::string path;
        uint64_t blocks;
        std::string password;
    };

}


#endif //BFS_CORE_BFS_IO_HPP__
