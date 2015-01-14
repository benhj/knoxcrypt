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

#pragma once

#include "CipherBuilder.hpp"
#include "EncryptionProperties.hpp"
#include "IByteTransformer.hpp"


#include <functional>
#include <memory>
#include <sstream>
  
#include <fstream>
#include <string>
#include <vector>

namespace cryptostreampp
{
    typedef std::shared_ptr<IByteTransformer> ByteTransformerPtr;

    class CryptoStreamPP;
    typedef std::shared_ptr<CryptoStreamPP> SharedCryptoStream;

    class CryptoStreamPP : public std::fstream
    {
      public:
        CryptoStreamPP(std::string const & path,
                       EncryptionProperties & encProps,
                       std::ios::openmode mode = std::ios::out | std::ios::binary);

        CryptoStreamPP& read(char * const buf, std::streamsize const n);
        CryptoStreamPP& write(char const * buf, std::streamsize const n);
        void open(std::string const &path,
                  std::ios::openmode mode = std::ios::out | std::ios::binary);

      private:
        CryptoStreamPP& doRead(char * const buf, std::streamsize const n);
        CryptoStreamPP& doWrite(char const * buf, std::streamsize const n);
        CryptoStreamPP();
        ByteTransformerPtr m_byteTransformer;
    };

    inline
    CryptoStreamPP::CryptoStreamPP(std::string const &path,
                                   EncryptionProperties & encProps, 
                                   std::ios::openmode mode)
        : std::fstream(path.c_str(), mode)
        , m_byteTransformer(buildCipherType(encProps))
    {
    }

    inline
    CryptoStreamPP&
    CryptoStreamPP::read(char * const buf, std::streamsize const n)
    {
        return doRead(buf, n);
    }

    inline
    CryptoStreamPP&
    CryptoStreamPP::write(char const * buf, std::streamsize const n)
    {
        return doWrite(buf, n);
    }


    inline
    CryptoStreamPP&
    CryptoStreamPP::doRead(char * const buf, std::streamsize const n)
    {
        std::ios_base::streamoff start = std::fstream::tellg();
        std::vector<char> in;
        in.resize(n);
        if(std::fstream::read(&in.front(), n).bad()) {
            return *this;
        }
        m_byteTransformer->decrypt(&in.front(), buf, start, n);
        return *this;
    }

    inline
    CryptoStreamPP&
    CryptoStreamPP::doWrite(char const * buf, std::streamsize const n)
    {
        std::vector<char> out;
        out.resize(n);
        std::ios_base::streamoff start = std::fstream::tellp();
        m_byteTransformer->encrypt((char*)buf, &out.front(), start, n);
        (void)std::fstream::write(&out.front(), n);
        return *this;
    }

    inline
    void
    CryptoStreamPP::open(std::string const &path, std::ios::openmode mode)
    {
        std::fstream::open(path.c_str(), mode);
    }
}

