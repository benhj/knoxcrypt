#ifndef BFS_BFS_ENTRY_SINK_HPP__
#define BFS_BFS_ENTRY_SINK_HPP__

#include <boost/iostreams/categories.hpp>  // sink_tag
#include <boost/shared_ptr.hpp>

#include <iosfwd>                          // streamsize
#include <string>
#include <fstream>

namespace bfs
{

    class BFSEntrySink
    {
      public:
        typedef char                          char_type;
        typedef boost::iostreams::sink_tag    category;

        BFSEntrySink(std::string const &bfsOutputPath,
                     std::string const &entryName,
                     uint64_t const fsize,
                     uint64_t const parentIndex);

        /**
         * @param buf the data to be written
         * @param n number of bytes to written
         * @return the number of bytes written
         */
        std::streamsize write(char_type const * const buf, std::streamsize const n) const;
        ~BFSEntrySink();

      private:
        std::string const m_bfsOutputPath;                    // the image path
        std::string const m_entryName;                        // the name of the entry to add
        uint64_t const m_fsize;                               // size of the file to add
        uint64_t const m_parentIndex;                         // parent folder index
        boost::shared_ptr<std::fstream> m_bfsOutputStream;    // the image stream (shared for copyability)
        uint64_t m_metaOffset;                                // point where meta data should be stored
        uint64_t m_fileOffset;                                // point where file data should be stored

        void writeFileName();
        void writeMetaBlock();

    };


}

#endif // BFS_BFS_ENTRY_SINK_HPP__
