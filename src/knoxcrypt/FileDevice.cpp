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

#include "knoxcrypt/FileDevice.hpp"

namespace knoxcrypt
{

    FileDevice::FileDevice(SharedFile const &entry)
        : m_entry(entry)
    {
    }

    std::streamsize
    FileDevice::read(char* s, std::streamsize n)
    {
        std::streamsize read = m_entry->read(s, n);
        if(read == 0) {
            return -1;
        }
        return read;
    }

    std::streamsize
    FileDevice::write(const char* s, std::streamsize n)
    {
        std::streamsize wrote = m_entry->write(s, n);
        m_entry->flush();
        return wrote;
    }

    std::streampos
    FileDevice::seek(boost::iostreams::stream_offset off, std::ios_base::seekdir way)
    {
        return m_entry->seek(off, way);
    }

    std::streampos
    FileDevice::tellg() const
    {
        return m_entry->tell();
    }

    std::streampos
    FileDevice::tellp() const
    {
        return m_entry->tell();
    }
}
