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

#ifndef BFS_FOLDER_MANAGER_HPP__
#define BFS_FOLDER_MANAGER_HPP__

#include "bfs/FileEntry.hpp"
#include "bfs/FolderEntry.hpp"

namespace bfs
{
    class FolderManager
    {
      public:
        FolderManager(std::string const &name);

        void addFile(std::string const& name, std::iostream &fileData)
        {
            m_thisFolderEntry.addFileEntry(name);
        }
        void addFolder(std::string const& name);

        void getFile(std::string &name, std::ostream &output);
        void getFile(uint64_t const fileId, std::ostream &output);

        void removeFile(std::string const &name);
        void removeFolder(std::string const &name);

        std::vector<std::string> listContents();

      private:
        FolderEntry m_thisFolderEntry;

    };
}

#endif // BFS_FOLDER_MANAGER_HPP__
