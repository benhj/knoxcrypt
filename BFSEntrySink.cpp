#include "BFSEntrySink.hpp"
#include "Detail.hpp"

#include <boost/make_shared.hpp>

#include <vector>

namespace bfs
{
    BFSEntrySink::BFSEntrySink(std::string const &bfsOutputPath,
                               std::string const &entryName,
                               uint64_t const fsize,
                               uint64_t const parentIndex)
        : m_bfsOutputPath(bfsOutputPath)
        , m_entryName(entryName)
        , m_fsize(fsize)
        , m_parentIndex(parentIndex)
        , m_bfsOutputStream(boost::make_shared<std::fstream>(bfsOutputPath.c_str(), std::ios::in | std::ios::out | std::ios::binary))
        , m_metaOffset(detail::getOffsetOfNextFreeMetaSpaceBlock(*m_bfsOutputStream))
        , m_fileOffset(detail::getOffsetOfNextFreeFileSpaceBlock(*m_bfsOutputStream))
    {
        updateSuperBlock();
        writeMetaBlock();
        writeFileName();
    }

    /**
     * writes the filename which has a max size of 50 bytes
     */
    void
    BFSEntrySink::writeFileName()
    {
        std::vector<uint8_t> fname;
        fname.resize(detail::MAX_FILENAME_LENGTH);
        std::string::size_type t = m_entryName.length();
        for(int i = 0; i < t; ++i) {
            fname[i] = m_entryName[i];
        }
        fname[t] = '\0';
        uint64_t fileCount = detail::getFileCount(*m_bfsOutputStream);
        uint64_t fileOffset = detail::getOffsetOfFileN(*m_bfsOutputStream, fileCount);
        (void)m_bfsOutputStream->seekp((std::streampos)fileOffset);
        m_bfsOutputStream->write((char*)&fname.front(), detail::MAX_FILENAME_LENGTH);
    }

    /**
     * writes the file metadata which consists of:-
     *
     * 8 bytes specifying the size
     * 8 bytes specifying the offset in the bfs image
     * 8 bytes specifying the parent folder index
     * 8 bytes specifying other data (e.g. read-only, last modified etc)
     */
    void
    BFSEntrySink::writeMetaBlock()
    {
        uint8_t sizeBytes[8];
        detail::convertInt64ToInt8Array(m_fsize, sizeBytes);
        uint8_t posBytes[8];
        detail::convertInt64ToInt8Array(m_fileOffset, posBytes);
        uint8_t parentBytes[8];
        detail::convertInt64ToInt8Array(m_parentIndex, parentBytes);
        (void)m_bfsOutputStream->seekp((std::streampos)m_metaOffset);
        m_bfsOutputStream->write((char*)sizeBytes, 8);    // size of file
        m_bfsOutputStream->write((char*)posBytes, 8);     // position in main file space
        m_bfsOutputStream->write((char*)parentBytes, 8);  // index of parent directory
        m_bfsOutputStream->write((char*)parentBytes, 8);  // extra space for other stuff (tbd)
    }

    /**
     * Updates file count and total file size information in the superblock
     */
    void BFSEntrySink::updateSuperBlock()
    {
        detail::incrementFileCount(*m_bfsOutputStream);
        detail::updateFileSpaceAccumulator(*m_bfsOutputStream, m_fsize);
    }

    std::streamsize
    BFSEntrySink::write(char_type const * const buf, std::streamsize const n) const
    {
        m_bfsOutputStream->write((char*)buf, n);
        return n;
    }

    BFSEntrySink::~BFSEntrySink()
    {
        m_bfsOutputStream->flush();
        m_bfsOutputStream->close();
    }

}
