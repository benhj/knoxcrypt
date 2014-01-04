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

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/

#ifndef BFS_BFS_IMAGE_STREAM_HPP__
#define BFS_BFS_IMAGE_STREAM_HPP__

#include "bfs/CoreBFSIO.hpp"
#include "cipher/IByteTransformer.hpp"

#include <boost/shared_ptr.hpp>

#include <fstream>
#include <string>

namespace bfs
{

    typedef boost::shared_ptr<cipher::IByteTransformer> ByteTransformerPtr;

    class BFSImageStream
    {
      public:
        explicit BFSImageStream(CoreBFSIO const &io,
                                std::ios::openmode mode = std::ios::out | std::ios::binary);

        BFSImageStream& read(char * const buf, std::streamsize const n);

        BFSImageStream& write(char const * buf, std::streamsize const n);

        BFSImageStream& seekg(std::streampos pos);
        BFSImageStream& seekg(std::streamoff off, std::ios_base::seekdir way);
        BFSImageStream& seekp(std::streampos pos);
        BFSImageStream& seekp(std::streamoff off, std::ios_base::seekdir way);
        std::streampos tellg();
        std::streampos tellp();

        void flush();

        void close();

      private:
        BFSImageStream();
        std::fstream m_stream;
        ByteTransformerPtr m_byteTransformer;
    };

}

#endif // BFS_BFS_IMAGE_STREAM_HPP__
