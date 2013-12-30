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

#include "bfs/FileEntryDevice.hpp"

namespace bfs
{

    FileEntryDevice::FileEntryDevice(FileEntry const &entry)
        : m_entry(entry)
    {
    }

    std::streamsize
    FileEntryDevice::read(char* s, std::streamsize n)
    {
        return m_entry.read(s, n);
    }

    std::streamsize
    FileEntryDevice::write(const char* s, std::streamsize n)
    {
        std::streamsize wrote = m_entry.write(s, n);
        m_entry.flush();
        return wrote;
    }

    std::streampos
    FileEntryDevice::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
    {
        return m_entry.seek(off, way);
    }
}

