/*
Copyright (c) <2013-2014>, <BenHJ>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/**
 * @brief an experimental fuse wrapper around the teasafe container
 *
 * Based on hello.c by Miklos Szeredi
 *
 */

#include "teasafe/TeaSafe.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "utility/EcholessPasswordPrompt.hpp"

#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>

#include <fuse.h>


#define TeaSafe_DATA ((teasafe::TeaSafe*) fuse_get_context()->private_data)

int exceptionDispatch(teasafe::TeaSafeException const &ex)
{
    if (ex == teasafe::TeaSafeException(teasafe::TeaSafeError::NotFound)) {
        return -ENOENT;
    }
    if (ex == teasafe::TeaSafeException(teasafe::TeaSafeError::AlreadyExists)) {
        return -EEXIST;
    }

    return 0;
}

static int
teasafe_getattr(const char *path, struct stat *stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 3;
        stbuf->st_blksize = 500;
        return 0;
    } else {
        try {
            teasafe::EntryInfo info = TeaSafe_DATA->getInfo(path);
            if (info.type() == teasafe::EntryType::FolderType) {
                stbuf->st_mode = S_IFDIR | 0777;
                stbuf->st_nlink = 3;
                stbuf->st_blksize = teasafe::detail::FILE_BLOCK_SIZE - teasafe::detail::FILE_BLOCK_META;
                return 0;
            } else if (info.type() == teasafe::EntryType::FileType) {
                stbuf->st_mode = S_IFREG | 0777;
                stbuf->st_nlink = 1;
                stbuf->st_size = info.size();
                stbuf->st_blksize = teasafe::detail::FILE_BLOCK_SIZE - teasafe::detail::FILE_BLOCK_META;
                return 0;
            } else {
                return -ENOENT;
            }
        } catch (teasafe::TeaSafeException const &e) {
            return exceptionDispatch(e);
        }
    }

    return 0;
}

int teasafe_rename(const char *path, const char *newpath)
{
    try {
        TeaSafe_DATA->renameEntry(path, newpath);
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }
    return 0;
}

// Create a directory
static int teasafe_mkdir(const char *path, mode_t mode)
{
    try {
        TeaSafe_DATA->addFolder(path);
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }
    return 0;
}

