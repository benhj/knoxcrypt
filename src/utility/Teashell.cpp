/*
  Copyright (c) <2014>, <BenHJ>
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
 * @brief a *very hacky* wrapper for accessing / modifying teasafe containers
 */

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/EntryInfo.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/TeaSafe.hpp"
#include "teasafe/TeaSafeException.hpp"
#include "teasafe/FileStreamPtr.hpp"
#include "utility/EcholessPasswordPrompt.hpp"

#include <boost/iostreams/copy.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/regex.hpp>

#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>



/// the 'ls' command for listing dir contents
void com_ls(teasafe::TeaSafe &theBfs, std::string const &path)
{
    std::string thePath(*path.begin() != '/' ? "/" : "");
    thePath.append(path);
    teasafe::TeaSafeFolder folder = theBfs.getTeaSafeFolder(thePath);
    std::vector<teasafe::EntryInfo> entries = folder.listAllEntries();
    std::vector<teasafe::EntryInfo>::iterator it = entries.begin();
    for (; it != entries.end(); ++it) {
        if (it->type() == teasafe::EntryType::FileType) {
            std::cout<<boost::format("%1% %|30t|%2%\n") % it->filename() % "<F>";
        } else {
            std::cout<<boost::format("%1% %|30t|%2%\n") % it->filename() % "<D>";
        }
    }
}

/// the 'rm' command for removing a folder or file
void com_rm(teasafe::TeaSafe &theBfs, std::string const &path)
{
    std::string thePath(*path.begin() != '/' ? "/" : "");
    thePath.append(path);
    teasafe::EntryInfo info = theBfs.getInfo(thePath);
    if (info.type() == teasafe::EntryType::FileType) {
        theBfs.removeFile(thePath);
    } else {
        theBfs.removeFolder(thePath, teasafe::FolderRemovalType::Recursive);
    }
}

/// the 'mkdir' command for adding a folder to the current working dir
void com_mkdir(teasafe::TeaSafe &theBfs, std::string const &path)
{
    std::string thePath(*path.begin() != '/' ? "/" : "");
    thePath.append(path);
    theBfs.addFolder(thePath);
}

/// the 'add' command for copying a file from the physical fs to the current
/// working directory
/// example usage:
/// add file://path/to/file.txt
void com_add(teasafe::TeaSafe &theBfs, std::string const &parent, std::string const &fileResource)
{
    // add the file to the container
    // this removed the first several chars assumes to be "file://"
    std::string resPath(fileResource.begin() + 7, fileResource.end());
    boost::filesystem::path p(resPath);
    std::string addPath(parent);
    (void)addPath.append(p.filename().string());

    theBfs.addFile(addPath);

    // create a stream to read resource from and a device to write to
    std::ifstream in(resPath.c_str(), std::ios_base::binary);
    teasafe::TeaSafeFileDevice device = theBfs.openFile(addPath, teasafe::OpenDisposition::buildWriteOnlyDisposition());
    boost::iostreams::copy(in, device);
}

/// for extracting a teasafe file to somewhere on a physical disk location
/// example usage:
/// extract file.txt file:///some/parent/path/
void com_extract(teasafe::TeaSafe &theBfs, std::string const &path, std::string const &dst)
{
    // resolve the destination by removing first 7 chars assumed to be 'file://'
    std::string dstPath(dst.begin() + 7, dst.end());

    // make sure destination parent has a trailing slash on the end
    if(*dstPath.rbegin() != '/') {
        dstPath.append("/");
    }

    // append filename on to dst path
    boost::filesystem::path p(path);
    dstPath.append(p.filename().string());

    // create source and sink
    teasafe::EntryInfo info = theBfs.getInfo(path);
    teasafe::TeaSafeFileDevice device = theBfs.openFile(path, teasafe::OpenDisposition::buildReadOnlyDisposition());
    std::ofstream out(dstPath.c_str(), std::ios_base::binary);
    boost::iostreams::copy(device, out);
}

/// takes a path and pushes a new path bit to it, going into that path
/// example usage when working path is /hello
/// push there
/// result: /hello/there
void com_push(teasafe::TeaSafe &theBfs, std::string &workingDir, std::string &fragment)
{
    std::string thePath(workingDir);
    if (*thePath.rbegin() != '/') {
        (void)thePath.append("/");
    }
    (void)thePath.append(fragment);
    if (theBfs.folderExists(thePath)) {
        workingDir = thePath;
    }
}

/// takes a path and pops a path bit from it, going into that path
/// example usage when working path is /hello/there
/// pop
/// result: /hello
void com_pop(teasafe::TeaSafe &theBfs, std::string &workingDir)
{
    boost::filesystem::path p(workingDir);

    if (!p.has_parent_path()) {
        workingDir = "/";
        return;
    }

    if (theBfs.folderExists(p.parent_path().string())) {
        workingDir = p.parent_path().string();
    }
}

/// for changing to a new folder. Path should be absolute.
void com_cd(teasafe::TeaSafe &theBfs, std::string &workingDir, std::string const &path)
{
    if (theBfs.folderExists(path)) {
        workingDir = path;
    }
}

