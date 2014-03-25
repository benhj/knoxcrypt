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

    namespace helper {

        /**
         * @brief converts at std::ios open mode to a c-style fopen mode
         * @note  only considers those modes that are actually used (haven't bothered with trunc)
         * @param mode the open mode
         * @return the fopen mode string
         */
        inline std::string getModeString(std::ios::openmode mode)
        {
            std::string modeString("");
            if((mode & std::ios::in) == std::ios::in) {
                if((mode & std::ios::out) == std::ios::out) {
                    if ((mode & std::ios::app) == std::ios::app) {

                        //
                        // read, write appending on end. Make new file
                        // if it doesn't already exist
                        //
                        modeString.append("ab+");
                        return modeString;
                    } else {

                        //
                        // Read, write. Will fail if file does not already exist
                        //
                        modeString.append("rb+");
                        return modeString;
                    }
                }
                //
                // Read-only
                //
                modeString.append("rb");
                return modeString;
            }
            if((mode & std::ios::out) == std::ios::out) {
                //
                // Write-only
                //
                modeString.append("wb");
                return modeString;
            }

        }

    }


    TeaSafeImageStream::TeaSafeImageStream(SharedCoreIO const &io, std::ios::openmode mode)
        : m_stream(fopen(io->path.c_str(), helper::getModeString(mode).c_str()))
        , m_byteTransformer(boost::make_shared<cipher::XTEAByteTransformer>(io->password))
        , m_pos(0)
    {

    }

    TeaSafeImageStream::~TeaSafeImageStream()
    {
    }

    TeaSafeImageStream&
    TeaSafeImageStream::read(char * const buf, std::streamsize const n)
    {
        std::ios_base::streamoff start = m_pos;
        std::vector<char> in;
        in.resize(n);
        m_pos += fread(&in.front(), sizeof(char), n, m_stream);
        m_byteTransformer->transform(&in.front(), buf, start, n);
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::write(char const * buf, std::streamsize const n)
    {
        std::vector<char> out;
        out.resize(n);
        std::ios_base::streamoff start = m_pos;
        m_byteTransformer->transform((char*)buf, &out.front(), start, n);
        m_pos += fwrite(&out.front(), sizeof(char), n, m_stream);
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::seekg(std::streampos pos)
    {
        fseek(m_stream, pos, SEEK_SET);
        m_pos = pos;
        return *this;
    }
    TeaSafeImageStream&
    TeaSafeImageStream::seekg(std::streamoff off, std::ios_base::seekdir way)
    {
        if(way == std::ios_base::beg) {
            fseek(m_stream, off, SEEK_SET);
        } else if(way == std::ios_base::cur) {
            fseek(m_stream, off, SEEK_CUR);
        } else {
            fseek(m_stream, off, SEEK_END);
        }
        m_pos = ftell(m_stream);
        std::cout<<m_pos<<"\t"<<off<<std::endl;
        return *this;
    }

    TeaSafeImageStream&
    TeaSafeImageStream::seekp(std::streampos pos)
    {
        fseek(m_stream, pos, SEEK_SET);
        m_pos = pos;
        return *this;
    }
    TeaSafeImageStream&
    TeaSafeImageStream::seekp(std::streamoff off, std::ios_base::seekdir way)
    {
        if(way == std::ios_base::beg) {
            fseek(m_stream, off, SEEK_SET);
        } else if(way == std::ios_base::cur) {
            fseek(m_stream, off, SEEK_CUR);
        } else {
            fseek(m_stream, off, SEEK_END);
        }
        m_pos = ftell(m_stream);
        return *this;
    }

    std::streampos
    TeaSafeImageStream::tellg()
    {
        return m_pos;
    }
    std::streampos
    TeaSafeImageStream::tellp()
    {
        return m_pos;
    }

    void
    TeaSafeImageStream::close()
    {
        fclose(m_stream);
    }

    void
    TeaSafeImageStream::flush()
    {
        fflush(m_stream);
    }

}
