
/**
 * @brief an experimental fuse wrapper around the bfs container
 */

#include "bfs/BFS.hpp"
//#include "bfs/FolderRemovalType.hpp"

#include <fuse.h>

#include <memory>

#define BFS_DATA ((bfs::BFS*) fuse_get_context()->private_data)

int exceptionDispatch(bfs::BFSException const &ex)
{
    if(ex == bfs::BFSException(bfs::BFSError::NotFound)) {
        return -ENOENT;
    }
    if(ex == bfs::BFSException(bfs::BFSError::AlreadyExists)) {
        return -EEXIST;
    }

    return 0;
}

static int
bfs_getattr(const char *path, struct stat *stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 3;
        return 0;
    } else {
        try {
            bfs::EntryInfo info = BFS_DATA->getInfo(path);
            if(info.type() == bfs::EntryType::FolderType){
                stbuf->st_mode = S_IFDIR | 0777;
                stbuf->st_nlink = 3;
                return 0;
            } else if(info.type() == bfs::EntryType::FileType){
                stbuf->st_mode = S_IFREG | 0777;
                stbuf->st_nlink = 1;
                stbuf->st_size = info.size();
                return 0;
            } else {
               return -ENOENT;
            }
        } catch (bfs::BFSException const &e) {
            return exceptionDispatch(e);
        }
    }

    return 0;
}


// Create a directory
static int bfs_mkdir(const char *path, mode_t mode)
{
    try {
        BFS_DATA->addFolder(path);
    } catch (bfs::BFSException const &e) {
        return exceptionDispatch(e);
    }
    return 0;
}

// Remove a file
static int bfs_unlink(const char *path)
{
    try {
        BFS_DATA->removeFile(path);
    } catch (bfs::BFSException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

// Remove a folder
static int bfs_rmdir(const char *path)
{
    try {
        BFS_DATA->removeFolder(path, bfs::FolderRemovalType::Recursive);
    } catch (bfs::BFSException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

// truncate a file
static int bfs_truncate(const char *path, off_t newsize)
{
    try {
        BFS_DATA->truncateFile(path, newsize);
    } catch (bfs::BFSException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

// open a file.. note most reading and writing functionality
// is deferred to the respective functions
static int bfs_open(const char *path, struct fuse_file_info *fi)
{
    if(!BFS_DATA->fileExists(path)) {
        try {
            BFS_DATA->addFile(path);
        } catch (bfs::BFSException const &e) {
            return exceptionDispatch(e);
        }
    }

    try {
        bfs::EntryInfo info = BFS_DATA->getInfo(path);
    } catch (bfs::BFSException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

static int bfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    bfs::FileEntryDevice device = BFS_DATA->openFile(path, bfs::OpenDisposition::buildReadOnlyDisposition());
    device.seek(offset, std::ios_base::beg);
    return device.read(buf, size);
}

static int bfs_write(const char *path, const char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{

    bfs::ReadOrWriteOrBoth openMode = bfs::ReadOrWriteOrBoth::ReadWrite;

    /*
    if((fi->flags & O_RDWR) == O_RDWR) {
        openMode = bfs::ReadOrWriteOrBoth::ReadWrite;
    }*/

    bfs::AppendOrOverwrite appendType = bfs::AppendOrOverwrite::Append;

    if((fi->flags & O_APPEND) == O_APPEND) {
        appendType = bfs::AppendOrOverwrite::Append;
    }

    bfs::TruncateOrKeep truncateType = bfs::TruncateOrKeep::Keep;

    if((fi->flags & O_TRUNC) == O_TRUNC) {
        truncateType = bfs::TruncateOrKeep::Truncate;
    }

    bfs::OpenDisposition od(openMode, appendType, bfs::CreateOrDontCreate::Create, truncateType);

    bfs::FileEntryDevice device = BFS_DATA->openFile(path, od);
    device.seek(offset, std::ios_base::beg);
    return device.write(buf, size);
}

static void *bfs_init(struct fuse_conn_info *conn)
{
    return BFS_DATA;
}

// create file; comment for git test
static int bfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    try {
        BFS_DATA->addFile(path);
    } catch (bfs::BFSException const &e) {
        return exceptionDispatch(e);
    }
    return 0;
}

static int bfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    try {
        BFS_DATA->truncateFile(path, offset);
    } catch (bfs::BFSException const &e) {
        return exceptionDispatch(e);
    }
    return 0;
}

// not sure what this does. Not figured out if we need it yet
// but I think its called a bunch of times
static int bfs_opendir(const char *path, struct fuse_file_info *fi)
{
    return 0;
}


// list the directory contents
static int bfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
    try {
        bfs::FolderEntry folder = BFS_DATA->getCurrent(path);

        std::vector<bfs::EntryInfo> infos = folder.listAllEntries();
        std::vector<bfs::EntryInfo>::iterator it = infos.begin();

        filler(buf, ".", NULL, 0);           /* Current directory (.)  */
        filler(buf, "..", NULL, 0);

        for(; it != infos.end(); ++it) {
            struct stat stbuf;
            if(it->type() == bfs::EntryType::FileType) {
                stbuf.st_mode = S_IFREG | 0755;
                stbuf.st_nlink = 1;
                stbuf.st_size = it->size();
            } else {
                stbuf.st_mode = S_IFDIR | 0744;
                stbuf.st_nlink = 3;
            }

            filler(buf, it->filename().c_str(), &stbuf, 0);

        }
    } catch (bfs::BFSException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

static struct fuse_operations bfs_oper = {
  .mkdir = bfs_mkdir,
  .unlink = bfs_unlink,
  .rmdir = bfs_rmdir,
  .truncate = bfs_truncate,
  .open = bfs_open,
  .read = bfs_read,
  .write = bfs_write,
  .create = bfs_create,
  .ftruncate = bfs_ftruncate,
  .opendir = bfs_opendir,
  .init = bfs_init,
  .readdir = bfs_readdir,
  .getattr = bfs_getattr
};


int main(int argc, char *argv[])
{

    int fuse_stat;

    bfs::BFS *bfsPtr = new bfs::BFS(argv[1], atoi(argv[2]));

    argc -= 2;
    for(int i = 0; i<argc;++i) {
        argv[i+1] = argv[i+3];
    }

    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &bfs_oper, bfsPtr);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;
}


