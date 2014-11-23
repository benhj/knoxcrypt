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
  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software without
  specific prior written permission.

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

#include <algorithm>
#include <iterator>
#include <functional>
#include <stdexcept>

namespace teasafe
{

    namespace {
        /**
         * @brief put a metadata section out of use by unsetting the first bit
         * @param folderData the data that stores the folder metadata
         * @param n the metadata chunk to put out of use
         */
        void metaDataToOutOfUse(TeaSafeFile folderData, int n)
        {
            uint32_t bufferSize = 1 + detail::MAX_FILENAME_LENGTH + 8;
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
                metaData.resize(bufSize == 0 ? bufferSize : bufSize);
                folderData.read((char*)&metaData.front(), bufSize==0 ? bufferSize : bufSize);
                return metaData;
            }
            throw std::runtime_error("Problem retrieving metadata");
        }

        std::vector<uint8_t> createFileNameVector(std::string const &name)
        {
            std::vector<uint8_t> filename;
            filename.assign(detail::MAX_FILENAME_LENGTH, 0);
            (void)std::copy(&name.front(), &name.front() + name.length(), &filename.front());
            filename[name.length()] = '\0'; // set null byte to indicate end of filename
            return filename;
        }

        /**
         * @brief determines if entry metadata is enabled. Entry metadata
         * consists of one byte, the first bit of which determines if the
         * metadata is in use or not. It won't be in use if the entry
         * associated with the metadata has been deleted in whcih case
         * this associated metadata can be overwritten when creating
         * a new entry.
         * @param metaData the metadata
         * @return a value indicating if specified entry metadata is in use
         */
        bool entryMetaDataIsEnabled(std::vector<uint8_t> const &bytes) 
        {
            uint8_t byte = bytes[0];
            return detail::isBitSetInByte(byte, 0);
        }

        /**
         * @brief retrieves the starting block index of a given entry
         * @param metaData the metadata that contains the block index
         * @return the starting block index
         */
        uint64_t getBlockIndexForEntry(std::vector<uint8_t> const &metaData) 
        {
            std::vector<uint8_t> theBuffer(metaData.begin() + detail::MAX_FILENAME_LENGTH + 1, metaData.end());
            return detail::convertInt8ArrayToInt64(&theBuffer.front());
        }

        /**
         * @brief retrieves the type of a given entry
         * @param metaData the metadata that contains the block index
         * @return the type of the entry
         */
        EntryType getTypeForEntry(std::vector<uint8_t> const &bytes) 
        {
            uint8_t byte = bytes[0];
            return (detail::isBitSetInByte(byte, 1) ? EntryType::FileType : EntryType::FolderType);
        }

        /**
         * @brief private accessor for getting entry name from metadata
         * @param metaData the metadata
         * @return name extracted from metadata
         */
        std::string getEntryName(std::vector<uint8_t> const &metaData) 
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

        /**
         * @brief retrieves the name of an entry with given index
         * @return the name
         */
        std::string getEntryName(TeaSafeFile const & folderData, uint64_t const n)
        {
            auto metaData(doSeekAndReadOfEntryMetaData(folderData, n));
            return getEntryName(metaData);
        }

        /**
         * @brief retrieves the number of entries in folder entry
         * @return the number of folder entries
         */
        long getNumberOfEntries(TeaSafeFile const & folderData, uint64_t const blocks)
        {
            auto out(folderData.getStream());
            uint64_t const offset = detail::getOffsetOfFileBlock(folderData.getStartVolumeBlockIndex(), blocks);
            (void)out->seekg(offset + detail::FILE_BLOCK_META);
            if(!out->bad()) { // bad when not initialized, i.e., when sparse image
                uint8_t buf[8];
                (void)out->read((char*)buf, 8);

                // there will never be a number of entries that is greater than
                // the max capacity of a long variable
                return static_cast<long>(detail::convertInt8ArrayToInt64(buf));
            }
            return 0; // block not yet initialized (in case of sparse image)
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
        , m_entryCount(getNumberOfEntries(m_folderData, m_io->blocks))
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
        auto filename(createFileNameVector(name));

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
        auto overWroteOld(doFindOffsetWhereMetaDataShouldBeWritten());

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

        // increment entry count, but only if brand new
        if (!overWroteOld) {
            detail::incrementFolderEntryCount(*m_folderData.getStream(),
                                              m_io,
                                              m_folderData.getStartVolumeBlockIndex());
            ++m_entryCount;
        }

        // make sure all data has been written
        m_folderData.flush();
    }

    SharedImageStream
    TeaSafeFolder::getStream() const
    {
        return m_folderData.getStream();
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
        auto info(doGetNamedEntryInfo(name));
        if (info) {
            if (info->type() == EntryType::FileType) {
                TeaSafeFile file(m_io, name, info->firstFileBlock(), openDisposition);
                file.setOptionalSizeUpdateCallback(std::bind(&EntryInfo::updateSize, 
                                                             info, 
                                                             std::placeholders::_1));
                return file;
            }
        }
        throw std::runtime_error("File entry with that name not found");
    }

