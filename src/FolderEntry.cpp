#include "BFSImageStream.hpp"
#include "DetailBFS.hpp"
#include "DetailFolder.hpp"
#include "FolderEntry.hpp"
#include <stdexcept>

namespace bfs
{

    namespace detail
    {
        /**
         * @brief put a metadata section out of use by unsetting the first bit
         * @param folderData the data that stores the folder metadata
         * @param n the metadata chunk to put out of use
         */
        void putMetaDataOutOfUse(FileEntry &folderData, int n)
        {
            uint32_t bufferSize = 1 + MAX_FILENAME_LENGTH + 8;
            uint32_t seekTo = (8 + (n * bufferSize));
            if (folderData.seek(seekTo) != -1) {
                uint8_t byte = 0;
                detail::setBitInByte(byte, 0, false /* unset */);
                folderData.write((char*)&byte, 1);
                folderData.flush();
                return;
            }
            throw std::runtime_error("Problem putting entry metaData out of use");
        }

        /**
         * @brief checks if a given bit is set in the first byte of the
         * entry metadata
         * @param folderData the FileEntry data representing this folder entry
         * @note folderData needs to be passed in by copy so that it doesn't
         * disrupt the udnerlying file data
         * @param n the entry metadata to look up about
         * @param bit the bit to check
         * @return true if bit is set, false otherwise
         */
        bool checkMetaByte(FileEntry folderData, int n, int bit)
        {
            // note in the following the '1' represents first byte,
            // the first bit of which indicates if entry is in use
            // the '8' is number of bytes representing the first block index
            // of the given file entry
            uint32_t bufferSize = 1 + MAX_FILENAME_LENGTH + 8;

            // note in the following the '8' bytes represent the number of
            // entries in the folder
            if (folderData.seek(8 + (n * bufferSize)) != -1) {
                uint8_t byte;
                folderData.read((char*)&byte, 1);
                return (detail::isBitSetInByte(byte, bit));
            }
            throw std::runtime_error("Problem getting entry metaData enabled for index");
        }

        /**
         * @brief retrieves data from the entry metadata
         * @param folderData the metadata
         * @param n index of the entry for which we want to retrieve the metadata of
         * @param bufSize the size of the read buffer
         * @param seekOff the seek offset
         * @return the read meta data
         */
        std::vector<uint8_t> doSeekAndReadOfEntryMetaData(FileEntry folderData,
                                                          int n,
                                                          uint32_t bufSize = 0,
                                                          uint64_t seekOff = 0)
        {
            // note in the following the '1' represents first byte,
            // the first bit of which indicates if entry is in use
            // the '8' is number of bytes representing the first block index
            // of the given file entry
            uint32_t bufferSize = 1 + detail::MAX_FILENAME_LENGTH + 8;

            // note in the following the '8' bytes represent the number of
            // entries in the folder
            if (folderData.seek(8 + (n * bufferSize) + seekOff) != -1) {

                std::vector<uint8_t> buffer;
                buffer.resize(bufSize==0?bufferSize:bufSize);
                folderData.read((char*)&buffer.front(), bufSize==0?bufferSize:bufSize);
                return buffer;
            }
            throw std::runtime_error("Problem retrieving metadata");
        }

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

    FolderEntry::FolderEntry(std::string const &imagePath,
                             uint64_t const totalBlocks,
                             uint64_t const startBlock,
                             std::string const &name)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_folderData(imagePath,
                       totalBlocks,
                       name,
                       startBlock,
                       OpenDisposition::buildAppendDisposition())
        , m_startBlock(startBlock)
        , m_name(name)
    {
    }

    FolderEntry::FolderEntry(std::string const &imagePath,
                             uint64_t const totalBlocks,
                             std::string const &name)
        : m_imagePath(imagePath)
        , m_totalBlocks(totalBlocks)
        , m_folderData(imagePath, totalBlocks, name)
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
        // seek to correct position
        seekToPositionWhereMetaDataWillBeWritten();

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

        // increment entry count
        detail::incrementFolderEntryCount(m_imagePath, m_folderData.getStartBlockIndex(), m_totalBlocks);

