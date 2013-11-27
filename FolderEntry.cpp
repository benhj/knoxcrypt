#include "FolderEntry.hpp"

namespace bfs
{

    FolderEntry::FolderEntry(std::string const &imagePath,
                            uint64_t const totalBlocks,
                            uint64_t const startBlock,
                            std::string const &name)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        // create folder data FileEntry in append mode
        , m_folderData(FileEntry(imagePath, totalBlocks, name, startBlock))
        , m_name(name)
    {
    }

    FileEntry
    FolderEntry::addFileEntry(std::string const &name)
    {
        // set the first bit to indicate that this entry is in use
        // it will point to a file and should not be written over
        uint8_t byte = 0;
        detail::setBitInByte(byte, 0);

        // set second bit to indicate that its type file; folder will be type 0
        detail::setBitInByte(byte, 1);

        // create a vector to hold filename
        std::vector<uint8_t> filename;
        filename.assign(detail::MAX_FILENAME_LENGTH, 0);
        int i = 0;
        for(; i < name.length(); ++i) {
            filename[i] = name[i];
        }
        filename[i] = '\0'; // set null byte to indicate end of filename

        // Create a new file entry
        FileEntry entry(m_imagePath, m_totalBlocks, m_name);

        // create bytes to represent first block
        uint64_t firstBlock = entry.getCurrentBlockIndex();
        uint8_t buf[8];
        detail::convertInt64ToInt8Array(firstBlock, buf);

        // write entry data to this folder
        (void)m_folderData.write((char*)&byte, 1);
        (void)m_folderData.write((char*)&filename.front(), detail::MAX_FILENAME_LENGTH);
        (void)m_folderData.write((char*)buf, 8);
        m_folderData.flush();

        return entry;
    }

    FolderEntry
    FolderEntry::addFolderEntry(std::string const &name)
    {
        /*
        // set the first bit to indicate that this entry is in use
        // it will point to a file and should not be written over
        uint8_t byte = 0;
        detail::setBitInByte(byte, 0);

        // set second bit to indicate that its type file; folder will be type 0
        detail::setBitInByte(byte, 1, false);

        // create a vector to hold filename
        std::vector<uint8_t> filename;
        filename.assign(detail::MAX_FILENAME_LENGTH, 0);
        int i = 0;
        for(; i < name.length(); ++i) {
            filename[i] = name[i];
        }
        filename[i] = '\0'; // set null byte to indicate end of filename

        // Create a new folder entry
        FileEntry entry(m_imagePath, m_totalBlocks, m_name);

        // create bytes to represent first block
        uint64_t firstBlock = entry.getCurrentBlockIndex();
        uint8_t buf[8];
        detail::convertInt64ToInt8Array(firstBlock, buf);

        // write entry data to this folder
        (void)m_folderData.write((char*)&byte, 1);
        (void)m_folderData.write((char*)&filename.front(), detail::MAX_FILENAME_LENGTH);
        (void)m_folderData.write((char*)buf, 8);
        m_folderData.flush();

        return entry;
        */
    }

    FileEntry
    FolderEntry::getFileEntry(std::string const &name)
    {

    }

    FolderEntry
    FolderEntry::getFolderEntry(std::string const &name)
    {

    }

    std::string
    FolderEntry::getName()
    {
        return m_name;
    }

}
