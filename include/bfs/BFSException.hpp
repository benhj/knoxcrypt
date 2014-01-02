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

#ifndef BFS_BFS_EXCEPTION_HPP__
#define BFS_BFS_EXCEPTION_HPP__

#include <exception>

namespace bfs
{

    enum class BFSError { NotFound, AlreadyExists, IllegalFilename, FolderNotEmpty };

    class BFSException : public std::exception
    {
      public:
        BFSException(BFSError const &error)
            : m_error(error)
        {

        }

        virtual const char* what() const throw()
        {
            if (m_error == BFSError::NotFound) {
                return "BFS: Entry not found";
            }

            if (m_error == BFSError::AlreadyExists) {
                return "BFS: Entry already exists";
            }

            if (m_error == BFSError::IllegalFilename) {
                return "BFS: illegal filename";
            }

            if (m_error == BFSError::FolderNotEmpty) {
                return "BFS: Folder not empty";
            }

            return "BFS: Unknown error";
        }

        bool operator==(BFSException const &other) const
        {
            return m_error == other.m_error;
        }

      private:
        BFSError m_error;
    };
}


#endif // BFS_BFS_EXCEPTION_HPP__
