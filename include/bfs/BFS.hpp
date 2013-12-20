#ifndef BFS_BFS_HPP__
#define BFS_BFS_HPP__

#include "bfs/EntryType.hpp"
#include "bfs/FolderEntry.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

namespace bfs
{

    class BFS
    {
        public:
            BFS(std::string const &imagePath, uint64_t const totalBlocks)
            : m_imagePath(imagePath)
            , m_rootFolder(imagePath, totalBlocks, 0, "root")
            {

            }

            bool fileExists(std::string const &path) const
            {
                return doFileExists(path);
            }

            bool folderExists(std::string const &path) const
            {
                return doFolderExists(path);
            }

        private:
            // the path of the bfs image
            std::string m_imagePath;

            // the root of the filesystem
            bfs::FolderEntry m_rootFolder;

            BFS(); // not required

            bool doFileExists(std::string const &path) const
            {
                return doExistanceCheck(path, EntryType::FileType);
            }

            bool doFolderExists(std::string const &path) const
            {
                return doExistanceCheck(path, EntryType::FolderType);
            }

            bool doExistanceCheck(std::string const &path, EntryType const &entryType) const
            {
                std::string thePath(path);
                char ch = *path.rbegin();
                // ignore trailing slash, but only if folder type
                // an entry of file type should never have a trailing
                // slash and is allowed to fail in this case
                if(ch == '/' && entryType == EntryType::FolderType) {
                    std::string(path.begin(), path.end() - 1).swap(thePath);
                }

                boost::filesystem::path pathToCheck(thePath);

                // iterate over path parts extracting sub folders along the way
                boost::filesystem::path::iterator it = pathToCheck.begin();
                FolderEntry folderOfInterest = m_rootFolder;
                boost::filesystem::path pathBuilder;
                for(; it != pathToCheck.end(); ++it) {

                    OptionalEntryInfo entryInfo = folderOfInterest.getEntryInfo(it->string());

                    if(!entryInfo) {
                        return false;
                    }
                    pathBuilder /= entryInfo->filename();

                    if(pathBuilder == pathToCheck) {
                        if(entryInfo->type() == entryType) {
                            return true;
                        }
                    }

                    // not at end of path yet we have a file so
                    // does not go any deeper
                    if(entryInfo->type() == EntryType::FileType) {
                        return false;
                    }

                    // recurse deeper
                    folderOfInterest = folderOfInterest.getFolderEntry(entryInfo->filename());
                }
                return false;
            }


    };

}

#endif // BFS_BFS_HPP__
