#ifndef BFS_FILE_STREAM_HPP__
#define BFS_FILE_STREAM_HPP__

#include "bfs/FileEntryDevice.hpp"

#include <boost/iostreams/stream.hpp>

// to be included when switch to c++11
//#include <memory>

namespace bfs
{
    typedef boost::iostreams::stream<FileEntryDevice> FileStream;

    // shared_ptr until I can get c++11 supported properly at
    // which point I'll switch to unique_ptr
    typedef boost::shared_ptr<FileStream> FileStreamPtr;


}

#endif // BFS_FILE_STREAM_HPP__
