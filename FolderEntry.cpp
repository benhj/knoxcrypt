#include "DetailBFS.hpp"
#include "DetailFolder.hpp"
#include "FolderEntry.hpp"
#include <stdexcept>

namespace bfs
{

    FolderEntry::FolderEntry(std::string const &imagePath,
                             uint64_t const totalBlocks,
                             uint64_t const startBlock,
                             std::string const &name)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_folderData(FileEntry(imagePath, totalBlocks, name, startBlock))
        , m_startBlock(startBlock)
        , m_name(name)
    {
    }

    FolderEntry::FolderEntry(std::string const &imagePath,
                             uint64_t const totalBlocks,
                             std::string const &name)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_folderData(m_imagePath, m_totalBlocks, m_name)
        , m_startBlock(m_folderData.getStartBlockIndex())
        , m_name(name)
    {
        // set initial number of entries; there will be none to begin with
        uint64_t startCount(0);
        uint8_t buf[8];
        detail::convertUInt64ToInt8Array(startCount, buf);
        (void)m_folderData.write((char*)buf, 8);
        m_folderData.flush();
    }

    namespace detail
    {
        std::vector<uint8_t> createFileNameVector(std::string const &name)
        {
            std::vector<uint8_t> filename;
            filename.assign(MAX_FILENAME_LENGTH, 0);
            int i = 0;
            for (; i < name.length(); ++i) {
                filename[i] = name[i];
            }
            filename[i] = '\0'; // set null byte to indicate end of filename
            return filename;
        }
    }

    std::streamsize
    FolderEntry::doWrite(char const * buf, std::streampos n)
    {
        return m_folderData.write(buf, n);
    }

    std::streamsize
    FolderEntry::doWriteFirstByteToEntryMetaData(EntryType const &entryType)
    {
        // set the first bit to indicate that this entry is in use
        // it will point to a file and should not be written over
        uint8_t byte = 0;
        detail::setBitInByte(byte, 0);

        // set second bit to indicate that its type file; folder will be type 0
        detail::setBitInByte(byte, 1, entryType==EntryType::FileType);

        // write out first byte
        return doWrite((char*)&byte, 1);
    }

    std::streamsize
    FolderEntry::doWriteFilenameToEntryMetaData(std::string const &name)
    {
        // create a vector to hold filename
        std::vector<uint8_t> filename = detail::createFileNameVector(name);

        // write out filename
        return doWrite((char*)&filename.front(), detail::MAX_FILENAME_LENGTH);
    }

    std::streamsize
    FolderEntry::doWriteFirstBlockIndexToEntryMetaData(FileEntry const &entry)
    {
        // create bytes to represent first block
        uint64_t firstBlock = entry.getStartBlockIndex();
        uint8_t buf[8];
        detail::convertUInt64ToInt8Array(firstBlock, buf);
        return doWrite((char*)buf, 8);
    }

    void
    FolderEntry::addFileEntry(std::string const &name)
    {
        // increment entry count
        detail::incrementFolderEntryCount(m_imagePath, m_folderData.getStartBlockIndex(), m_totalBlocks);

        // seek right to end in order to append new entry
        (void)m_folderData.seek(0, std::ios_base::end);

        // create and write first byte of filename metadata
        (void)doWriteFirstByteToEntryMetaData(EntryType::FileType);

        // Create a new file entry
        FileEntry entry(m_imagePath, m_totalBlocks, m_name);

        // flushing also ensures that first block is registered as being allocated
        entry.flush();

        // create and write filename
        (void)doWriteFilenameToEntryMetaData(name);

        // write the first block index to the file entry metadata
        (void)doWriteFirstBlockIndexToEntryMetaData(entry);

        // make sure all data has been written
        m_folderData.flush();

        // need to reset the file entry to make sure in correct place
        // NOTE: could probably optimize to not have to do this
        m_folderData = FileEntry(m_imagePath, m_totalBlocks, m_name, m_startBlock);
    }

    void
    FolderEntry::addFolderEntry(std::string const &name)
    {
        // increment entry count
        detail::incrementFolderEntryCount(m_imagePath, m_folderData.getStartBlockIndex(), m_totalBlocks);

        // seek right to end in order to append new entry
        (void)m_folderData.seek(0, std::ios_base::end);

        // create and write first byte of filename metadata
        (void)doWriteFirstByteToEntryMetaData(EntryType::FolderType);

        // Create a new sub-folder entry
        FolderEntry entry(m_imagePath, m_totalBlocks, m_name);

        // create and write filename
        (void)doWriteFilenameToEntryMetaData(name);

        // write the first block index to the file entry metadata
        (void)doWriteFirstBlockIndexToEntryMetaData(entry.m_folderData);

        // make sure all data has been written
        m_folderData.flush();

        // need to reset the file entry to make sure in correct place
        // NOTE: could probably optimize to not have to do this
        m_folderData = FileEntry(m_imagePath, m_totalBlocks, m_name, m_startBlock);
    }

