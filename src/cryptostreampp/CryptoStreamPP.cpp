/*
  Copyright (c) <2015>, <BenHJ>
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

#include "CipherBuilder.hpp"

#include "cryptostreampp/CryptoStreamPP.hpp"

#include <vector>

namespace cryptostreampp
{
    CryptoStreamPP::CryptoStreamPP(std::string const &path,
                                   EncryptionProperties const & encProps, 
                                   std::ios::openmode mode)
        : m_stream(path.c_str(), mode)
        , m_byteTransformer(buildCipherType(encProps))
        , m_gpos(0)
        , m_ppos(0)
    {
        // setup cipher
        m_byteTransformer->init();
    }

    CryptoStreamPP&
    CryptoStreamPP::read(char * const buf, std::streamsize const n)
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

    CryptoStreamPP&
    CryptoStreamPP::write(char const * buf, std::streamsize const n)
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

    CryptoStreamPP&
    CryptoStreamPP::seekg(std::streampos pos)
    {
        if(m_stream.seekg(pos).bad()) {
            m_gpos = -1;
        } else {
            m_gpos = pos;
        }
        return *this;
    }
    CryptoStreamPP&
    CryptoStreamPP::seekg(std::streamoff off, std::ios_base::seekdir way)
    {
        if(m_stream.seekg(off, way).bad()) {
            m_gpos = -1;
        } else {
            m_gpos = m_stream.tellg();
        }
        return *this;
    }

    CryptoStreamPP&
    CryptoStreamPP::seekp(std::streampos pos)
    {
        // do not allow seeking past end of stream!!
        if(m_stream.seekp(pos).bad()) {
            m_ppos = -1;
        } else {
            m_ppos = pos;
        }
        return *this;
    }

    CryptoStreamPP&
    CryptoStreamPP::seekp(std::streamoff off, std::ios_base::seekdir way)
    {
        if(m_stream.seekp(off, way).bad()) {
            m_ppos = -1;
        } else {
            m_ppos = m_stream.tellp();
        }
        return *this;
    }

    std::streampos
    CryptoStreamPP::tellg()
    {
        return m_gpos;
    }
    std::streampos
    CryptoStreamPP::tellp()
    {
        return m_ppos;
    }

    void
    CryptoStreamPP::close()
    {
        m_stream.close();
    }

    void
    CryptoStreamPP::flush()
    {
        m_stream.flush();
    }

    bool
    CryptoStreamPP::is_open() const
    {
        return m_stream.is_open();
    }

    void
    CryptoStreamPP::open(std::string const &path,
                         std::ios::openmode mode)
    {
        m_stream.open(path.c_str(), mode);
        m_ppos = 0;
        m_gpos = 0;
    }

    bool
    CryptoStreamPP::bad() const
    {
        return m_stream.bad();
    }

    void
    CryptoStreamPP::clear()
    {
        m_stream.clear();
    }

}
