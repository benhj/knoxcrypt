/*
Copyright (c) <2013-2014>, <BenHJ>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/

#include "bfs/BFSImageStream.hpp"
#include "bfs/DetailBFS.hpp"
#include "bfs/DetailFolder.hpp"
#include "bfs/FolderEntry.hpp"
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
        void putMetaDataOutOfUse(FileEntry folderData, int n)
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

                std::vector<uint8_t> metaData;
                metaData.resize(bufSize==0?bufferSize:bufSize);
                folderData.read((char*)&metaData.front(), bufSize==0?bufferSize:bufSize);
                return metaData;
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

    FolderEntry::FolderEntry(CoreBFSIO const &io,
                             uint64_t const startVolumeBlock,
                             std::string const &name)
        : m_io(io)
        , m_folderData(io,
                       name,
                       startVolumeBlock,
                       OpenDisposition::buildAppendDisposition())
        , m_startVolumeBlock(startVolumeBlock)
        , m_name(name)
        , m_entryCount(doGetNumberOfEntries())
    {
    }

    FolderEntry::FolderEntry(CoreBFSIO const &io,
                             std::string const &name)
        : m_io(io)
        , m_folderData(m_io, name)
        , m_startVolumeBlock(m_folderData.getStartVolumeBlockIndex())
        , m_name(name)
        , m_entryCount(0)
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
    FolderEntry::doWriteFirstBlockIndexToEntryMetaData(uint64_t firstBlock)
    {
        // create bytes to represent first block
        uint8_t buf[8];
        detail::convertUInt64ToInt8Array(firstBlock, buf);
        return doWrite((char*)buf, 8);
    }

    void
    FolderEntry::writeNewMetaDataForEntry(std::string const &name,
                                          EntryType const &entryType,
                                          uint64_t startBlock)
    {
        this->doWriteNewMetaDataForEntry(name, entryType, startBlock);
    }

    void
    FolderEntry::doWriteNewMetaDataForEntry(std::string const &name,
                                            EntryType const &entryType,
                                            uint64_t startBlock)
    {
        OptionalOffset overWroteOld = doFindOffsetWhereMetaDataShouldBeWritten();

        if (overWroteOld) {
            m_folderData = FileEntry(m_io, m_name, m_startVolumeBlock,
                                     OpenDisposition::buildOverwriteDisposition());
            m_folderData.seek(*overWroteOld);
        } else {
            m_folderData.seek(0, std::ios_base::end);
        }

        // create and write first byte of filename metadata
        (void)doWriteFirstByteToEntryMetaData(entryType);

        // create and write filename
        (void)doWriteFilenameToEntryMetaData(name);

        // write the first block index to the file entry metadata
        (void)doWriteFirstBlockIndexToEntryMetaData(startBlock);

        // make sure all data has been written
        m_folderData.flush();

        // increment entry count, but only if brand new
        if (!overWroteOld) {
            detail::incrementFolderEntryCount(m_io, m_folderData.getStartVolumeBlockIndex());
            ++m_entryCount;
        }
    }

    void
    FolderEntry::addFileEntry(std::string const &name)
    {
        // Create a new file entry
        FileEntry entry(m_io, name);

        // write the first block index to the file entry metadata
        this->doWriteNewMetaDataForEntry(name, EntryType::FileType, entry.getStartVolumeBlockIndex());
    }

    void
    FolderEntry::addFolderEntry(std::string const &name)
    {
        // Create a new sub-folder entry
        FolderEntry entry(m_io, name);

        // write the first block index to the file entry metadata
        this->doWriteNewMetaDataForEntry(name, EntryType::FolderType, entry.m_folderData.getStartVolumeBlockIndex());
    }

    FileEntry
    FolderEntry::getFileEntry(std::string const &name,
                              OpenDisposition const &openDisposition) const
    {

        uint64_t i(0);

        // loop over entries until name found
        for (; i < m_entryCount; ++i) {
            // read all metadata
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, i);
            std::string entryName(doGetEntryName(metaData));
            if (entryName == name && doGetTypeForEntry(metaData) == EntryType::FileType) {
                uint64_t n = doGetBlockIndexForEntry(metaData);
                return FileEntry(m_io, name, n, openDisposition);
            }
        }

        throw std::runtime_error("File entry with that name not found");
    }

    FolderEntry
    FolderEntry::getFolderEntry(std::string const &name) const
    {

        uint64_t i(0);

        // loop over entries until name found
        for (; i < m_entryCount; ++i) {
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, i);
            std::string entryName(doGetEntryName(metaData));
            if (entryName == name && doGetTypeForEntry(metaData) == EntryType::FolderType) {
                uint64_t n = doGetBlockIndexForEntry(metaData);
                return FolderEntry(m_io, n, name);
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
    FolderEntry::listAllEntries() const
    {
        std::vector<EntryInfo> entries;

        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {

            // read all metadata
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);
            if (doEntryMetaDataIsEnabled(metaData)) {
                entries.push_back(doGetEntryInfo(metaData, entryIndex));
            }
        }
        return entries;
    }

    std::vector<EntryInfo>
    FolderEntry::doListEntriesBasedOnType(EntryType entryType) const
    {
        std::vector<EntryInfo> entries;
        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {
            // only push back if the metadata is enabled
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);

            if (doEntryMetaDataIsEnabled(metaData) &&
                doGetTypeForEntry(metaData) == entryType) {
                entries.push_back(doGetEntryInfo(metaData, entryIndex));
            }
        }
        return entries;
    }

    std::vector<EntryInfo>
    FolderEntry::listFileEntries() const
    {
        return doListEntriesBasedOnType(EntryType::FileType);
    }

    std::vector<EntryInfo>
    FolderEntry::listFolderEntries() const
    {
        return doListEntriesBasedOnType(EntryType::FolderType);
    }

    void
    FolderEntry::doPutMetaDataOutOfUse(std::string const &name)
    {
        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        FileEntry temp(m_io, m_name, m_startVolumeBlock,
                       OpenDisposition::buildOverwriteDisposition());
        detail::putMetaDataOutOfUse(temp, doGetMetaDataIndexForEntry(name));
    }

    void
    FolderEntry::putMetaDataOutOfUse(std::string const &name)
    {
        this->doPutMetaDataOutOfUse(name);
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
        this->doPutMetaDataOutOfUse(name);
    }

    void
    FolderEntry::removeFolderEntry(std::string const &name)
    {
        FolderEntry entry = getFolderEntry(name);

        // loop over entries unlinking files and recursing into sub folders
        // and deleting their entries
        std::vector<EntryInfo> infos = entry.listAllEntries();
        std::vector<EntryInfo>::iterator it = infos.begin();
        for (; it != infos.end(); ++it) {
            if (it->type() == EntryType::FileType) {
                entry.removeFileEntry(it->filename());
            } else {
                entry.removeFolderEntry(it->filename());
            }
        }

        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        this->doPutMetaDataOutOfUse(name);

        // unlink entry's data
        entry.m_folderData.unlink();
    }

    OptionalEntryInfo
    FolderEntry::getEntryInfo(std::string const &name) const
    {
        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {

            // read all metadata
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);
            if (doEntryMetaDataIsEnabled(metaData)) {
                EntryInfo info = doGetEntryInfo(metaData, entryIndex);
                if(info.filename() == name) {
                    return OptionalEntryInfo(info);
                }
            }
        }
        return OptionalEntryInfo();
    }

    EntryInfo
    FolderEntry::getEntryInfo(uint64_t const entryIndex) const
    {
        std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);
        return doGetEntryInfo(metaData, entryIndex);
    }

    EntryInfo
    FolderEntry::doGetEntryInfo(std::vector<uint8_t> const &metaData, uint64_t const entryIndex) const
    {
        std::string const entryName = doGetEntryName(metaData);
        EntryType const entryType = doGetTypeForEntry(metaData);
        uint64_t fileSize = 0;
        uint64_t startBlock;
        if (entryType == EntryType::FileType) {
            // note disposition doesn't matter here, can be anything
            uint64_t n = doGetBlockIndexForEntry(metaData);
            FileEntry fe(m_io, entryName, n, OpenDisposition::buildAppendDisposition());
            fileSize = fe.fileSize();
            startBlock = fe.getStartVolumeBlockIndex();
        } else {
            uint64_t n = doGetBlockIndexForEntry(metaData);
            FolderEntry fe(m_io, n, entryName);
            startBlock = fe.m_folderData.getStartVolumeBlockIndex();
        }

        return EntryInfo(entryName,
                         fileSize,
                         entryType,
                         true, // writable
                         startBlock,
                         entryIndex);
    }

    uint64_t
    FolderEntry::doGetNumberOfEntries() const
    {
        bfs::BFSImageStream out(m_io, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = detail::getOffsetOfFileBlock(m_folderData.getStartVolumeBlockIndex(), m_io.blocks);
        (void)out.seekg(offset + detail::FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        out.close();
        return detail::convertInt8ArrayToInt64(buf);
    }



    bool
    FolderEntry::doEntryMetaDataIsEnabled(std::vector<uint8_t> const &bytes) const
    {
        uint8_t byte = bytes[0];
        return detail::isBitSetInByte(byte, 0);
    }

    EntryType
    FolderEntry::doGetTypeForEntry(std::vector<uint8_t> const &bytes) const
    {
        uint8_t byte = bytes[0];
        return (detail::isBitSetInByte(byte, 1) ? EntryType::FileType : EntryType::FolderType);
    }

    std::string
    FolderEntry::doGetEntryName(uint64_t const n) const
    {
        std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, n);
        return doGetEntryName(metaData);
    }

    std::string
    FolderEntry::doGetEntryName(std::vector<uint8_t> const &metaData) const
    {
        std::string nameDat(metaData.begin() + 1, metaData.end() - 8);
        std::string returnString;
        int c = 0;
        while (nameDat[c] != '\0') {
            returnString.push_back(nameDat[c]);
            ++c;
        }
        return returnString;
    }

    uint64_t
    FolderEntry::doGetMetaDataIndexForEntry(std::string const &name) const
    {
        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {
            if (name == doGetEntryName(entryIndex)) {
                return entryIndex;
            }
        }
        throw std::runtime_error("Folder entry with that name not found");
    }

    uint64_t
    FolderEntry::doGetBlockIndexForEntry(std::vector<uint8_t> const &metaData) const
    {
        std::vector<uint8_t> theBuffer(metaData.begin() + detail::MAX_FILENAME_LENGTH + 1, metaData.end());
        return detail::convertInt8ArrayToInt64(&theBuffer.front());
    }

    OptionalOffset
    FolderEntry::doFindOffsetWhereMetaDataShouldBeWritten()
    {

        // loop over all entries and try and find a previously deleted one
        // If its deleted, the first bit of the entry metadata will be unset

        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);
            if (!doEntryMetaDataIsEnabled(metaData)) {
                uint32_t bufferSize = 1 + detail::MAX_FILENAME_LENGTH + 8;
                std::ios_base::streamoff offset = (8 + (entryIndex * bufferSize));
                return OptionalOffset(offset);
            }
        }

        // free entry not found so signify that we should seek right to
        // end by returning an empty optional
        return OptionalOffset();
    }

}
