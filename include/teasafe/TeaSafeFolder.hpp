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

#ifndef TeaSafe_TEASAFE_FOLDER_HPP__
#define TeaSafe_TEASAFE_FOLDER_HPP__

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/EntryInfo.hpp"
#include "teasafe/TeaSafeFile.hpp"

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>

#include <map>

namespace teasafe
{

    typedef boost::optional<std::ios_base::streamoff> OptionalOffset;
    typedef boost::shared_ptr<EntryInfo> SharedEntryInfo;

    class TeaSafeFolder
    {
      public:
        /**
         * @brief constructs a TeaSafeFolder to write to. In this case the
         * starting block is unknown
         * @param the core teasafe io (path, blocks, password)
         * @param startBlock the index of the starting file block making up entry data
         * @param writable if data entries can be added to folder
         * @param name the name of the entry
         * @param enforceRootBlock true if we want to enforce the starting root block
         */
        TeaSafeFolder(SharedCoreIO const &io,
                      std::string const &name = "root",
                      bool const enforceRootBlock = false);

        /**
         * @brief constructs a TeaSafeFolder to read from
         * @param the core teasafe io (path, blocks, password)
         * @param startBlock the index of the starting file block making up entry data
         * @param writable if data entries can be added to folder
         * @param name the name of the entry
         */
        TeaSafeFolder(SharedCoreIO const &io,
                      uint64_t const startBlock,
                      std::string const &name = "root");


        /**
         * @brief appends a new file entry and start index to the entry data
         * @param name the name of the entry
         * @return a copy of a TeaSafeFile that will be used to reference the file data
         */
        void addTeaSafeFile(std::string const &name);

        /**
         * @brief appends a new folder entry and start index of the entry data
         * @param name the name of the entry
         * @return a copy of a TeaSafeFolder that will be used to reference the folder data
         */
        void addTeaSafeFolder(std::string const &name);

        /**
         * @brief retrieves a TeaSafeFile with specific name
         * @param name the name of the entry to lookup
         * @param openDisposition open mode
         * @return a copy of the TeaSafeFile with name
         */
        TeaSafeFile getTeaSafeFile(std::string const &name,
                                   OpenDisposition const &openDisposition) const;

        /**
         * @brief retrieves a TeaSafeFolder with specific name
         * @param name the name of the entry to lookup
         * @return a copy of the TeaSafeFolder with name
         */
        TeaSafeFolder getTeaSafeFolder(std::string const &name) const;

        /**
         * @brief retrieves the name of this folder
         * @return the name
         */
        std::string getName() const;

        SharedImageStream getStream() const;


        /**
         * @brief retrieves the entry info attributes of a given indexed entry
         * @param index the index of the entry to get the attributes of
         * @return the entry information
         */
        EntryInfo getEntryInfo(uint64_t const index) const;


        /**
         * @brief retrieves an entry info it exists
         * @param name the name of the info
         * @return an entry info if it exsist
         */
        SharedEntryInfo getEntryInfo(std::string const &name) const;


        /**
         * @brief  a private version of getEntryInfo
         * @see    getEntryInfo
         * @param  name
         * @return entryInfo
         */
        SharedEntryInfo doGetNamedEntryInfo(std::string const &name) const;

        /**
         * @brief retrieves an entry info of a file if it exists
         * @param name the name of the info
         * @return an entry info if it exsist
         */
        SharedEntryInfo getFolderEntryInfo(std::string const &name) const;

        /**
         * @brief returns a vector of all entry infos
         * @return all entry infos
         */
        std::vector<EntryInfo> listAllEntries() const;

        /**
         * @brief returns a vector of file entry infos
         * @return all file entry infos
         */
        std::vector<EntryInfo> listFileEntries() const;

        /**
         * @brief returns a vector of folder entry infos
         * @return all folder entry infos
         */
        std::vector<EntryInfo> listFolderEntries() const;

        /**
         * @brief does what it says
         * @param name the name of the entry
         */
        void removeTeaSafeFile(std::string const &name);

        /**
         * @brief removes a sub folder and all its content
         * @param name the name of the entry
         */
        void removeTeaSafeFolder(std::string const &name);

        /**
         * @brief puts metadata for given entry out of use
         * @param name name of entry
         */
        void putMetaDataOutOfUse(std::string const &name);

        /**
         * @brief for writing new entry metadata
         * @param name name of entry
         * @param entryType the type of the entry
         * @param startBlock start block of entry
         */
        void writeNewMetaDataForEntry(std::string const &name,
                                      EntryType const& entryType,
                                      uint64_t startBlock);

