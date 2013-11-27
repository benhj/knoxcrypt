#ifndef BFS_FOLDER_ENTRY_HPP__
#define BFS_FOLDER_ENTRY_HPP__

#include "FileEntry.hpp"

namespace bfs
{

    class FolderEntry
    {
    public:
        /**
         * @brief constructs a FolderEntry
         * @param imagePath path of the bfs image
         * @param totalBlocks total file blocks in the bfs image
         * @param startBlock the index of the starting file block making up entry data
         * @param name the name of the entry
         */
        FolderEntry(std::string const &imagePath,
                    uint64_t const totalBlocks,
                    uint64_t const startBlock,
                    std::string const &name = "root");

        /**
         * @brief appends a new file entry and start index to the entry data
         * @param name the name of the entry
         * @return a copy of a FileEntry that will be used to reference the file data
         */
        FileEntry addFileEntry(std::string const &name);

        /**
         * @brief appends a new folder entry and start index of the entry data
         * @param name the name of the entry
         * @return a copy of a FolderEntry that will be used to reference the folder data
         */
        FolderEntry addFolderEntry(std::string const &name);

        /**
         * @brief retrieves a FileEntry with specific name
         * @param name the name of the entry to lookup
         * @return a copy of the FileEntry with name
         */
        FileEntry getFileEntry(std::string const &name);

        /**
         * @brief retrieves a FolderEntry with specific name
         * @param name the name of the entry to lookup
         * @return a copy of the FolderEntry with name
         */
        FolderEntry getFolderEntry(std::string const &name);

        /**
         * @brief retrieves the name of this folder
         * @return the name
         */
        std::string getName();

    private:

        FolderEntry();

        // the path of the bfs image
        std::string const m_imagePath;

        // total file blocks in the bfs image
        uint64_t const m_totalBlocks;

        // the underlying file blocks storing the folder entry data
        FileEntry m_folderData;

        // stores the name of this folder
        std::string m_name;

    };

}



#endif // BFS_FOLDER_ENTRY_HPP__
