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

#include "bfs/EntryInfo.hpp"

namespace bfs
{
    EntryInfo::EntryInfo(std::string const &fileName,
                         uint64_t const &fileSize,
                         EntryType const &entryType,
                         bool const writable,
                         uint64_t const firstFileBlock,
                         uint64_t const folderIndex)
        : m_fileName(fileName)
        , m_fileSize(fileSize)
        , m_entryType(entryType)
        , m_writable(writable)
        , m_firstFileBlock(firstFileBlock)
        , m_folderIndex(folderIndex)
    {

    }

    std::string
    EntryInfo::filename() const
    {
        return m_fileName;
    }

    uint64_t
    EntryInfo::size() const
    {
        return m_fileSize;
    }

    EntryType
    EntryInfo::type() const
    {
        return m_entryType;
    }

    bool
    EntryInfo::writable() const
    {
        return m_writable;
    }

    uint64_t
    EntryInfo::firstFileBlock() const
    {
        return m_firstFileBlock;
    }

    uint64_t
    EntryInfo::folderIndex() const
    {
        return m_folderIndex;
    }

}
