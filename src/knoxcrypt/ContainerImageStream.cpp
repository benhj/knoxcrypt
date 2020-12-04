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


#include "knoxcrypt/ContainerImageStream.hpp"

/// Since these are statics need to make sure they're instantiated here!
bool cryptostreampp::IByteTransformer::m_init = false;
uint8_t cryptostreampp::IByteTransformer::g_bigKey[32]; 
uint8_t cryptostreampp::IByteTransformer::g_bigIV[32];   

namespace knoxcrypt
{
    ContainerImageStream::ContainerImageStream(SharedCoreIO const &io, std::ios::openmode mode)
        : m_cryptoStream(std::make_shared<cryptostreampp::CryptoStreamPP>(io->path, 
                                                                          io->encProps,
                                                                          mode,
                                                                          io->firstTimeInit))
    {
        io->firstTimeInit = false;
    }

    ContainerImageStream&
    ContainerImageStream::read(char * const buf, std::streamsize const n)
    {
        (void)m_cryptoStream->read(buf, n);
        return *this;
    }

    ContainerImageStream&
    ContainerImageStream::write(char const * buf, std::streamsize const n)
    {
        (void)m_cryptoStream->write(buf, n);
        return *this;
    }

    ContainerImageStream&
    ContainerImageStream::seekg(std::streampos pos)
    {
        (void)m_cryptoStream->seekg(pos);
        return *this;
    }
    ContainerImageStream&
    ContainerImageStream::seekg(std::streamoff off, std::ios_base::seekdir way)
    {
        (void)m_cryptoStream->seekg(off, way);
        return *this;
    }

    ContainerImageStream&
    ContainerImageStream::seekp(std::streampos pos)
    {
        (void)m_cryptoStream->seekp(pos);
        return *this;
    }

    ContainerImageStream&
    ContainerImageStream::seekp(std::streamoff off, std::ios_base::seekdir way)
    {
        (void)m_cryptoStream->seekp(off, way);
        return *this;
    }

    std::streampos
    ContainerImageStream::tellg()
    {
        return m_cryptoStream->tellg();
    }
    std::streampos
    ContainerImageStream::tellp()
    {
        return m_cryptoStream->tellp();
    }

    void
    ContainerImageStream::close()
    {
        m_cryptoStream->close();
    }

    void
    ContainerImageStream::flush()
    {
        m_cryptoStream->flush();
    }

    bool
    ContainerImageStream::is_open() const
    {
        return m_cryptoStream->is_open();
    }

    void
    ContainerImageStream::open(SharedCoreIO const &io,
                             std::ios::openmode mode)
    {
        m_cryptoStream->open(io->path, mode);
    }

    bool
    ContainerImageStream::bad() const
    {
        return m_cryptoStream->bad();
    }

    void
    ContainerImageStream::clear()
    {
        m_cryptoStream->clear();
    }
}
