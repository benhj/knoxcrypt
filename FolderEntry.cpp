#include "DetailFolder.hpp"
#include "FolderEntry.hpp"
#include <stdexcept>

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
        uint64_t firstBlock = entry.getStartBlockIndex();
        uint8_t buf[8];
        detail::convertUInt64ToInt8Array(firstBlock, buf);

        // indicate that first block of new file entry is in use. NOte at this
        // point in time, it won't have any data. When data
        // needs to be added to the file entry, it will be opened
        // in append mode
        // Also: we need to do this before adding data to the folder entry
        // as data added to the folder entry might write over the block we
        // allocated for file
        std::fstream out(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        detail::updateVolumeBitmapWithOne(out, firstBlock, m_totalBlocks);
        out.close();

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
    	// find out total number of entries
    	// NOTE: should probably initialize this in constructor
        std::fstream out(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = detail::getOffsetOfFileBlock(m_folderData.getStartBlockIndex(), m_totalBlocks);
        (void)out.seekg(offset + detail::FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        uint64_t count = detail::convertInt8ArrayToInt64(buf);
        out.close();
        uint64_t i(0);

        // loop over entries until name found
        for(; i < count; ++i) {
        	std::string entryName(getEntryName(i));
        	if(entryName == name) {
        		uint64_t n = getBlockIndexForEntry(i);
        		return FileEntry(m_imagePath, m_totalBlocks, m_name, n);
        	}
        }

        throw std::runtime_error("File entry with that name not found");
    }

    FolderEntry
    FolderEntry::getFolderEntry(std::string const &name)
    {

    }

    uint64_t
    FolderEntry::getBlockIndexForEntry(uint64_t const n)
    {

    	uint32_t bufferSize = detail::MAX_FILENAME_LENGTH + 1 + 8;

    	// note in the following the '8' bytes represent the number of
    	// entries in the folder
    	if(m_folderData.seek((n * bufferSize) + 8 + 1 + detail::MAX_FILENAME_LENGTH) != -1) {
    		uint8_t buf[8];
    		m_folderData.read((char*)&buf, 8);
    		return detail::convertInt8ArrayToInt64(buf);
    	}
    	throw std::runtime_error("Problem retrieving block index for file entry");
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
    		// read past the first byte up until the end minus the
    		// first block index bytes
    		std::string nameDat(buffer.begin() + 1, buffer.end() - 8);
    		std::string returnString;
    		int c = 0;
    		while(nameDat.at(c) != '\0') {
    			returnString.push_back(nameDat.at(c));
    			++c;
    		}
    		return returnString;
    	}
    	throw std::runtime_error("Problem getting entry name for index");
    }

    std::string
    FolderEntry::getName()
    {
        return m_name;
    }

}
