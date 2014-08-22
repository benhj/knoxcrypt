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
  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software without
  specific prior written permission.

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
#include "cipher/AESByteTransformer.hpp"
#include "cipher/TwofishByteTransformer.hpp"

#include <boost/make_shared.hpp>

#include <vector>

namespace teasafe
{

    boost::shared_ptr<cipher::IByteTransformer> buildCipherType(SharedCoreIO const &io)
    {
        if(io->cipher == 2) {
            return boost::make_shared<cipher::TwofishByteTransformer>(io->password,
                                                                      io->iv,
                                                                      io->iv2,
                                                                      io->iv3,
                                                                      io->iv3);
        } else {
            return boost::make_shared<cipher::AESByteTransformer>(io->password,
                                                                  io->iv,
                                                                  io->iv2,
                                                                  io->iv3,
                                                                  io->iv3);
        }
    }


    TeaSafeImageStream::TeaSafeImageStream(SharedCoreIO const &io, std::ios::openmode mode)
        : m_stream(io->path.c_str(), mode)
        , m_byteTransformer(buildCipherType(io))
        , m_gpos(0)
        , m_ppos(0)
    {
        // attach callback
        if(io->ccb) {
            m_byteTransformer->registerSignalHandler(*io->ccb);
        }

        // setup cipher
        m_byteTransformer->init();
    }

    TeaSafeImageStream&
    TeaSafeImageStream::read(char * const buf, std::streamsize const n)
    {
        std::ios_base::streamoff start = m_gpos;
        std::vector<char> in;
        in.resize(n);
        if(m_stream.read(&in.front(), n).bad()) {
            m_gpos = -1;
            return *this;
        }
        m_gpos += n;
        m_byteTransformer->decrypt(&in.front(), buf, start, n);
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::write(char const * buf, std::streamsize const n)
    {
        std::vector<char> out;
        out.resize(n);
        std::ios_base::streamoff start = m_ppos;
        m_byteTransformer->encrypt((char*)buf, &out.front(), start, n);
        if(m_stream.write(&out.front(), n).bad()) {
            m_ppos = -1;
            return *this;
        }
        m_ppos += n;
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::seekg(std::streampos pos)
    {
        if(m_stream.seekg(pos).bad()) {
            m_gpos = -1;
        } else {
            m_gpos = pos;
        }
        return *this;
    }
    TeaSafeImageStream&
    TeaSafeImageStream::seekg(std::streamoff off, std::ios_base::seekdir way)
    {
        if(m_stream.seekg(off, way).bad()) {
            m_gpos = -1;
        } else {
            m_gpos = m_stream.tellg();
        }
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::seekp(std::streampos pos)
    {
        // do not allow seeking past end of stream!!
        if(m_stream.seekp(pos).bad()) {
            m_ppos = -1;
        } else {
            m_ppos = pos;
        }
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::seekp(std::streamoff off, std::ios_base::seekdir way)
    {
        if(m_stream.seekp(off, way).bad()) {
            m_ppos = -1;
        } else {
            m_ppos = m_stream.tellp();
        }
        return *this;
    }

    std::streampos
    TeaSafeImageStream::tellg()
    {
        return m_gpos;
    }
    std::streampos
    TeaSafeImageStream::tellp()
    {
        return m_ppos;
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

    bool
    TeaSafeImageStream::is_open() const
    {
        return m_stream.is_open();
    }

    void
    TeaSafeImageStream::open(SharedCoreIO const &io,
                             std::ios::openmode mode)
    {
        m_stream.open(io->path.c_str(), mode);
        m_ppos = 0;
        m_gpos = 0;
    }

    bool
    TeaSafeImageStream::bad() const
    {
        return m_stream.bad();
    }

    void
    TeaSafeImageStream::clear()
    {
        m_stream.clear();
    }

}
