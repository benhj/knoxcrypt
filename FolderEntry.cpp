#include "DetailFolder.hpp"
#include "FolderEntry.hpp"

namespace bfs
{

    FolderEntry::FolderEntry(std::string const &imagePath,
                            uint64_t const totalBlocks,
                            uint64_t const startBlock,
                            std::string const &name,
                            bool const writable)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        // create folder data FileEntry in append mode
        , m_folderData(writable ? FileEntry(imagePath, totalBlocks, name, startBlock) :
        				FileEntry(imagePath, totalBlocks, startBlock))
        , m_name(name)
    {
    }

    FileEntry
    FolderEntry::addFileEntry(std::string const &name)
    {

        // increment entry count
        detail::incrementFolderEntryCount(m_imagePath, m_folderData.getStartBlockIndex(), m_totalBlocks);

        // seek right to end in order to append new entry
        (void)m_folderData.seek(0, std::ios_base::end);

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
        detail::convertUInt64ToInt8Array(firstBlock, buf);

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
        // increment entry count
        detail::incrementFolderEntryCount(m_imagePath, m_folderData.getStartBlockIndex(), m_totalBlocks);

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
    FolderEntry::getEntryName(uint64_t const index)
    {
        // note in the following the '1' represents first byte,
        // the first bit of which indicates if entry is in use
        // the '8' is number of bytes representing the first block index
        // of the given file entry
    	uint32_t bufferSize = detail::MAX_FILENAME_LENGTH + 1 + 8;

    	// note in the following the '8' bytes represent the number of
    	// entries in the folder
    	if(m_folderData.seek((index * bufferSize) + 8) != -1) {

    		std::vector<uint8_t> buffer;
    		buffer.resize(bufferSize);
    		m_folderData.read((char*)&buffer.front(), bufferSize);
    		std::string nameDat(buffer.begin() + 1, buffer.end() - 8);
    		std::string returnString;
    		int c = 0;
    		while(nameDat.at(c) != '\0') {
    			returnString.push_back(nameDat.at(c));
    			++c;
    		}
    		return returnString;
    	}
    	return "ERROR";
    }

    std::string
    FolderEntry::getName()
    {
        return m_name;
    }

}
