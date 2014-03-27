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
*/

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/TeaSafeFolder.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "teasafe/detail/DetailFolder.hpp"

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <stdexcept>

namespace teasafe
{

    namespace detail
    {
        /**
         * @brief put a metadata section out of use by unsetting the first bit
         * @param folderData the data that stores the folder metadata
         * @param n the metadata chunk to put out of use
         */
        void putMetaDataOutOfUse(TeaSafeFile folderData, int n)
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
        std::vector<uint8_t> doSeekAndReadOfEntryMetaData(TeaSafeFile folderData,
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

    TeaSafeFolder::TeaSafeFolder(SharedCoreIO const &io,
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
        , m_entryInfoCacheMap()
    {
    }

    TeaSafeFolder::TeaSafeFolder(SharedCoreIO const &io,
                                 std::string const &name,
                                 bool const enforceRootBlock)
        : m_io(io)
        , m_folderData(m_io, name, enforceRootBlock)
        , m_startVolumeBlock(m_folderData.getStartVolumeBlockIndex())
        , m_name(name)
        , m_entryCount(0)
        , m_entryInfoCacheMap()
    {
        // set initial number of entries; there will be none to begin with
        uint64_t startCount(0);
        uint8_t buf[8];
        detail::convertUInt64ToInt8Array(startCount, buf);
        (void)m_folderData.write((char*)buf, 8);
        m_folderData.flush();
    }

    std::streamsize
    TeaSafeFolder::doWrite(char const * buf, std::streampos n)
    {
        return m_folderData.write(buf, n);
    }

    std::streamsize
    TeaSafeFolder::doWriteFirstByteToEntryMetaData(EntryType const &entryType)
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
    TeaSafeFolder::doWriteFilenameToEntryMetaData(std::string const &name)
    {
        // create a vector to hold filename
        std::vector<uint8_t> filename = detail::createFileNameVector(name);

        // write out filename
        return doWrite((char*)&filename.front(), detail::MAX_FILENAME_LENGTH);
    }

    std::streamsize
    TeaSafeFolder::doWriteFirstBlockIndexToEntryMetaData(uint64_t firstBlock)
    {
        // create bytes to represent first block
        uint8_t buf[8];
        detail::convertUInt64ToInt8Array(firstBlock, buf);
        return doWrite((char*)buf, 8);
    }

    void
    TeaSafeFolder::writeNewMetaDataForEntry(std::string const &name,
                                            EntryType const &entryType,
                                            uint64_t startBlock)
    {
        this->doWriteNewMetaDataForEntry(name, entryType, startBlock);
    }

    void
    TeaSafeFolder::doWriteNewMetaDataForEntry(std::string const &name,
                                              EntryType const &entryType,
                                              uint64_t startBlock)
    {
        OptionalOffset overWroteOld = doFindOffsetWhereMetaDataShouldBeWritten();

        if (overWroteOld) {
            m_folderData = TeaSafeFile(m_io, m_name, m_startVolumeBlock,
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
    TeaSafeFolder::addTeaSafeFile(std::string const &name)
    {
        // Create a new file entry
        TeaSafeFile entry(m_io, name);

        // write the first block index to the file entry metadata
        this->doWriteNewMetaDataForEntry(name, EntryType::FileType, entry.getStartVolumeBlockIndex());
    }

    void
    TeaSafeFolder::addTeaSafeFolder(std::string const &name)
    {
        // Create a new sub-folder entry
        TeaSafeFolder entry(m_io, name);

        // write the first block index to the file entry metadata
        this->doWriteNewMetaDataForEntry(name, EntryType::FolderType, entry.m_folderData.getStartVolumeBlockIndex());
    }

    TeaSafeFile
    TeaSafeFolder::getTeaSafeFile(std::string const &name,
                                  OpenDisposition const &openDisposition) const
    {

        // optimization is to build the file based on metadata stored in the
        // entry info which is hopefully cached
        SharedEntryInfo info(doGetNamedEntryInfo(name));
        if(info) {
            if(info->type() == EntryType::FileType) {
                TeaSafeFile file(m_io, name, info->firstFileBlock(), openDisposition);
                file.setOptionalSizeUpdateCallback(boost::bind(&EntryInfo::updateSize, info, _1));
                return file;
            }
        }
        throw std::runtime_error("File entry with that name not found");
    }

    TeaSafeFolder
    TeaSafeFolder::getTeaSafeFolder(std::string const &name) const
    {

        uint64_t i(0);

        // optimization is to build the file based on metadata stored in the
        // entry info which is hopefully cached
        SharedEntryInfo info(doGetNamedEntryInfo(name));
        if(info) {
            if(info->type() == EntryType::FolderType) {
                return TeaSafeFolder(m_io, info->firstFileBlock(), name);
            }
        }
        throw std::runtime_error("Folder entry with that name not found");
    }

    std::string
    TeaSafeFolder::getName() const
    {
        return m_name;
    }

    std::vector<EntryInfo>
    TeaSafeFolder::listAllEntries() const
    {
        std::vector<EntryInfo> entries;

        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {

            // read all metadata
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);
            if (doEntryMetaDataIsEnabled(metaData)) {
                entries.push_back(*doGetEntryInfo(metaData, entryIndex));
            }
        }
        return entries;
    }

    std::vector<EntryInfo>
    TeaSafeFolder::doListEntriesBasedOnType(EntryType entryType) const
    {
        std::vector<EntryInfo> entries;
        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {
            // only push back if the metadata is enabled
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);

            if (doEntryMetaDataIsEnabled(metaData) &&
                doGetTypeForEntry(metaData) == entryType) {
                entries.push_back(*doGetEntryInfo(metaData, entryIndex));
            }
        }
        return entries;
    }

    std::vector<EntryInfo>
    TeaSafeFolder::listFileEntries() const
    {
        return doListEntriesBasedOnType(EntryType::FileType);
    }

    std::vector<EntryInfo>
    TeaSafeFolder::listFolderEntries() const
    {
        return doListEntriesBasedOnType(EntryType::FolderType);
    }

    void
    TeaSafeFolder::doPutMetaDataOutOfUse(std::string const &name)
    {
        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        TeaSafeFile temp(m_io, m_name, m_startVolumeBlock,
                         OpenDisposition::buildOverwriteDisposition());
        detail::putMetaDataOutOfUse(temp, doGetMetaDataIndexForEntry(name));
    }

    void
    TeaSafeFolder::putMetaDataOutOfUse(std::string const &name)
    {
        this->doPutMetaDataOutOfUse(name);
    }

    void
    TeaSafeFolder::invalidateEntryInEntryInfoCache(std::string const &name)
    {
        EntryInfoCacheMap::iterator it = m_entryInfoCacheMap.find(name);
        if(it != m_entryInfoCacheMap.end()) {
            m_entryInfoCacheMap.erase(it);
        }
    }

    void
    TeaSafeFolder::removeTeaSafeFile(std::string const &name)
    {
        // first unlink; this deallocates the file blocks, updating the
        // volume bitmap accordingly; note doesn't matter what opendisposition is here
        TeaSafeFile entry = getTeaSafeFile(name, OpenDisposition::buildAppendDisposition());
        entry.unlink();

        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        this->doPutMetaDataOutOfUse(name);

        // removes any info with name from cache
        this->invalidateEntryInEntryInfoCache(name);

    }

    void
    TeaSafeFolder::removeTeaSafeFolder(std::string const &name)
    {
        TeaSafeFolder entry = getTeaSafeFolder(name);

        // loop over entries unlinking files and recursing into sub folders
        // and deleting their entries
        std::vector<EntryInfo> infos = entry.listAllEntries();
        std::vector<EntryInfo>::iterator it = infos.begin();
        for (; it != infos.end(); ++it) {
            if (it->type() == EntryType::FileType) {
                entry.removeTeaSafeFile(it->filename());
            } else {
                entry.removeTeaSafeFolder(it->filename());
            }
        }

        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        this->doPutMetaDataOutOfUse(name);

        // unlink entry's data
        entry.m_folderData.unlink();

        // removes any info with name from cache
        this->invalidateEntryInEntryInfoCache(name);
    }

    SharedEntryInfo
    TeaSafeFolder::getEntryInfo(std::string const &name) const
    {
        return this->doGetNamedEntryInfo(name);
    }

    SharedEntryInfo
    TeaSafeFolder::doGetNamedEntryInfo(std::string const &name) const
    {
        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {

            // read all metadata
            std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);
            if (doEntryMetaDataIsEnabled(metaData)) {
                SharedEntryInfo info = doGetEntryInfo(metaData, entryIndex);
                if (info->filename() == name) {
                    return info;
                }
            }
        }
        return SharedEntryInfo();
    }

    EntryInfo
    TeaSafeFolder::getEntryInfo(uint64_t const entryIndex) const
    {
        std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, entryIndex);
        return *doGetEntryInfo(metaData, entryIndex);
    }

    SharedEntryInfo
    TeaSafeFolder::doGetEntryInfo(std::vector<uint8_t> const &metaData, uint64_t const entryIndex) const
    {
        std::string const entryName = doGetEntryName(metaData);

        // experimental optimization; insert info in to cache
        EntryInfoCacheMap::const_iterator it = m_entryInfoCacheMap.find(entryName);
        if(it != m_entryInfoCacheMap.end()) {
            return it->second;
        }

        EntryType const entryType = doGetTypeForEntry(metaData);
        uint64_t fileSize = 0;
        uint64_t startBlock;
        if (entryType == EntryType::FileType) {
            // note disposition doesn't matter here, can be anything
            uint64_t n = doGetBlockIndexForEntry(metaData);
            TeaSafeFile fe(m_io, entryName, n, OpenDisposition::buildAppendDisposition());
            fileSize = fe.fileSize();
            startBlock = n;
        } else {
            uint64_t n = doGetBlockIndexForEntry(metaData);
            TeaSafeFolder fe(m_io, n, entryName);
            startBlock = n;
        }

        SharedEntryInfo info(boost::make_shared<EntryInfo>(entryName,
                                                           fileSize,
                                                           entryType,
                                                           true, // writable
                                                           startBlock,
                                                           entryIndex));

        m_entryInfoCacheMap.insert(std::make_pair(entryName, info));

        return info;
    }

    uint64_t
    TeaSafeFolder::doGetNumberOfEntries() const
    {
        teasafe::TeaSafeImageStream out(m_io, std::ios::in | std::ios::out | std::ios::binary);
        uint64_t const offset = detail::getOffsetOfFileBlock(m_folderData.getStartVolumeBlockIndex(), m_io->blocks);
        (void)out.seekg(offset + detail::FILE_BLOCK_META);
        uint8_t buf[8];
        (void)out.read((char*)buf, 8);
        out.close();
        return detail::convertInt8ArrayToInt64(buf);
    }

    bool
    TeaSafeFolder::doEntryMetaDataIsEnabled(std::vector<uint8_t> const &bytes) const
    {
        uint8_t byte = bytes[0];
        return detail::isBitSetInByte(byte, 0);
    }

    EntryType
    TeaSafeFolder::doGetTypeForEntry(std::vector<uint8_t> const &bytes) const
    {
        uint8_t byte = bytes[0];
        return (detail::isBitSetInByte(byte, 1) ? EntryType::FileType : EntryType::FolderType);
    }

    std::string
    TeaSafeFolder::doGetEntryName(uint64_t const n) const
    {
        std::vector<uint8_t> metaData = detail::doSeekAndReadOfEntryMetaData(m_folderData, n);
        return doGetEntryName(metaData);
    }

    std::string
    TeaSafeFolder::doGetEntryName(std::vector<uint8_t> const &metaData) const
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
    TeaSafeFolder::doGetMetaDataIndexForEntry(std::string const &name) const
    {
        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {
            if (name == doGetEntryName(entryIndex)) {
                return entryIndex;
            }
        }
        throw std::runtime_error("Folder entry with that name not found");
    }

    uint64_t
    TeaSafeFolder::doGetBlockIndexForEntry(std::vector<uint8_t> const &metaData) const
    {
        std::vector<uint8_t> theBuffer(metaData.begin() + detail::MAX_FILENAME_LENGTH + 1, metaData.end());
        return detail::convertInt8ArrayToInt64(&theBuffer.front());
    }

    OptionalOffset
    TeaSafeFolder::doFindOffsetWhereMetaDataShouldBeWritten()
    {

        // loop over all entries and try and find a previously deleted one
        // If its deleted, the first bit of the entry metadata will be unset

        // Note possible way of optimizing this? Store in cache available entries
        // to overwrite?

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
