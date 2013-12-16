#ifndef BFS_ENTRY_INFO_HPP__
#define BFS_ENTRY_INFO_HPP__

#include "EntryType.hpp"

#include <string>

namespace bfs
{

    class EntryInfo
    {
      public:
        EntryInfo(std::string const &fileName,
                  uint64_t const &fileSize,
                  EntryType const &entryType,
                  bool const writable,
                  uint64_t const firstFileBlock,
                  uint64_t const folderIndex);

        std::string filename() const;

        uint64_t size() const;

        EntryType type() const;

        bool writable() const;

        uint64_t firstFileBlock() const;

        uint64_t folderIndex() const;

      private:
        std::string m_fileName;
        uint64_t m_fileSize;
        EntryType m_entryType;
        bool m_writable;
        uint64_t m_firstFileBlock;
        uint64_t m_folderIndex;
    };

}

#endif // BFS_ENTRY_INFO_HPP__
