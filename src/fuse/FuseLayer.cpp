/*
  Copyright (c) <2013-2015>, <BenHJ>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
  3. Neither the name of the copyright holder nor the names of its contributors
  may be used to endorse or promote products derived from this software without
  specific prior written permission.

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
 * Based on fuse by Miklos Szeredi
 *
 */

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/TeaSafe.hpp"
#include "teasafe/TeaSafeException.hpp"
#include "utility/CipherCallback.hpp"
#include "utility/EcholessPasswordPrompt.hpp"
#include "utility/EventType.hpp"
#include "utility/PassHasher.hpp"

#include <boost/program_options.hpp>
#include <boost/progress.hpp>

#include <fuse.h>
#include <stdint.h>
#include <vector>
#include <functional>

#define TeaSafe_DATA ((teasafe::TeaSafe*) fuse_get_context()->private_data)

namespace fuselayer
{

    namespace detail
    {

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

    }

    class FuseLayer
    {
      public:
        static
        int
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
                    auto info(TeaSafe_DATA->getInfo(path));
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
                    return detail::exceptionDispatch(e);
                }
            }

            return 0;
        }

        static
        int
        teasafe_rename(const char *path, const char *newpath)
        {
            try {
                TeaSafe_DATA->renameEntry(path, newpath);
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }
            return 0;
        }

        // Create a directory
        static
        int
        teasafe_mkdir(const char *path, mode_t)
        {
            try {
                TeaSafe_DATA->addFolder(path);
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }
            return 0;
        }

        // Remove a file
        static
        int
        teasafe_unlink(const char *path)
        {
            try {
                TeaSafe_DATA->removeFile(path);
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }

            return 0;
        }

        // Remove a folder
        static
        int
        teasafe_rmdir(const char *path)
        {
            try {
                TeaSafe_DATA->removeFolder(path, teasafe::FolderRemovalType::Recursive);
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }

            return 0;
        }

        // truncate a file
        static
        int
        teasafe_truncate(const char *path, off_t newsize)
        {
            try {
                TeaSafe_DATA->truncateFile(path, newsize);
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }

            return 0;
        }

        // open a file.. note most reading and writing functionality
        // is deferred to the respective functions
        static
        int
        teasafe_open(const char *path, struct fuse_file_info *)
        {
            if (!TeaSafe_DATA->fileExists(path)) {
                try {
                    TeaSafe_DATA->addFile(path);
                } catch (teasafe::TeaSafeException const &e) {
                    return detail::exceptionDispatch(e);
                }
            }

            try {
                auto info(TeaSafe_DATA->getInfo(path));
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }

            return 0;
        }

        static
        int
        teasafe_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *)
        {
            auto device(TeaSafe_DATA->openFile(path, teasafe::OpenDisposition::buildReadOnlyDisposition()));
            device.seek(offset, std::ios_base::beg);
            auto read = device.read(buf,size);
            if(read < 0) { 
                return 0;
            }
            return read;
        }

        static
        int
        teasafe_write(const char *path, const char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
        {

            auto openMode = teasafe::ReadOrWriteOrBoth::ReadWrite;

            /*
              if((fi->flags & O_RDWR) == O_RDWR) {
              openMode = teasafe::ReadOrWriteOrBoth::ReadWrite;
              }*/

            auto appendType = teasafe::AppendOrOverwrite::Append;

            if ((fi->flags & O_APPEND) == O_APPEND) {
                appendType = teasafe::AppendOrOverwrite::Append;
            }

            auto truncateType = teasafe::TruncateOrKeep::Keep;

            if ((fi->flags & O_TRUNC) == O_TRUNC) {
                truncateType = teasafe::TruncateOrKeep::Truncate;
            }

            teasafe::OpenDisposition od(openMode, appendType, teasafe::CreateOrDontCreate::Create, truncateType);

            auto device(TeaSafe_DATA->openFile(path, od));
            device.seek(offset, std::ios_base::beg);
            auto written = device.write(buf, size);
            if(written < 0) {
                return 0;
            }
            return written;
        }

	static
	int 
	teasafe_access(const char *, int)
	{ return 0; }

        static
        void
        *teasafe_init(struct fuse_conn_info *)
        {
            return TeaSafe_DATA;
        }

        // create file; comment for git test
        static
        int
        teasafe_create(const char *path, mode_t, struct fuse_file_info *)
        {
            try {
                TeaSafe_DATA->addFile(path);
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }
            return 0;
        }

        static
        int
        teasafe_ftruncate(const char *path, off_t offset, struct fuse_file_info *)
        {
            try {
                TeaSafe_DATA->truncateFile(path, offset);
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }
            return 0;
        }

        // not sure what this does. Not figured out if we need it yet
        // but I think its called a bunch of times
        static
        int
        teasafe_opendir(const char *, struct fuse_file_info *)
        {
            return 0;
        }


        // list the directory contents
        static
        int
        teasafe_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t, struct fuse_file_info *)
        {
            try {
                auto folder(TeaSafe_DATA->getFolder(path));

                auto & infos(folder.listAllEntries());

                filler(buf, ".", NULL, 0);           /* Current directory (.)  */
                filler(buf, "..", NULL, 0);

                for(auto const &it : infos) {
                    struct stat stbuf;
                    if (it.second->type() == teasafe::EntryType::FileType) {
                        stbuf.st_mode = S_IFREG | 0755;
                        stbuf.st_nlink = 1;
                        stbuf.st_size = it.second->size();
                    } else {
                        stbuf.st_mode = S_IFDIR | 0744;
                        stbuf.st_nlink = 3;
                    }

                    filler(buf, it.second->filename().c_str(), &stbuf, 0);

                }
            } catch (teasafe::TeaSafeException const &e) {
                return detail::exceptionDispatch(e);
            }

            return 0;
        }

        // for getting stats about the overall filesystem
        // (used when issuing a 'df' command). Note that in TeaSafe,
        // the number of inodes corresponds to the number of blocks
        static
        int
        teasafe_statfs(const char *, struct statvfs *statv)
        {
            TeaSafe_DATA->statvfs(statv);
            return 0;
        }

        // not actually used, but pasted here to shut-up some warning messages
