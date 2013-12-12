#ifndef BFS_FOLDER_MANAGER_HPP__
#define BFS_FOLDER_MANAGER_HPP__

#include "FileEntry.hpp"
#include "FolderEntry.hpp"

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
