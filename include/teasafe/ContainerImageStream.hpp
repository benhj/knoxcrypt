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

#include "teasafe/CoreTeaSafeIO.hpp"
#include "utility/EventType.hpp"
#include "cryptostreampp/CryptoStreamPP.hpp"

#include <functional>
#include <memory>

#include <fstream>
#include <string>

namespace teasafe
{
    class ContainerImageStream;
    using SharedImageStream = std::shared_ptr<ContainerImageStream>;

    class ContainerImageStream
    {
      public:
        explicit ContainerImageStream(SharedCoreIO const &io,
                                      std::ios::openmode mode = std::ios::out | std::ios::binary);

        ContainerImageStream& read(char * const buf, std::streamsize const n);

        ContainerImageStream& write(char const * buf, std::streamsize const n);

        ContainerImageStream& seekg(std::streampos pos);
        ContainerImageStream& seekg(std::streamoff off, std::ios_base::seekdir way);
        ContainerImageStream& seekp(std::streampos pos);
        ContainerImageStream& seekp(std::streamoff off, std::ios_base::seekdir way);
        std::streampos tellg();
        std::streampos tellp();
        bool bad() const;
        void clear();

        void flush();

        void close();

        bool is_open() const;

        void open(SharedCoreIO const &io,
                  std::ios::openmode mode = std::ios::out | std::ios::binary);
      private:
        ContainerImageStream();
        cryptostreampp::SharedCryptoStream m_cryptoStream;
    };

}