        // need to reset the file entry to make sure in correct place
        // NOTE: could probably optimize to not have to do this
        m_folderData = FileEntry(m_imagePath, m_totalBlocks, m_name, m_startBlock,
                                 OpenDisposition::buildAppendDisposition());
    }

    void
    FolderEntry::addFolderEntry(std::string const &name)
    {
        // seek to correct position
        seekToPositionWhereMetaDataWillBeWritten();

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

        // increment entry count
        detail::incrementFolderEntryCount(m_imagePath, m_folderData.getStartBlockIndex(), m_totalBlocks);

        // need to reset the file entry to make sure in correct place
        // NOTE: could probably optimize to not have to do this
        m_folderData = FileEntry(m_imagePath, m_totalBlocks, m_name, m_startBlock,
                                 OpenDisposition::buildAppendDisposition());
    }

    FileEntry
    FolderEntry::getFileEntry(std::string const &name,
                              OpenDisposition const &openDisposition) const
    {
        // find out total number of entries. Should initialize this in constructor?
        uint64_t count = getNumberOfEntries();
        uint64_t i(0);

        // loop over entries until name found
        for (; i < count; ++i) {
            std::string entryName(getEntryName(i));
            if (entryName == name && getTypeForEntry(i) == EntryType::FileType) {
                uint64_t n = getBlockIndexForEntry(i);
                return FileEntry(m_imagePath, m_totalBlocks, name, n, openDisposition);
            }
        }

        throw std::runtime_error("File entry with that name not found");
    }

