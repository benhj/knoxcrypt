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
*/


#include "teasafe/TeaSafeImageStream.hpp"
#include "cipher/XTEAByteTransformer.hpp"

#include <boost/make_shared.hpp>

#include <vector>

namespace teasafe
{

    TeaSafeImageStream::TeaSafeImageStream(SharedCoreIO const &io, std::ios::openmode mode)
        : m_stream(io->path.c_str(), mode)
        , m_byteTransformer(boost::make_shared<cipher::XTEAByteTransformer>(io->password))
        , m_pos(0)
    {
    }

    TeaSafeImageStream&
    TeaSafeImageStream::read(char * const buf, std::streamsize const n)
    {
        std::ios_base::streamoff start = m_pos;//m_stream.tellg();
        std::vector<char> in;
        in.resize(n);
        (void)m_stream.read(&in.front(), n);
        m_pos += n;
        m_byteTransformer->transform(&in.front(), buf, start, n);
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::write(char const * buf, std::streamsize const n)
    {
        std::vector<char> out;
        out.resize(n);
        std::ios_base::streamoff start = m_pos;//m_stream.tellp();
        m_byteTransformer->transform((char*)buf, &out.front(), start, n);
        (void)m_stream.write(&out.front(), n);
        m_pos += n;
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::seekg(std::streampos pos)
    {
        (void)m_stream.seekg(pos);
        m_pos = pos;
        return *this;
    }
    TeaSafeImageStream&
    TeaSafeImageStream::seekg(std::streamoff off, std::ios_base::seekdir way)
    {
        (void)m_stream.seekg(off, way);
        m_pos = m_stream.tellg();
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::seekp(std::streampos pos)
    {
        (void)m_stream.seekp(pos);
        m_pos = pos;
        return *this;
    }
    TeaSafeImageStream&
    TeaSafeImageStream::seekp(std::streamoff off, std::ios_base::seekdir way)
    {
        (void)m_stream.seekp(off, way);
        m_pos = m_stream.tellp();
        return *this;
    }

    std::streampos
    TeaSafeImageStream::tellg()
    {
        return m_pos;//m_stream.tellg();
    }
    std::streampos
    TeaSafeImageStream::tellp()
    {
        return m_pos;//m_stream.tellp();
    }

    void
    TeaSafeImageStream::close()
    {
        m_stream.close();
    }

    void
    TeaSafeImageStream::flush()
    {
        m_stream.flush();
    }

}
