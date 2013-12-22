
/**
 * @brief an experimental fuse wrapper around the bfs container
 */

#include "bfs/BFS.hpp"
//#include "bfs/FolderRemovalType.hpp"

#include <fuse.h>

#include <memory>

#define BFS_DATA ((bfs::BFS*) fuse_get_context()->private_data)


static int
bfs_getattr(const char *path, struct stat *stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 3;
                return 0;
    } else {

        try {
            bfs::EntryInfo info = BFS_DATA->getInfo(path);

            if(info.type() == bfs::EntryType::FolderType){
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 3;
            } else if(info.type() == bfs::EntryType::FileType){
                stbuf->st_mode = S_IFREG | 0444;
                stbuf->st_nlink = 1;
                stbuf->st_size = info.size();
            } else {
               return -ENOENT;
            }
        } catch (bfs::BFSException const &e) {

        }
    }

    return 0;
}

// create a file
static int bfs_mknod(const char *path, mode_t mode, dev_t dev)
{

    std::cout<<"hello mknod!"<<std::endl;
    BFS_DATA->addFile(path);

    return 0;
}

// Create a directory
static int bfs_mkdir(const char *path, mode_t mode)
{
    printf("hello mkdir\n");
    std::cout<<"path: "<<path<<std::endl;
    BFS_DATA->addFolder(path);

    return 0;
}

// Remove a file
static int bfs_unlink(const char *path)
{
    BFS_DATA->removeFile(path);

    return 0;
}

// Remove a folder
static int bfs_rmdir(const char *path)
{
    BFS_DATA->removeFolder(path, bfs::FolderRemovalType::Recursive);

    return 0;
}

// truncate a file
static int bfs_truncate(const char *path, off_t newsize)
{
    BFS_DATA->truncateFile(path, newsize);

    return 0;
}

// open a file
static int bfs_open(const char *path, struct fuse_file_info *fi)
{
    BFS_DATA->openFile(path, bfs::OpenDisposition::buildOverwriteDisposition());
    return 0;
}

static int bfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{

    bfs::FileEntryDevice device = BFS_DATA->openFile(path, bfs::OpenDisposition::buildReadOnlyDisposition());
    (void)device.read(buf, size);
    return 0;
}

static int bfs_write(const char *path, const char *buf, size_t size, off_t offset,
         struct fuse_file_info *fi)
{
    bfs::FileEntryDevice device = BFS_DATA->openFile(path, bfs::OpenDisposition::buildOverwriteDisposition());
    (void)device.write(buf, size);
    return 0;
}

static void *bfs_init(struct fuse_conn_info *conn)
{
    return BFS_DATA;
}

static int bfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    std::cout<<"hello!"<<std::endl;
    BFS_DATA->addFile(path);
    return 0;
}

static int bfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    BFS_DATA->truncateFile(path, offset);
    return 0;
}

static int bfs_opendir(const char *path, struct fuse_file_info *fi)
{
    std::cout<<"hello"<<std::endl;
    BFS_DATA->setFolder(path);
    return 0;
}

static int bfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{

    BFS_DATA->setFolder(path);

    bfs::FolderEntry folder = BFS_DATA->getCurrentFolder();

    std::vector<bfs::EntryInfo> infos = folder.listAllEntries();
    std::vector<bfs::EntryInfo>::iterator it = infos.begin();

    filler(buf, ".", NULL, 0);           /* Current directory (.)  */
    filler(buf, "..", NULL, 0);

    for(; it != infos.end(); ++it) {
        filler(buf, it->filename().c_str(), NULL, 0);
    }

    /*
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0))
            break;
    }

    closedir(dp);*/
    return 0;
}

static struct fuse_operations bfs_oper = {
  .mknod = bfs_mknod,
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
  .getattr = bfs_getattr,
};


int main(int argc, char *argv[])
{

    int fuse_stat;

    bfs::BFS *bfsPtr = new bfs::BFS(argv[1], atoi(argv[2]));

    argc -= 2;
    argv[1] = argv[3];
    argv[2] = argv[4];
    argv[3] = argv[5];

    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &bfs_oper, bfsPtr);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;
}


