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

#include "EncryptionProperties.hpp"
#include "IByteTransformer.hpp"

#include <functional>
#include <memory>
  
#include <fstream>
#include <string>

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
        std::streampos tellg();
        std::streampos tellp();
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

}