    FileEntry
    FolderEntry::getFileEntry(std::string const &name) const
    {
        // find out total number of entries. Should initialize this in constructor?
        uint64_t count = getNumberOfEntries();
        uint64_t i(0);

        // loop over entries until name found
        for (; i < count; ++i) {
            std::string entryName(getEntryName(i));
            if (entryName == name && getTypeForEntry(i) == EntryType::FileType) {
                uint64_t n = getBlockIndexForEntry(i);
                return FileEntry(m_imagePath, m_totalBlocks, name, n);
            }
        }

        throw std::runtime_error("File entry with that name not found");
    }

    FolderEntry
    FolderEntry::getFolderEntry(std::string const &name)
    {
        // find out total number of entries. Should initialize this in constructor?
        uint64_t count = getNumberOfEntries();

        uint64_t i(0);

        // loop over entries until name found
        for (; i < count; ++i) {
            std::string entryName(getEntryName(i));
            if (entryName == name && getTypeForEntry(i) == EntryType::FolderType) {
                uint64_t n = getBlockIndexForEntry(i);
                return FolderEntry(m_imagePath, m_totalBlocks, n, name);
            }
        }

        throw std::runtime_error("Folder entry with that name not found");
    }

    std::string
    FolderEntry::getName() const
    {
        return m_name;
    }

    std::vector<EntryInfo>
    FolderEntry::listAllEntries()
    {
        std::vector<EntryInfo> entries;
        uint64_t entryCount = getNumberOfEntries();
        for (long entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
            entries.push_back(getEntryInfo(entryIndex));
        }
        return entries;
    }

    EntryInfo
    FolderEntry::getEntryInfo(uint64_t const entryIndex) const
    {
        std::string const entryName = getEntryName(entryIndex);
        EntryType const entryType = getTypeForEntry(entryIndex);
        FileEntry fe(getFileEntry(entryName));
        return EntryInfo(entryName,
                        fe.fileSize(),
                        entryType,
                        true, // writable
                        fe.getStartBlockIndex(),
                        entryIndex);
    }

    uint64_t
    FolderEntry::getNumberOfEntries() const
    {
        std::fstream out(m_imagePath.c_str(), std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = detail::getOffsetOfFileBlock(m_folderData.getStartBlockIndex(), m_totalBlocks);
        (void)out.seekg(offset + detail::FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        out.close();
        return detail::convertInt8ArrayToInt64(buf);
    }

    EntryType
    FolderEntry::getTypeForEntry(uint64_t const n) const
    {
        // note in the following the '1' represents first byte,
        // the first bit of which indicates if entry is in use
        // the '8' is number of bytes representing the first block index
        // of the given file entry
        uint32_t bufferSize = 1 + detail::MAX_FILENAME_LENGTH + 8;

        // note in the following the '8' bytes represent the number of
        // entries in the folder
        if (m_folderData.seek(8 + (n * bufferSize)) != -1) {
            uint8_t byte;
            m_folderData.read((char*)&byte, 1);
            return (detail::isBitSetInByte(byte, 1) ? EntryType::FileType : EntryType::FolderType);
        }
        throw std::runtime_error("Problem getting entry name for index");
    }

    std::string
    FolderEntry::getEntryName(uint64_t const index) const
    {
        // note in the following the '1' represents first byte,
        // the first bit of which indicates if entry is in use
        // the '8' is number of bytes representing the first block index
        // of the given file entry
        uint32_t bufferSize = 1 + detail::MAX_FILENAME_LENGTH + 8;

        // note in the following the '8' bytes represent the number of
        // entries in the folder
        if (m_folderData.seek(8 + (index * bufferSize)) != -1) {

            std::vector<uint8_t> buffer;
            buffer.resize(bufferSize);
            m_folderData.read((char*)&buffer.front(), bufferSize);
            // read past the first byte up until the end minus the
            // first block index bytes
            std::string nameDat(buffer.begin() + 1, buffer.end() - 8);
            std::string returnString;
            int c = 0;
            while (nameDat.at(c) != '\0') {
                returnString.push_back(nameDat.at(c));
                ++c;
            }
            return returnString;
        }
        throw std::runtime_error("Problem getting entry name for index");
    }

    uint64_t
    FolderEntry::getBlockIndexForEntry(uint64_t const n) const
    {

        // note in the following, the '1' represents meta data for this
        // filename descriptor and the 8 represents bytes storing the file
        // entry offset. The values for the buffer are stored in the order
        // in which they are given in the addition
        uint32_t bufferSize = 1 + detail::MAX_FILENAME_LENGTH + 8;

        // note in the following the '8' bytes represent the number of
        // entries in the folder so we seek past that bit; then we
        // seek to the correct filename descriptor position given bufferSize
        // Then, for the given descriptor that we've seeked to, we need to seek
        // past the first byte which is descriptor metadata and past the actual
        // filename data. We then end up at the start of the final 8 bytes which
        // represents the offset of the first block making up the file entry
        if (m_folderData.seek(8 + (n * bufferSize) + 1 + detail::MAX_FILENAME_LENGTH) != -1) {
            uint8_t buf[8];
            m_folderData.read((char*)&buf, 8);
            return detail::convertInt8ArrayToInt64(buf);
        }
        throw std::runtime_error("Problem retrieving block index for file entry");
    }

}
