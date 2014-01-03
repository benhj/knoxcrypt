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

#include "bfs/BFSImageStream.hpp"
#include "cipher/BasicByteTransformer.hpp"

#include <boost/make_shared.hpp>

#include <vector>

namespace bfs
{

    BFSImageStream::BFSImageStream(CoreBFSIO const &io, std::ios::openmode mode)
        : m_stream(io.path.c_str(), mode)
        , m_cipher(boost::make_shared<cipher::BasicByteTransformer>(io.password))
    {
    }

    BFSImageStream&
    BFSImageStream::read(char * const buf, std::streamsize const n)
    {
        std::ios_base::streamoff start = m_stream.tellg();
        std::vector<char> in;
        in.resize(n);
        (void)m_stream.read(&in.front(), n);
        m_cipher->transform(&in.front(), buf, start, n);
        return *this;
    }

    BFSImageStream&
    BFSImageStream::write(char const * buf, std::streamsize const n)
    {
        std::vector<char> out;
        out.resize(n);
        std::ios_base::streamoff start = m_stream.tellp();
        m_cipher->transform((char*)buf, &out.front(), start, n);
        (void)m_stream.write(&out.front(), n);
        return *this;
    }

    BFSImageStream&
    BFSImageStream::seekg(std::streampos pos)
    {
        (void)m_stream.seekg(pos);
        return *this;
    }
    BFSImageStream&
    BFSImageStream::seekg(std::streamoff off, std::ios_base::seekdir way)
    {
        (void)m_stream.seekg(off, way);
        return *this;
    }

    BFSImageStream&
    BFSImageStream::seekp(std::streampos pos)
    {
        (void)m_stream.seekp(pos);
        return *this;
    }
    BFSImageStream&
    BFSImageStream::seekp(std::streamoff off, std::ios_base::seekdir way)
    {
        (void)m_stream.seekp(off, way);
        return *this;
    }

    std::streampos
    BFSImageStream::tellg()
    {
        return m_stream.tellg();
    }
    std::streampos
    BFSImageStream::tellp()
    {
        return m_stream.tellp();
    }

    void
    BFSImageStream::close()
    {
        m_stream.close();
    }

    void
    BFSImageStream::flush()
    {
        m_stream.flush();
    }

}
