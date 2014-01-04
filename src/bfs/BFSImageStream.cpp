/*
Copyright (c) <2013-2014>, <BenHJ>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/


#include "bfs/BFSImageStream.hpp"
#include "cipher/BasicByteTransformer.hpp"

#include <boost/make_shared.hpp>

#include <vector>

namespace bfs
{

    BFSImageStream::BFSImageStream(CoreBFSIO const &io, std::ios::openmode mode)
        : m_stream(io.path.c_str(), mode)
        , m_byteTransformer(boost::make_shared<cipher::BasicByteTransformer>(io.password))
    {
    }

    BFSImageStream&
    BFSImageStream::read(char * const buf, std::streamsize const n)
    {
        std::ios_base::streamoff start = m_stream.tellg();
        std::vector<char> in;
        in.resize(n);
        (void)m_stream.read(&in.front(), n);
        m_byteTransformer->transform(&in.front(), buf, start, n);
        return *this;
    }

    BFSImageStream&
    BFSImageStream::write(char const * buf, std::streamsize const n)
    {
        std::vector<char> out;
        out.resize(n);
        std::ios_base::streamoff start = m_stream.tellp();
        m_byteTransformer->transform((char*)buf, &out.front(), start, n);
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