    FolderEntry
    FolderEntry::getFolderEntry(std::string const &name) const
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
            // only push back if the metadata is enabled
            if (entryMetaDataIsEnabled(entryIndex)) {
                entries.push_back(getEntryInfo(entryIndex));
            }
        }
        return entries;
    }

    std::vector<EntryInfo>
    FolderEntry::doListEntriesBasedOnType(EntryType entryType)
    {
        std::vector<EntryInfo> entries;
        uint64_t entryCount = getNumberOfEntries();
        for (long entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
            // only push back if the metadata is enabled
            if (entryMetaDataIsEnabled(entryIndex) &&
                getTypeForEntry(entryIndex) == entryType) {
                entries.push_back(getEntryInfo(entryIndex));
            }
        }
        return entries;
    }

    std::vector<EntryInfo>
    FolderEntry::listFileEntries()
    {
        return doListEntriesBasedOnType(EntryType::FileType);
    }

    std::vector<EntryInfo>
    FolderEntry::listFolderEntries()
    {
        return doListEntriesBasedOnType(EntryType::FolderType);
    }

    void
    FolderEntry::removeFileEntry(std::string const &name)
    {
        // first unlink; this deallocates the file blocks, updating the
        // volume bitmap accordingly; note doesn't matter what opendisposition is here
        FileEntry entry = getFileEntry(name, OpenDisposition::buildAppendDisposition());
        entry.unlink();

        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        FileEntry temp(m_imagePath, m_totalBlocks, m_name, m_startBlock,
                       OpenDisposition::buildOverwriteDisposition());
        detail::putMetaDataOutOfUse(temp, getMetaDataIndexForEntry(name));
        assert(!entryMetaDataIsEnabled(getMetaDataIndexForEntry(name)));
    }

    void
    FolderEntry::removeFolderEntry(std::string const &name)
    {
        FolderEntry entry = getFolderEntry(name);

        // loop over entries unlinking files and recursing into sub folders
        // and deleting their entries
        std::vector<EntryInfo> infos = entry.listAllEntries();
        std::vector<EntryInfo>::iterator it = infos.begin();
        for(; it != infos.end(); ++it) {
            if(it->type() == EntryType::FileType) {
                entry.removeFileEntry(it->filename());
            } else {
                entry.removeFolderEntry(it->filename());
            }
        }

        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        OpenDisposition openDisposition(AppendOrOverwrite::Overwrite,
                                        CreateOrDontCreate::Create,
                                        TruncateOrKeep::Keep);
        FileEntry temp(m_imagePath, m_totalBlocks, m_name, m_startBlock, openDisposition);
        detail::putMetaDataOutOfUse(temp, getMetaDataIndexForEntry(name));
        assert(!entryMetaDataIsEnabled(getMetaDataIndexForEntry(name)));

        // unlink entry's data
        entry.m_folderData.unlink();
    }

    EntryInfo
    FolderEntry::getEntryInfo(uint64_t const entryIndex) const
    {
        std::string const entryName = getEntryName(entryIndex);
        EntryType const entryType = getTypeForEntry(entryIndex);
        uint64_t fileSize = 0;
        uint64_t startBlock;
        if (entryType == EntryType::FileType) {
            // note disposition doesn't matter here, can be anything
            FileEntry fe(getFileEntry(entryName, OpenDisposition::buildAppendDisposition()));
            fileSize = fe.fileSize();
            startBlock = fe.getStartBlockIndex();
        } else {
            FolderEntry fe(getFolderEntry(entryName));
            startBlock = fe.m_folderData.getStartBlockIndex();
        }

        return EntryInfo(entryName,
                         fileSize,
                         entryType,
                         true, // writable
                         startBlock,
                         entryIndex);
    }

    uint64_t
    FolderEntry::getNumberOfEntries() const
    {
        bfs::BFSImageStream out(m_imagePath, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = detail::getOffsetOfFileBlock(m_folderData.getStartBlockIndex(), m_totalBlocks);
        (void)out.seekg(offset + detail::FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        out.close();
        return detail::convertInt8ArrayToInt64(buf);
    }



    bool
    FolderEntry::entryMetaDataIsEnabled(uint64_t const n) const
    {
        // the 0th bit indicates if metadata is in use. When a new
        // entry is created it is set to being in use. When deleted
        // it is unset
        return detail::checkMetaByte(m_folderData, n, 0);
    }

    EntryType
    FolderEntry::getTypeForEntry(uint64_t const n) const
    {
        // the 1st bit indicates if metadata represents a file or a folder
        // if set, it represents a file; if unset, it represents a folder
        return (detail::checkMetaByte(m_folderData, n, 1) ? EntryType::FileType : EntryType::FolderType);
    }

    std::string
    FolderEntry::getEntryName(uint64_t const n) const
    {
        std::vector<uint8_t> buffer = detail::doSeekAndReadOfEntryMetaData(m_folderData, n);
        std::string nameDat(buffer.begin() + 1, buffer.end() - 8);
        std::string returnString;
        int c = 0;
        while (nameDat.at(c) != '\0') {
            returnString.push_back(nameDat.at(c));
            ++c;
        }
        return returnString;
    }

    uint64_t
    FolderEntry::getMetaDataIndexForEntry(std::string const &name) const
    {
        uint64_t entryCount = getNumberOfEntries();
        for (long entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
            if (name == getEntryName(entryIndex)) {
                return entryIndex;
            }
        }
        throw std::runtime_error("Folder entry with that name not found");
    }

    uint64_t
    FolderEntry::getBlockIndexForEntry(uint64_t const n) const
    {
        std::vector<uint8_t> buffer = detail::doSeekAndReadOfEntryMetaData(m_folderData,
                                                                           n,
                                                                           8,
                                                                           (1 + detail::MAX_FILENAME_LENGTH));
        return detail::convertInt8ArrayToInt64(&buffer.front());
    }

    void
    FolderEntry::seekToPositionWhereMetaDataWillBeWritten()
    {
        // make sure we start in correct position
        (void)m_folderData.seek(0);

        // loop over all entries and try and find a previously deleted one
        // If its deleted, the first bit of the entry metadata will be unset
        uint64_t entryCount = getNumberOfEntries();
        for (long entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
            // only push back if the metadata is enabled
            if (!entryMetaDataIsEnabled(entryIndex)) {
                uint32_t bufferSize = 1 + detail::MAX_FILENAME_LENGTH + 8;
                m_folderData.seek(8 + (entryIndex * bufferSize));
                return;
            }
        }

        // free entry not found so seek right to end
        m_folderData.seek(0, std::ios_base::end);
    }

}
