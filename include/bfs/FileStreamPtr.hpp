#ifndef BFS_FILE_STREAM_HPP__
#define BFS_FILE_STREAM_HPP__

#include <boost/scoped_ptr.hpp>

namespace bfs
{
    typedef boost::iostreams::stream<FileEntryDevice> FileStream;
    typedef boost::scoped_ptr<FileStream> FileStreamPtr;


}

#endif // BFS_FILE_STREAM_HPP__
