#ifndef BFS_FOLDER_ENTRY_HPP__
#define BFS_FOLDER_ENTRY_HPP__

#include "bfs/CoreBFSIO.hpp"
#include "bfs/EntryInfo.hpp"
#include "bfs/FileEntry.hpp"

#include <boost/optional.hpp>

namespace bfs
{

    typedef boost::optional<EntryInfo> OptionalEntryInfo;

    class FolderEntry
    {
      public:
        /**
         * @brief constructs a FolderEntry to write to. In this case the
         * starting block is unknown
         * @param the core bfs io (path, blocks, password)
         * @param startBlock the index of the starting file block making up entry data
         * @param writable if data entries can be added to folder
         * @param name the name of the entry
         */
        FolderEntry(CoreBFSIO const &io,
                    std::string const &name = "root");

        /**
         * @brief constructs a FolderEntry to read from
         * @param the core bfs io (path, blocks, password)
         * @param startBlock the index of the starting file block making up entry data
         * @param writable if data entries can be added to folder
         * @param name the name of the entry
         */
        FolderEntry(CoreBFSIO const &io,
                    uint64_t const startBlock,
                    std::string const &name = "root");


        /**
         * @brief appends a new file entry and start index to the entry data
         * @param name the name of the entry
         * @return a copy of a FileEntry that will be used to reference the file data
         */
        void addFileEntry(std::string const &name);

        /**
         * @brief appends a new folder entry and start index of the entry data
         * @param name the name of the entry
         * @return a copy of a FolderEntry that will be used to reference the folder data
         */
        void addFolderEntry(std::string const &name);

        /**
         * @brief retrieves a FileEntry with specific name
         * @param name the name of the entry to lookup
         * @param openDisposition open mode
         * @return a copy of the FileEntry with name
         */
        FileEntry getFileEntry(std::string const &name,
                               OpenDisposition const &openDisposition) const;

        /**
         * @brief retrieves a FolderEntry with specific name
         * @param name the name of the entry to lookup
         * @return a copy of the FolderEntry with name
         */
        FolderEntry getFolderEntry(std::string const &name) const;

        /**
         * @brief retrieves the name of this folder
         * @return the name
         */
        std::string getName() const;


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
        OptionalEntryInfo getEntryInfo(std::string const &name) const;

        /**
         * @brief retrieves an entry info of a file if it exists
         * @param name the name of the info
         * @return an entry info if it exsist
         */
        OptionalEntryInfo getFolderEntryInfo(std::string const &name) const;

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
        void removeFileEntry(std::string const &name);

        /**
         * @brief removes a sub folder and all its content
         * @param name the name of the entry
         */
        void removeFolderEntry(std::string const &name);

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

        FolderEntry();

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
         * @brief puts metadata for given entry out of use
         * @param name name of entry
         */
        void doPutMetaDataOutOfUse(std::string const &name);

        /**
         * @brief retrieves the starting block index of a given entry
         * @param n the entry to retrieve the block index of
         * @return the starting block index
         */
        uint64_t getBlockIndexForEntry(uint64_t const n) const;

        /**
         * @brief retrieves the number of entries in folder entry
         * @return the number of folder entries
         */
        uint64_t getNumberOfEntries() const;

        /**
         * @brief retrieves the name of an entry with given index
         * @return the name
         */
        std::string getEntryName(uint64_t const index) const;

        /**
         * @brief returns the entry index given the name
         * @param name the name of the entry
         * @return the index
         */
        uint64_t getMetaDataIndexForEntry(std::string const &name) const;

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
         * @param n the entry to retrieve the type of
         * @return the type of the entry
         */
        EntryType getTypeForEntry(uint64_t const n) const;

        /**
         * @brief determines if entry metadata is enabled. Entry metadata
         * consists of one byte, the first bit of which determines if the
         * metadata is in use or not. It won't be in use if the entry
         * associated with the metadata has been deleted in whcih case
         * this associated metadata can be overwritten when creating
         * a new entry.
         * @param n the entry for which we determine if in use
         * @return a value indicating if specified entry metadata is in use
         */
        bool entryMetaDataIsEnabled(uint64_t const n) const;

        /**
         * @brief seeks to where the metadata should be written. If
         * metadata for a previous entry has been deleted, we should
         * use that position instead to write new data
         * @return true if pre-existing metadata is to be overwritten,
         * false otherwise
         */
        bool seekToPositionWhereMetaDataWillBeWritten();

        /**
         * @brief lists a particular type of entry, file or folder
         * @return a list of entries of specified type
         */
        std::vector<EntryInfo> doListEntriesBasedOnType(EntryType entryType) const;

        // the core bfs io (path, blocks, password)
        CoreBFSIO m_io;

        // the underlying file blocks storing the folder entry data
        mutable FileEntry m_folderData;

        uint64_t m_startVolumeBlock;

        // stores the name of this folder
        std::string m_name;

    };

}



#endif // BFS_FOLDER_ENTRY_HPP__