      private:

        TeaSafeFolder();

        /**
         * @brief for writing new entry metadata
         * @param name name of entry
         * @param entryType the type of the entry
         * @param startBlock start block of entry
         */
        void doWriteNewMetaDataForEntry(std::string const &name,
                                        EntryType const& entryType,
                                        uint64_t startBlock);

        /**
         * @brief a private accessor for getting file entry from metadata
         * @param metaData the entry metadata
         * @param index the index of the entry
         * @return the info metadata in entry info struct
         */
        SharedEntryInfo doGetEntryInfo(std::vector<uint8_t> const &metaData, uint64_t const index) const;

        /**
         * @brief puts metadata for given entry out of use
         * @param name name of entry
         */
        void doPutMetaDataOutOfUse(std::string const &name);

        /**
         * @brief retrieves the starting block index of a given entry
         * @param metaData the metadata that contains the block index
         * @return the starting block index
         */
        uint64_t doGetBlockIndexForEntry(std::vector<uint8_t> const &metaData) const;

        /**
         * @brief retrieves the number of entries in folder entry
         * @return the number of folder entries
         */
        uint64_t doGetNumberOfEntries() const;

        /**
         * @brief retrieves the name of an entry with given index
         * @return the name
         */
        std::string doGetEntryName(uint64_t const index) const;

        /**
         * @brief returns the entry index given the name
         * @param name the name of the entry
         * @return the index
         */
        uint64_t doGetMetaDataIndexForEntry(std::string const &name) const;

        /**
         * @brief write metadata to this folder entry
         * @note assumes in correct position
         * @param buf
         * @param n
         * @return numberof bytes written
         */
        std::streamsize doWrite(char const * buf, std::streampos n);

        /**
         * @brief write the first byte of the file metadata data
         * @note assumes in correct position
         * @return
         */
        std::streamsize doWriteFirstByteToEntryMetaData(EntryType const &entryType);

        /**
         * @brief write filename file metadata
         * @return number of bytes writeen
         */
        std::streamsize doWriteFilenameToEntryMetaData(std::string const &name);

        /**
         * @brief writes the first block index bytes to the file metadata
         * @param first block, the block index of the file entry
         * @return number of bytes written
         */
        std::streamsize doWriteFirstBlockIndexToEntryMetaData(uint64_t firstBlock);

        /**
         * @brief retrieves the type of a given entry
         * @param metaData the metadata that contains the block index
         * @return the type of the entry
         */
        EntryType doGetTypeForEntry(std::vector<uint8_t> const &metaData) const;

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
        bool doEntryMetaDataIsEnabled(std::vector<uint8_t> const &metaData) const;

        /**
         * @brief private accessor for getting entry name from metadata
         * @param metaData the metadata
         * @return name extracted from metadata
         */
        std::string doGetEntryName(std::vector<uint8_t> const &metaData) const;

        /**
         * @brief seeks to where the metadata should be written. If
         * metadata for a previous entry has been deleted, we should
         * use that position instead to write new data
         * @return true if pre-existing metadata is to be overwritten,
         * false otherwise
         */
        OptionalOffset
        doFindOffsetWhereMetaDataShouldBeWritten();

        /**
         * @brief lists a particular type of entry, file or folder
         * @return a list of entries of specified type
         */
        std::vector<EntryInfo> doListEntriesBasedOnType(EntryType entryType) const;

        /**
         * @brief after unlinking this will be called to indicate that cache entry
         * should be removed
         * @param name entry name
         */
        void invalidateEntryInEntryInfoCache(std::string const &name);

        // the core teasafe io (path, blocks, password)
        SharedCoreIO m_io;

        // the underlying file blocks storing the folder entry data
        mutable TeaSafeFile m_folderData;

        uint64_t m_startVolumeBlock;

        // stores the name of this folder
        std::string m_name;

        // as an optimization, store the number of entries so that we
        // don't have to read each time (read once during construction)
        uint64_t m_entryCount;

        // An experimental optimization: a map will store entry infos as they
        // are generated so that in future, they don't have to be regenerated.
        // Question: when to invalidate/update an entry in the cache?
        typedef std::map<std::string, SharedEntryInfo> EntryInfoCacheMap;
        mutable EntryInfoCacheMap m_entryInfoCacheMap;

    };

}



#endif // TeaSafe_TEASAFE_FOLDER_HPP__
