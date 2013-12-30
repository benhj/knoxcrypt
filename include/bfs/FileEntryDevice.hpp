/*
The MIT License (MIT)

Copyright (c) 2013 Ben H.D. Jones

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BFS_FILE_ENTRY_DEVICE_HPP__
#define BFS_FILE_ENTRY_DEVICE_HPP__

#include <bfs/FileEntry.hpp>

#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset

namespace bfs
{
    class FileEntryDevice
    {

        public:

            typedef char                                   char_type;
            typedef boost::iostreams::seekable_device_tag  category;

            explicit FileEntryDevice(FileEntry const &entry);

            std::streamsize read(char* s, std::streamsize n);
            std::streamsize write(const char* s, std::streamsize n);
            std::streampos seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way);

        private:
            FileEntryDevice();
            FileEntry m_entry;
    };

}

#endif // BFS_FILE_ENTRY_DEVICE_HPP__
