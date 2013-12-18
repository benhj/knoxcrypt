#include "bfs/EntryInfo.hpp"

namespace bfs
{
    EntryInfo::EntryInfo(std::string const &fileName,
                         uint64_t const &fileSize,
                         EntryType const &entryType,
                         bool const writable,
                         uint64_t const firstFileBlock,
                         uint64_t const folderIndex)
        : m_fileName(fileName)
        , m_fileSize(fileSize)
        , m_entryType(entryType)
        , m_writable(writable)
        , m_firstFileBlock(firstFileBlock)
        , m_folderIndex(folderIndex)
    {

    }

    std::string
    EntryInfo::filename() const
    {
        return m_fileName;
    }

    uint64_t
    EntryInfo::size() const
    {
        return m_fileSize;
    }

    EntryType
    EntryInfo::type() const
    {
        return m_entryType;
    }

    bool
    EntryInfo::writable() const
    {
        return m_writable;
    }

    uint64_t
    EntryInfo::firstFileBlock() const
    {
        return m_firstFileBlock;
    }

    uint64_t
    EntryInfo::folderIndex() const
    {
        return m_folderIndex;
    }

}