#ifndef __linux__
        static
        int
        teasafe_setxattr(const char *,
                         const char *,
                         const char *,
                         size_t,
                         int,
                         uint32_t)
#else
            static
            int
            teasafe_setxattr(const char *,
                             const char *,
                             const char *,
                             size_t,
                             int)
#endif
        {
            return 0;
        }

        // to shut-up 'function not implemented warnings'
        // not presently required
        static
        int
        teasafe_flush(const char *, struct fuse_file_info *)
        {
            return 0;
        }

        // to shut-up 'function not implemented warnings'
        // not presently required
        static
        int
        teasafe_chmod(const char *, mode_t)
        {
            return 0;
        }

        // to shut-up 'function not implemented warnings'
        // not presently required
        static
        int
        teasafe_chown(const char *, uid_t, gid_t)
        {
            return 0;
        }

        static
        int
        teasafe_utimens(const char *, const struct timespec *)
        {
            return 0;
        }

    };

}

// to shut-up 'function not implemented warnings'
// not presently required
static struct fuse_operations teasafe_oper;

/**
 * @brief initialize the fuse operations struct
 * @param ops the fue callback functions
 */
void initOperations(struct fuse_operations &ops, fuselayer::FuseLayer &fuseLayer)
{
    ops.mkdir     = fuseLayer.teasafe_mkdir;
    ops.unlink    = fuseLayer.teasafe_unlink;
    ops.rmdir     = fuseLayer.teasafe_rmdir;
    ops.truncate  = fuseLayer.teasafe_truncate;
    ops.open      = fuseLayer.teasafe_open;
    ops.read      = fuseLayer.teasafe_read;
    ops.write     = fuseLayer.teasafe_write;
    ops.create    = fuseLayer.teasafe_create;
    ops.ftruncate = fuseLayer.teasafe_ftruncate;
    ops.opendir   = fuseLayer.teasafe_opendir;
    ops.init      = fuseLayer.teasafe_init;
    ops.readdir   = fuseLayer.teasafe_readdir;
    ops.getattr   = fuseLayer.teasafe_getattr;
    ops.rename    = fuseLayer.teasafe_rename;
    ops.statfs    = fuseLayer.teasafe_statfs;
    ops.setxattr  = fuseLayer.teasafe_setxattr;
    ops.flush     = fuseLayer.teasafe_flush;
    ops.chmod     = fuseLayer.teasafe_chmod;
    ops.chown     = fuseLayer.teasafe_chown;
    ops.utimens   = fuseLayer.teasafe_utimens;
    ops.access    = fuseLayer.teasafe_access;
}

