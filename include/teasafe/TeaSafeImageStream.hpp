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

#ifndef TeaSafe_TeaSafe_IMAGE_STREAM_HPP__
#define TeaSafe_TeaSafe_IMAGE_STREAM_HPP__

#include "teasafe/CoreTeaSafeIO.hpp"
#include "utility/EventType.hpp"
#include "cipher/IByteTransformer.hpp"

#include <boost/function.hpp>
#include <memory>
  
#include <fstream>
#include <string>

namespace teasafe
{

    typedef std::shared_ptr<cipher::IByteTransformer> ByteTransformerPtr;

    class TeaSafeImageStream;
    typedef std::shared_ptr<TeaSafeImageStream> SharedImageStream;

    class TeaSafeImageStream
    {
      public:
        explicit TeaSafeImageStream(SharedCoreIO const &io,
                                    std::ios::openmode mode = std::ios::out | std::ios::binary);

        TeaSafeImageStream& read(char * const buf, std::streamsize const n);

        TeaSafeImageStream& write(char const * buf, std::streamsize const n);

        TeaSafeImageStream& seekg(std::streampos pos);
        TeaSafeImageStream& seekg(std::streamoff off, std::ios_base::seekdir way);
        TeaSafeImageStream& seekp(std::streampos pos);
        TeaSafeImageStream& seekp(std::streamoff off, std::ios_base::seekdir way);
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
        TeaSafeImageStream();
        std::fstream m_stream;
        ByteTransformerPtr m_byteTransformer;
        std::streampos m_gpos;
        std::streampos m_ppos;
    };

}

#endif // TeaSafe_TeaSafe_IMAGE_STREAM_HPP__
