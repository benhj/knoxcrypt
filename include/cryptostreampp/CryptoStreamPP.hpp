/*
  Copyright (c) <2013-2015>, <BenHJ>
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

#pragma once

#include "CipherBuilder.hpp"
#include "EncryptionProperties.hpp"
#include "IByteTransformer.hpp"


#include <functional>
#include <memory>
  
#include <fstream>
#include <string>
#include <vector>

namespace cryptostreampp
{
    typedef std::shared_ptr<IByteTransformer> ByteTransformerPtr;

    class CryptoStreamPP;
    typedef std::shared_ptr<CryptoStreamPP> SharedCryptoStream;

    class CryptoStreamPP
    {
      public:
        CryptoStreamPP(std::string const & path,
                       EncryptionProperties const &encProps,
                       std::ios::openmode mode = std::ios::out | std::ios::binary);

        CryptoStreamPP& read(char * const buf, std::streamsize const n);
        CryptoStreamPP& write(char const * buf, std::streamsize const n);
        CryptoStreamPP& seekg(std::streampos pos);
        CryptoStreamPP& seekg(std::streamoff off, std::ios_base::seekdir way);
        CryptoStreamPP& seekp(std::streampos pos);
        CryptoStreamPP& seekp(std::streamoff off, std::ios_base::seekdir way);
        std::streampos tellg() const;
        std::streampos tellp() const;
        bool bad() const;
        void clear();
        void flush();
        void close();
        bool is_open() const;
        void open(std::string const &path,
                  std::ios::openmode mode = std::ios::out | std::ios::binary);
      private:
        CryptoStreamPP();
        std::fstream m_stream;
        ByteTransformerPtr m_byteTransformer;
        std::streampos m_gpos;
        std::streampos m_ppos;
    };

    inline
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

    inline
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

    inline
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

    inline
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

    inline
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

    inline
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

    inline
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

    inline
    std::streampos
    CryptoStreamPP::tellg() const
    {
        return m_gpos;
    }

    inline
    std::streampos
    CryptoStreamPP::tellp() const
    {
        return m_ppos;
    }

    inline
    void
    CryptoStreamPP::close()
    {
        m_stream.close();
    }

    inline
    void
    CryptoStreamPP::flush()
    {
        m_stream.flush();
    }

    inline
    bool
    CryptoStreamPP::is_open() const
    {
        return m_stream.is_open();
    }

    inline
    void
    CryptoStreamPP::open(std::string const &path,
                         std::ios::openmode mode)
    {
        m_stream.open(path.c_str(), mode);
        m_ppos = 0;
        m_gpos = 0;
    }

    inline
    bool
    CryptoStreamPP::bad() const
    {
        return m_stream.bad();
    }

    inline
    void
    CryptoStreamPP::clear()
    {
        m_stream.clear();
    }

}