// Remove a file
static int teasafe_unlink(const char *path)
{
    try {
        TeaSafe_DATA->removeFile(path);
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

// Remove a folder
static int teasafe_rmdir(const char *path)
{
    try {
        TeaSafe_DATA->removeFolder(path, teasafe::FolderRemovalType::Recursive);
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

// truncate a file
static int teasafe_truncate(const char *path, off_t newsize)
{
    try {
        TeaSafe_DATA->truncateFile(path, newsize);
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

// open a file.. note most reading and writing functionality
// is deferred to the respective functions
static int teasafe_open(const char *path, struct fuse_file_info *fi)
{
    if (!TeaSafe_DATA->fileExists(path)) {
        try {
            TeaSafe_DATA->addFile(path);
        } catch (teasafe::TeaSafeException const &e) {
            return exceptionDispatch(e);
        }
    }

    try {
        teasafe::EntryInfo info = TeaSafe_DATA->getInfo(path);
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

static int teasafe_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    teasafe::FileEntryDevice device = TeaSafe_DATA->openFile(path, teasafe::OpenDisposition::buildReadOnlyDisposition());
    device.seek(offset, std::ios_base::beg);
    return device.read(buf, size);
}

static int teasafe_write(const char *path, const char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi)
{

    teasafe::ReadOrWriteOrBoth openMode = teasafe::ReadOrWriteOrBoth::ReadWrite;

    /*
      if((fi->flags & O_RDWR) == O_RDWR) {
      openMode = teasafe::ReadOrWriteOrBoth::ReadWrite;
      }*/

    teasafe::AppendOrOverwrite appendType = teasafe::AppendOrOverwrite::Append;

    if ((fi->flags & O_APPEND) == O_APPEND) {
        appendType = teasafe::AppendOrOverwrite::Append;
    }

    teasafe::TruncateOrKeep truncateType = teasafe::TruncateOrKeep::Keep;

    if ((fi->flags & O_TRUNC) == O_TRUNC) {
        truncateType = teasafe::TruncateOrKeep::Truncate;
    }

    teasafe::OpenDisposition od(openMode, appendType, teasafe::CreateOrDontCreate::Create, truncateType);

    teasafe::FileEntryDevice device = TeaSafe_DATA->openFile(path, od);
    device.seek(offset, std::ios_base::beg);
    return device.write(buf, size);
}

static void *teasafe_init(struct fuse_conn_info *conn)
{
    return TeaSafe_DATA;
}

// create file; comment for git test
static int teasafe_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    try {
        TeaSafe_DATA->addFile(path);
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }
    return 0;
}

static int teasafe_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
    try {
        TeaSafe_DATA->truncateFile(path, offset);
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }
    return 0;
}

// not sure what this does. Not figured out if we need it yet
// but I think its called a bunch of times
static int teasafe_opendir(const char *path, struct fuse_file_info *fi)
{
    return 0;
}


// list the directory contents
static int teasafe_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
    try {
        teasafe::FolderEntry folder = TeaSafe_DATA->getCurrent(path);

        std::vector<teasafe::EntryInfo> infos = folder.listAllEntries();
        std::vector<teasafe::EntryInfo>::iterator it = infos.begin();

        filler(buf, ".", NULL, 0);           /* Current directory (.)  */
        filler(buf, "..", NULL, 0);

        for (; it != infos.end(); ++it) {
            struct stat stbuf;
            if (it->type() == teasafe::EntryType::FileType) {
                stbuf.st_mode = S_IFREG | 0755;
                stbuf.st_nlink = 1;
                stbuf.st_size = it->size();
            } else {
                stbuf.st_mode = S_IFDIR | 0744;
                stbuf.st_nlink = 3;
            }

            filler(buf, it->filename().c_str(), &stbuf, 0);

        }
    } catch (teasafe::TeaSafeException const &e) {
        return exceptionDispatch(e);
    }

    return 0;
}

static struct fuse_operations teasafe_oper =
{
    .mkdir = teasafe_mkdir,
    .unlink = teasafe_unlink,
    .rmdir = teasafe_rmdir,
    .truncate = teasafe_truncate,
    .open = teasafe_open,
    .read = teasafe_read,
    .write = teasafe_write,
    .create = teasafe_create,
    .ftruncate = teasafe_ftruncate,
    .opendir = teasafe_opendir,
    .init = teasafe_init,
    .readdir = teasafe_readdir,
    .getattr = teasafe_getattr,
    .rename = teasafe_rename
};

int main(int argc, char *argv[])
{

    // parse the program options
    uint64_t rootBlock;
    bool debug = true;
    bool magic = false;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("imageName", po::value<std::string>(), "teasafe image path")
        ("mountPoint", po::value<std::string>(), "mountPoint path")
        ("debug", po::value<bool>(&debug)->default_value(true), "fuse debug")
        ("coffee", po::value<bool>(&magic)->default_value(false), "magic partition")
    ;

    po::positional_options_description positionalOptions;
    (void)positionalOptions.add("imageName", 1);
    (void)positionalOptions.add("mountPoint", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).
                  options(desc).positional(positionalOptions).run(),
                  vm);
        po::notify(vm);
        if (vm.count("help") ||
            vm.count("mountPoint")==0 || vm.count("imageName") == 0) {
            std::cout << desc << std::endl;
            return 1;
        }

        if(vm.count("help")) {
            std::cout<<desc<<"\n";
        } else {

            std::cout<<"\nMounting as follows: \n\n";
            std::cout<<"image path: "<<vm["imageName"].as<std::string>()<<std::endl;
            std::cout<<"mount point: "<<vm["mountPoint"].as<std::string>()<<std::endl;
            std::cout<<"\n";

        }
    } catch (...) {
        std::cout<<"Problem parsing options"<<std::endl;
        std::cout<<desc<<std::endl;
        return 1;
    }

    // Setup a core teasafe io object which stores highlevel info about accessing
    // the TeaSafe image
    teasafe::SharedCoreIO io(boost::make_shared<teasafe::CoreTeaSafeIO>());
    io->path = vm["imageName"].as<std::string>().c_str();
    io->password = teasafe::utility::getPassword();
    io->rootBlock = magic ? atoi(teasafe::utility::getPassword("magic number: ").c_str()) : 0;

    // Obtain the number of blocks in the image by reading the image's block count
    teasafe::TeaSafeImageStream stream(io, std::ios::in | std::ios::binary);
    io->blocks = teasafe::detail::getBlockCount(stream);

    printf("Counting allocated blocks. Please wait...\n");

    io->freeBlocks = teasafe::detail::getNumberOfAllocatedBlocks(stream);

    printf("Finished counting allocated blocks.\n");

    stream.close();

    // Create the basic file system
    teasafe::TeaSafe theBfs(io);

    // make arguments fuse-compatable
    argc = 4;
    argv[1] = const_cast<char*>(vm["mountPoint"].as<std::string>().c_str());

    // always run in single-threaded + debug mode; multi-threaded buggy
    argv[2] = const_cast<char*>(std::string("-s").c_str());
    argv[3] = const_cast<char*>(std::string("-d").c_str());

    // debug mode? (this is also single-threaded)
    // debug mode prints out a load of debug info to console
    //if(debug) {
    //    argv[2] = const_cast<char*>(std::string("-d").c_str());
    //}

    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");
    int fuse_stat = fuse_main(argc, argv, &teasafe_oper, &theBfs);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;

}
