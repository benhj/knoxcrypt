/*
  The MIT License (MIT)

  Copyright (c) 2013 Ben H.D. Jones

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