int main(int argc, char *argv[])
{

    // parse the program options
    bool debug = true;
    bool magic = false;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("imageName", po::value<std::string>(), "teasafe image path")
        ("mountPoint", po::value<std::string>(), "mountPoint path")
        ("debug", po::value<bool>(&debug)->default_value(true), "fuse debug")
        ("coffee", po::value<bool>(&magic)->default_value(false), "mount alternative sub-volume")
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

        if (vm.count("help")) {
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
    teasafe::SharedCoreIO io(std::make_shared<teasafe::CoreTeaSafeIO>());
    io->path = vm["imageName"].as<std::string>().c_str();
    io->password = teasafe::utility::getPassword("teasafe password: ");
    io->rootBlock = magic ? atoi(teasafe::utility::getPassword("magic number: ").c_str()) : 0;

    // Obtain the initialization vector from the first 8 bytes
    // and the number of xtea rounds from the ninth byte
    // and the cipher type from the tenth byte
    teasafe::detail::readImageIVAndRounds(io);

    // Obtain the number of blocks in the image by reading the image's block count
    long const amount = teasafe::detail::CIPHER_BUFFER_SIZE / 100000;
    std::function<void(teasafe::EventType)> f(std::bind(&teasafe::cipherCallback, std::placeholders::_1, amount));
    io->ccb = f;
    teasafe::ContainerImageStream stream(io, std::ios::in | std::ios::binary);

    // compare password hashes
    uint8_t hashRecovered[32];
    teasafe::detail::getPassHash(stream, hashRecovered);
    uint8_t hashEntered[32];
    teasafe::utility::sha256((char*)io->password.c_str(), hashEntered);
    if(!teasafe::utility::compareTwoHashes(hashEntered, hashRecovered)) {
        std::cout<<"Incorrect password"<<std::endl;
        exit(0);
    }


    io->blocks = teasafe::detail::getBlockCount(stream);

    printf("Counting allocated blocks. Please wait...\n");

    io->freeBlocks = io->blocks - teasafe::detail::getNumberOfAllocatedBlocks(stream);
    io->blockBuilder = std::make_shared<teasafe::FileBlockBuilder>(io);

    printf("Finished counting allocated blocks.\n");

    stream.close();

    // Create the basic file system
    teasafe::TeaSafe theBfs(io);

    // make arguments fuse-compatible
    char      arg0[] = "teasafe";
    char* arg1 = (char*)vm["mountPoint"].as<std::string>().c_str();
    char      arg2[] = "-s";
    char      arg3[] = "-d";

#ifdef __APPLE__
    char      arg4[] = "-o";
    char      arg5[] = "noappledouble";
    char* fuseArgs[] = { &arg0[0], &arg1[0], &arg2[0], &arg3[0], &arg4[0], &arg5[0], NULL };
#else
    char* fuseArgs[] = { &arg0[0], &arg1[0], &arg2[0], &arg3[0], NULL };
#endif

    int fuseArgCount = (int)(sizeof(fuseArgs) / sizeof(fuseArgs[0])) - 1;

    // turn over control to fuse
    fprintf(stderr, "about to call fuse_main\n");

    // initializse fuse_operations
    fuselayer::FuseLayer fuseLayer;
    initOperations(teasafe_oper, fuseLayer);

    int fuse_stat = fuse_main(fuseArgCount, fuseArgs, &teasafe_oper, &theBfs);
    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);

    return fuse_stat;

}