    TeaSafeFolder
    TeaSafeFolder::getTeaSafeFolder(std::string const &name) const
    {
        // optimization is to build the file based on metadata stored in the
        // entry info which is hopefully cached
        auto info(doGetNamedEntryInfo(name));
        if (info) {
            if (info->type() == EntryType::FolderType) {
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
            auto metaData(doSeekAndReadOfEntryMetaData(m_folderData, entryIndex));
            if (entryMetaDataIsEnabled(metaData)) {
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
            auto metaData(doSeekAndReadOfEntryMetaData(m_folderData, entryIndex));

            if (entryMetaDataIsEnabled(metaData) &&
                getTypeForEntry(metaData) == entryType) {
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
        metaDataToOutOfUse(temp, doGetMetaDataIndexForEntry(name));

        // removes any info with name from cache
        this->invalidateEntryInEntryInfoCache(name);
    }

    void
    TeaSafeFolder::putMetaDataOutOfUse(std::string const &name)
    {
        this->doPutMetaDataOutOfUse(name);
    }

    void
    TeaSafeFolder::invalidateEntryInEntryInfoCache(std::string const &name)
    {
        auto it(m_entryInfoCacheMap.find(name));
        if (it != m_entryInfoCacheMap.end()) {
            m_entryInfoCacheMap.erase(it);
        }
    }

    void
    TeaSafeFolder::removeTeaSafeFile(std::string const &name)
    {
        // first unlink; this deallocates the file blocks, updating the
        // volume bitmap accordingly; note doesn't matter what opendisposition is here
        auto entry(getTeaSafeFile(name, OpenDisposition::buildAppendDisposition()));
        entry.unlink();

        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        this->doPutMetaDataOutOfUse(name);

    }

    void
    TeaSafeFolder::removeTeaSafeFolder(std::string const &name)
    {
        auto entry(getTeaSafeFolder(name));

        // loop over entries unlinking files and recursing into sub folders
        // and deleting their entries
        auto infos(entry.listAllEntries());
        for (auto const &it : infos) {
            if (it.type() == EntryType::FileType) {
                entry.removeTeaSafeFile(it.filename());
            } else {
                entry.removeTeaSafeFolder(it.filename());
            }
        }

        // second set the metadata to an out of use state; this metadata can
        // then be later overwritten when a new entry is then added
        this->doPutMetaDataOutOfUse(name);

        // unlink entry's data
        entry.m_folderData.unlink();
    }

    SharedEntryInfo
    TeaSafeFolder::getEntryInfo(std::string const &name) const
    {
        return this->doGetNamedEntryInfo(name);
    }

    SharedEntryInfo
    TeaSafeFolder::doGetNamedEntryInfo(std::string const &name) const
    {

        // try and pul out of cache fisrt
        auto it(m_entryInfoCacheMap.find(name));
        if (it != m_entryInfoCacheMap.end()) {
            return it->second;
        }

        // wasn't in cache so need to build
        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {

            // read all metadata
            auto metaData(doSeekAndReadOfEntryMetaData(m_folderData, entryIndex));
            if (entryMetaDataIsEnabled(metaData)) {
                auto info(doGetEntryInfo(metaData, entryIndex));
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
        auto metaData(doSeekAndReadOfEntryMetaData(m_folderData, entryIndex));
        return *doGetEntryInfo(metaData, entryIndex);
    }

    SharedEntryInfo
    TeaSafeFolder::doGetEntryInfo(std::vector<uint8_t> const &metaData, uint64_t const entryIndex) const
    {

        auto const entryName(getEntryName(metaData));

        // experimental optimization; insert info in to cache
        auto it(m_entryInfoCacheMap.find(entryName));
        if (it != m_entryInfoCacheMap.end()) {
            return it->second;
        }

        auto const entryType(getTypeForEntry(metaData));
        uint64_t fileSize = 0;
        uint64_t startBlock;
        if (entryType == EntryType::FileType) {
            // note disposition doesn't matter here, can be anything
            uint64_t n = getBlockIndexForEntry(metaData);
            TeaSafeFile fe(m_io, entryName, n, OpenDisposition::buildAppendDisposition());
            fileSize = fe.fileSize();
            startBlock = n;
        } else {
            uint64_t n = getBlockIndexForEntry(metaData);
            TeaSafeFolder fe(m_io, n, entryName);
            startBlock = n;
        }

        auto info(std::make_shared<EntryInfo>(entryName,
                                              fileSize,
                                              entryType,
                                              true, // writable
                                              startBlock,
                                              entryIndex));

        m_entryInfoCacheMap.insert(std::make_pair(entryName, info));

        return info;
    }

    long
    TeaSafeFolder::doGetMetaDataIndexForEntry(std::string const &name) const
    {
        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {
            if (name == getEntryName(m_folderData, entryIndex)) {
                return entryIndex;
            }
        }
        throw std::runtime_error("Folder entry with that name not found");
    }


    OptionalOffset
    TeaSafeFolder::doFindOffsetWhereMetaDataShouldBeWritten()
    {

        // loop over all entries and try and find a previously deleted one
        // If its deleted, the first bit of the entry metadata will be unset

        // Note possible way of optimizing this? Store in cache available entries
        // to overwrite?

        for (long entryIndex = 0; entryIndex < m_entryCount; ++entryIndex) {
            auto metaData(doSeekAndReadOfEntryMetaData(m_folderData, entryIndex));
            if (!entryMetaDataIsEnabled(metaData)) {
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
