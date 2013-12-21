
/**
 * @brief an experimental fuse wrapper around the bfs container
 */

#include "bfs/BFS.hpp"
#include "bfs/FolderRemovalType.hpp"

#include <fuse.h>

#include <memory>

namespace bfs
{

    std::unique_ptr<BFS> g_bfs;

    int
    bfs_getattr(const char *path, struct stat *stbuf)
    {
        memset(stbuf, 0, sizeof(struct stat));

        if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 3;
        }

        return 0;
    }

    // create a file
    int bfs_mknod(const char *path, mode_t mode, dev_t dev)
    {

        g_bfs->addFile(path);

        return 0;
    }

    // Create a directory
    int bfs_mkdir(const char *path, mode_t mode)
    {

        g_bfs->addFolder(path);

        return 0;
    }

    // Remove a file
    int bfs_unlink(const char *path)
    {
        g_bfs->removeFile(path);

        return 0;
    }

    // Remove a folder
    int bfs_rmdir(const char *path)
    {
        g_bfs->removeFolder(path, FolderRemovalType::Recursive);

        return 0;
    }

    // truncate a file
    int bfs_truncate(const char *path, off_t newsize)
    {
        g_bfs->truncateFile(path, newsize);

        return 0;
    }

    // open a file
    int bfs_open(const char *path, struct fuse_file_info *fi)
    {
        g_bfs->openFile(path, OpenDisposition::buildOverwriteDisposition());
        return 0;
    }

    int bfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
    {

        FileEntryDevice device = g_bfs->openFile(path, OpenDisposition::buildReadOnlyDisposition());
        (void)device.read(buf, size);
        return 0;
    }

    int bfs_write(const char *path, const char *buf, size_t size, off_t offset,
             struct fuse_file_info *fi)
    {
        FileEntryDevice device = g_bfs->openFile(path, OpenDisposition::buildOverwriteDisposition());
        (void)device.write(buf, size);
        return 0;
    }

    void bfs_init(std::string const &imageStream, uint64_t totalBlocks)
    {
        g_bfs.reset(new BFS(imageStream, totalBlocks));
    }

    int bfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
    {
        g_bfs->addFile(path);
        return 0;
    }

    int bfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
    {
        g_bfs->truncateFile(path, offset);
        return 0;
    }

    int bfs_opendir(const char *path, struct fuse_file_info *fi)
    {
        g_bfs->setFolder(path);
        return 0;
    }

    int bfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                             off_t offset, struct fuse_file_info *fi)
    {

        //g_bfs->setFolder(path);

        FolderEntry folder = g_bfs->getCurrentFolder();

        std::vector<EntryInfo> infos = folder.listAllEntries();
        std::vector<EntryInfo>::iterator it = infos.begin();

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

    struct fuse_operations bfs_oper = {
      .mknod = bfs_mknod,
      .mkdir = bfs_mkdir,
      //.unlink = bfs_unlink,
      //.rmdir = bfs_rmdir,
      //.truncate = bfs_truncate,
      //.open = bfs_open,
      //.read = bfs_read,
      //.write = bfs_write,
      //.create = bfs_create,
      //.ftruncate = bfs_ftruncate,
      //.opendir = bfs_opendir,
      .readdir = bfs_readdir,
      .getattr = bfs_getattr,
    };
}

int main(int argc, char *argv[])
{

    int fuse_stat;

    bfs::bfs_init(argv[1], atoi(argv[2]));

    argv[1] = argv[3];
    argv[2] = NULL;
    argv[3] = NULL;
    argc = 2;

    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(argc, argv, &bfs::bfs_oper, NULL);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);



    return fuse_stat;
}