std::string formattedPath(std::string const &workingDir, std::string const &path)
{
    std::string wd(workingDir);
    // absolute
    if (*path.begin() == '/') {
        return path;
    }
    // or relative
    return std::string(wd.append(path));
}

/// parses the command string
void parse(teasafe::TeaSafe &theBfs, std::string const &commandStr, std::string &workingDir)
{
    std::vector<std::string> comTokens;
    boost::algorithm::split_regex(comTokens, commandStr, boost::regex("\\s+"));

    if (comTokens[0] == "ls") {
        if (comTokens.size() > 1) {
            com_ls(theBfs, formattedPath(workingDir, comTokens[1]));
        }
        com_ls(theBfs, workingDir);
    } else if (comTokens[0] == "pwd") {
        std::cout<<workingDir<<std::endl;
    } else if (comTokens[0] == "rm") {
        if (comTokens.size() < 2) {
            std::cout<<"Error: please specify path"<<std::endl;
        } else {
            com_rm(theBfs, formattedPath(workingDir, comTokens[1]));
        }
    } else if (comTokens[0] == "mkdir") {
        if (comTokens.size() < 2) {
            std::cout<<"Error: please specify path"<<std::endl;
        } else {
            com_mkdir(theBfs, formattedPath(workingDir, comTokens[1]));
        }
    } else if (comTokens[0] == "add") {
        if (comTokens.size() < 2) {
            std::cout<<"Error: please specify path"<<std::endl;
        } else {
            com_add(theBfs, workingDir, comTokens[1]);
        }
    } else if (comTokens[0] == "push") {
        if (comTokens.size() < 2) {
            std::cout<<"Error: please specify path"<<std::endl;
        } else {
            com_push(theBfs, workingDir, comTokens[1]);
        }
    } else if (comTokens[0] == "pop") {
        com_pop(theBfs, workingDir);
    } else if (comTokens[0] == "cd") {
        if (comTokens.size() < 2) {
            std::cout<<"Error: please specify path"<<std::endl;
        } else {
            com_cd(theBfs, workingDir, formattedPath(workingDir, comTokens[1]));
        }
    } else if (comTokens[0] == "extract") {
        if (comTokens.size() < 3) {
            std::cout<<"Error: please specify src and dst paths"<<std::endl;
        } else {
            com_extract(theBfs, formattedPath(workingDir, comTokens[1]), comTokens[2]);
        }
    }
}

/// indefinitely loops over user input expecting commands
int loop(teasafe::TeaSafe &theBfs)
{
    std::string currentPath("/");
    while (1) {
        std::string commandStr;
        std::cout<<"ts$> ";
        getline(std::cin, commandStr);
        try {
            parse(theBfs, commandStr, currentPath);
        } catch (...) {
            std::cout<<"Some error occurred!"<<std::endl;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // parse the program options
    uint64_t rootBlock;
    bool magic = false;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("imageName", po::value<std::string>(), "teasafe image path")
        ("coffee", po::value<bool>(&magic)->default_value(false), "mount alternative sub-volume")
        ;

    po::positional_options_description positionalOptions;
    (void)positionalOptions.add("imageName", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).
                  options(desc).positional(positionalOptions).run(),
                  vm);
        po::notify(vm);
        if (vm.count("help") ||
            vm.count("imageName") == 0) {
            std::cout << desc << std::endl;
            return 1;
        }

        if (vm.count("help")) {
            std::cout<<desc<<"\n";
        } else {

            std::cout<<"image path: "<<vm["imageName"].as<std::string>()<<std::endl;
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
    io->password = teasafe::utility::getPassword("teasafe password: ");
    io->rootBlock = magic ? atoi(teasafe::utility::getPassword("magic number: ").c_str()) : 0;

    // Obtain the initialization vector from the first 8 bytes
    // and the number of xtea rounds from the ninth byte
    {
        std::ifstream in(io->path.c_str(), std::ios::in | std::ios::binary);
        std::vector<uint8_t> ivBuffer;
        ivBuffer.resize(8);
        (void)in.read((char*)&ivBuffer.front(), teasafe::detail::IV_BYTES);
        char i;
        (void)in.read((char*)&i, 1);
        // note, i should always > 0 <= 255
        io->rounds = (unsigned int)i;
        in.close();
        io->iv = teasafe::detail::convertInt8ArrayToInt64(&ivBuffer.front());
    }

    // Obtain the number of blocks in the image by reading the image's block count
    teasafe::TeaSafeImageStream stream(io, std::ios::in | std::ios::binary);
    io->blocks = teasafe::detail::getBlockCount(stream);

    printf("Counting allocated blocks. Please wait...\n");

    io->freeBlocks = io->blocks - teasafe::detail::getNumberOfAllocatedBlocks(stream);
    io->blockBuilder = boost::make_shared<teasafe::FileBlockBuilder>(io);

    printf("Finished counting allocated blocks.\n");

    stream.close();

    // Create the basic file system
    teasafe::TeaSafe theBfs(io);
    return loop(theBfs);
}
