#ifndef BFS_BFS_HPP__
#define BFS_BFS_HPP__

#include "bfs/EntryType.hpp"
#include "bfs/BFSException.hpp"
#include "bfs/FolderEntry.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <string>

namespace bfs
{
    class BFS
    {

            typedef boost::optional<FolderEntry> OptionalFolderEntry;

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

            bool folderExists(std::string const &path)
            {
                return doFolderExists(path);
            }

            void addFile(std::string const &path)
            {
                std::string thePath(path);
                char ch = *path.rbegin();
                // file entries with trailing slash should throw
                if(ch == '/') {
                    throw BFSException(BFSError::IllegalFilename);
                }

                boost::filesystem::path pathToAdd(thePath);
                boost::filesystem::path parentPath(pathToAdd.parent_path());

                OptionalFolderEntry parentEntry = doGetParentFolderEntry(parentPath.string());
                if(!parentEntry) {
                    throw BFSException(BFSError::NotFound);
                }

                // throw if already exists
                if(doFileExists(pathToAdd.string())) {
                    throw BFSException(BFSError::AlreadyExists);
                }
                if(doFolderExists(pathToAdd.string())) {
                    throw BFSException(BFSError::AlreadyExists);
                }

                parentEntry->addFileEntry(pathToAdd.filename().string());

            }

            void addFolder(std::string const &path) const
            {
                std::string thePath(path);
                char ch = *path.rbegin();
                // ignore trailing slash
                if(ch == '/') {
                    std::string(path.begin(), path.end() - 1).swap(thePath);
                }

                boost::filesystem::path pathToAdd(thePath);
                boost::filesystem::path parentPath(pathToAdd.parent_path());

                OptionalFolderEntry parentEntry = doGetParentFolderEntry(parentPath.string());
                if(!parentEntry) {
                    throw BFSException(BFSError::NotFound);
                }

                // throw if already exists
                if(doFileExists(pathToAdd.string())) {
                    throw BFSException(BFSError::AlreadyExists);
                }
                if(doFolderExists(pathToAdd.string())) {
                    throw BFSException(BFSError::AlreadyExists);
                }

                parentEntry->addFolderEntry(pathToAdd.filename().string());
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

            OptionalFolderEntry doGetParentFolderEntry(std::string const &path) const
            {
                boost::filesystem::path pathToCheck(path);

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

                        if(entryInfo->type() == EntryType::FolderType) {
                            return OptionalFolderEntry(folderOfInterest.getFolderEntry(entryInfo->filename()));
                        } else {
                            return OptionalFolderEntry();
                        }
                    }
                    // recurse deeper
                    folderOfInterest = folderOfInterest.getFolderEntry(entryInfo->filename());
                }


                return OptionalFolderEntry();

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
                boost::filesystem::path parentPath(pathToCheck.parent_path());
                OptionalFolderEntry parentEntry = doGetParentFolderEntry(parentPath.string());
                if(!parentEntry) {
                    return false;
                }

                std::string filename = pathToCheck.filename().string();

                OptionalEntryInfo entryInfo = parentEntry->getEntryInfo(filename);

                if(!entryInfo) {
                    return false;
                }

                if(entryInfo->type() == entryType) {
                    return true;
                }

                return false;
            }
    };
}

#endif // BFS_BFS_HPP__
