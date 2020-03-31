/*
  Copyright (c) <2014-2015>, <BenHJ>
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
 * @brief a *very hacky* wrapper for accessing / modifying knoxcrypt containers
 *
 * Attempts to provide basic 'shell-like' access to give a real fs feel
 * (has 'ls', 'cd', 'pwd', 'rm', 'mkdir' commands)
 */

#include "knoxcrypt/CoreIO.hpp"
#include "knoxcrypt/EntryInfo.hpp"
#include "knoxcrypt/CompoundFolderEntryIterator.hpp"
#include "knoxcrypt/FileBlockBuilder.hpp"
#include "knoxcrypt/CoreFS.hpp"
#include "knoxcrypt/KnoxCryptException.hpp"
#include "knoxcrypt/FileStreamPtr.hpp"
#include "utility/CipherCallback.hpp"
#include "utility/CopyFromPhysical.hpp"
#include "utility/EcholessPasswordPrompt.hpp"
#include "utility/ExtractToPhysical.hpp"
#include "utility/RecursiveFolderAdder.hpp"
#include "utility/RemoveEntry.hpp"
#include "utility/PassHasher.hpp"

#include <boost/iostreams/copy.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string/regex.hpp>

#include <fstream>
#include <functional>
#include <iostream>
#include <stdint.h>
#include <vector>

/// called back from RecursiveFolderAdder
void operationCallback(std::string const &str)
{
    std::cout<<str<<std::endl;
}

/// describes a particular command
struct CommandDescriptor
{
    CommandDescriptor(std::string const &command_,
                      std::string const &desc_,
                      std::string const &usage_)
      : command(command_)
      , desc(desc_)
      , usage(usage_)
    {

    }
    std::string command;
    std::string desc;
    std::string usage;
};

/// stores all available commands
using Commands = std::vector<CommandDescriptor>;
Commands g_availableCommands;

/// lists all available commands
void com_help()
{
    std::cout<<std::endl;
    std::cout<<boost::format("%1% %|20t|%2% %|80t|%3%\n") % "Command" % "Description" % "Usage";
    std::cout<<boost::format("%1% %|20t|%2% %|80t|%3%\n") % "-------" % "-----------" % "-----";
    std::cout<<std::endl;
    for(auto const & it : g_availableCommands) {
        std::cout<<boost::format("%1% %|20t|%2% %|80t|%3%\n") % it.command % it.desc % it.usage;
    }
    std::cout<<std::endl;
}

/// the 'ls' command for listing dir contents
void com_ls(knoxcrypt::CoreFS &theBfs, std::string const &path)
{
    // make sure path begins with a slash
    std::string thePath(*path.begin() != '/' ? "/" : "");
    thePath.append(path);

    // iterate over entries in folder and print filenames of each
    auto folder = theBfs.getFolder(thePath);
    auto it = folder.listAllEntries();
    knoxcrypt::CompoundFolderEntryIterator end;
    while(it != end) {
        if ((*it)->type() == knoxcrypt::EntryType::FileType) {
            std::cout<<boost::format("%1% %|30t|%2%\n") % (*it)->filename() % "<F>";
        } else {
            std::cout<<boost::format("%1% %|30t|%2%\n") % (*it)->filename() % "<D>";
        }
        ++it;
    }
}

/// attempts to implement tab complete by matching the provided string
/// with an entry at the given parent path
std::string tabCompleteknoxcryptEntry(knoxcrypt::CoreFS &theBfs, std::string const &path)
{
    // make sure path begins with a slash
    std::string thePath(*path.begin() != '/' ? "/" : "");
    thePath.append(path);

    // find out the parent of the thing we're trying to tab complete
    // the item we're trying to tab-complete should be a child of this parent
    boost::filesystem::path bp(path);
    auto parentPath(bp.parent_path().string());

    // get the parent folder
    auto folder = theBfs.getFolder(parentPath);

    // iterate over entries in folder
    auto it = folder.listAllEntries();
    knoxcrypt::CompoundFolderEntryIterator end;
    while(it != end) {
        // try to match the entry with the thing that we want to tab-complete
        auto extracted((*it)->filename().substr(0, bp.filename().string().length()));
        if(extracted == bp.filename()) {
            return (*it)->filename(); // match, return name of entry
        }
        ++it;
    }

    return bp.filename().string(); // no match, return non tab-completed token
}

/// attempts to tab-complete a command
std::string tabCompleteCommand(std::string const &command)
{
    // iterate over entries in folder
    for (auto const & it : g_availableCommands) {

        // try to match the entry with the thing that we want to tab-complete
        auto extracted(it.command.substr(0, command.length()));
        if(extracted == command) {
            return it.command; // match, return name of command
        }
    }
    return command; // no match, return un tab-completed version
}

/// the 'rm' command for removing a folder or file
void com_rm(knoxcrypt::CoreFS &theBfs, std::string const &path)
{
    std::string thePath(*path.begin() != '/' ? "/" : "");
    thePath.append(path);
    knoxcrypt::utility::removeEntry(theBfs, thePath);
}

/// the 'mkdir' command for adding a folder to the current working dir
void com_mkdir(knoxcrypt::CoreFS &theBfs, std::string const &path)
{
    std::string thePath(*path.begin() != '/' ? "/" : "");
    thePath.append(path);
    theBfs.addFolder(thePath);
}

/// the 'add' command for copying a file from the physical fs to the current
/// working directory
/// example usage:
/// add file://path/to/file.txt
void com_add(knoxcrypt::CoreFS &theBfs, std::string const &parent, std::string const &fileResource)
{
    // add the file to the container
    // this removed the first several chars assumes to be "file://"
    std::string resPath(fileResource.begin() + 7, fileResource.end());
    knoxcrypt::utility::copyFromPhysical(theBfs, parent, resPath, std::bind(operationCallback,
                                                                          std::placeholders::_1));
}

/// for extracting a knoxcrypt file to somewhere on a physical disk location
/// example usage:
/// extract file.txt file:///some/parent/path/
void com_extract(knoxcrypt::CoreFS &theBfs, std::string const &path, std::string const &dst)
{
    std::string dstPath(dst.begin() + 7, dst.end());
    knoxcrypt::utility::extractToPhysical(theBfs, path, dstPath, std::bind(operationCallback,
                                                                         std::placeholders::_1));
}

/// takes a path and pushes a new path bit to it, going into that path
/// example usage when working path is /hello
/// push there
/// result: /hello/there
void com_push(knoxcrypt::CoreFS &theBfs, std::string &workingDir, std::string &fragment)
{
    auto thePath(workingDir);
    if (*thePath.rbegin() != '/') {
        (void)thePath.append("/");
    }
    (void)thePath.append(fragment);
    if (theBfs.folderExists(thePath)) {
        workingDir = thePath;
    } else {
        std::cout<<"Not a folder or not found"<<std::endl;
    }
}

/// takes a path and pops a path bit from it, going into that path
/// example usage when working path is /hello/there
/// pop
/// result: /hello
void com_pop(knoxcrypt::CoreFS &theBfs, std::string &workingDir)
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
void com_cd(knoxcrypt::CoreFS &theBfs, std::string &workingDir, std::string const &path)
{
    // try to directly resolve
    if (theBfs.folderExists(path)) {
        workingDir = path;
    } else {
        std::cout<<"Not a folder or not found"<<std::endl;
    }
}

/// removes '.' and '..' bits
/// from http://stackoverflow.com/questions/1746136/how-do-i-normalize-a-pathname-using-boostfilesystem
boost::filesystem::path normalize(const boost::filesystem::path &path)
{
    auto absPath = boost::filesystem::absolute(path);
    auto it = absPath.begin();
    auto result = *it++;

    // For the rest remove ".." and "." in a path with no symlinks
    for (; it != absPath.end(); ++it) {
        // Just move back on ../
        if (*it == "..") {
            result = result.parent_path();
        }
        // Ignore "."
        else if (*it != ".") {
            // Just cat other path entries
            result /= *it;
        }
    }

    return result;
}

/// gets the correct path to cd in to
std::string formattedPath(std::string const &workingDir, std::string const &path)
{
    auto wd(workingDir);
    // absolute
    if (*path.begin() == '/') {
        return path;
    }
    // or relative
    if(*wd.rbegin() != '/') {
        (void)wd.append("/");
    }

    // resolve '.' and '..' bits before returning
    boost::filesystem::path p(wd.append(path));
    boost::filesystem::path normalized(normalize(p));
    return std::string(normalized.string());
}

/// parses the command string
void parse(knoxcrypt::CoreFS &theBfs, std::string const &commandStr, std::string &workingDir)
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
            std::cout<<"Error: please specify /path"<<std::endl;
        } else {
            com_rm(theBfs, formattedPath(workingDir, comTokens[1]));
        }
    } else if (comTokens[0] == "mkdir") {
        if (comTokens.size() < 2) {
            std::cout<<"Error: please specify /path"<<std::endl;
        } else {
            com_mkdir(theBfs, formattedPath(workingDir, comTokens[1]));
        }
    } else if (comTokens[0] == "add") {
        if (comTokens.size() < 2) {
            std::cout<<"Error: please specify file:///path"<<std::endl;
        } else {
            com_add(theBfs, workingDir, comTokens[1]);
        }
    } else if (comTokens[0] == "push") {
        if (comTokens.size() < 2) {
            std::cout<<"Error: please specify /path"<<std::endl;
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
            std::cout<<"Error: please specify /src/path and file:///dst/parent/path/"<<std::endl;
        } else {
            com_extract(theBfs, formattedPath(workingDir, comTokens[1]), comTokens[2]);
        }
    } else if (comTokens[0] == "help") {
        com_help();
    } else if (comTokens[0] == "quit") {
        exit(0);
    } else if (comTokens[0] == "exit") {
        exit(0);
    }
}

/// removes need to press enter / return when using getchar
/// prevents 'double echo' of characters when using getchar by disabling echo
void setupTerminal()
{
    static struct termios oldt, newt;

    /// See http://stackoverflow.com/questions/1798511/how-to-avoid-press-enter-with-any-getchar

    /*tcgetattr gets the parameters of the current terminal
    STDIN_FILENO will tell tcgetattr that it should write the settings
    of stdin to oldt*/
    tcgetattr( STDIN_FILENO, &oldt);
    /*now the settings will be copied*/
    newt = oldt;

    /*ICANON normally takes care that one line at a time will be processed
    that means it will return if it sees a "\n" or an EOF or an EOL*/
    newt.c_lflag &= ~(ICANON);

    // also disable standard output -- Ben
    newt.c_lflag &= ~ECHO;
    newt.c_lflag |= ECHONL;
    newt.c_lflag &= ICRNL;

    /*Those new settings will be set to STDIN
    TCSANOW tells tcsetattr to change attributes immediately. */
    tcsetattr( STDIN_FILENO, TCSANOW, &newt);
}

/// handles the eventuality of a backspace or delete key having been pressed
void handleBackspace(int &cursorPos, std::string &toReturn)
{
    // check that the cursor position is > 0 to prevent accidental
    // deletion of prompt!
    if(cursorPos > 0) {
        // backspace which only moves cursor, so need to write a blank
        // space over the original char immediately afterwards
        // to emulate having removed it
        std::cout<<"\b ";
        // now move cursor back again
        std::cout<<"\b";
        --cursorPos;
        std::string copy(toReturn.begin(), toReturn.end() - 1);
        copy.swap(toReturn);
    }
}

/// handles tab-key; attempts to provide rudimentary tab-completion
void handleTabKey(knoxcrypt::CoreFS &theBfs,
                  std::string const &workingPath,
                  int &cursorPos,
                  std::string &toReturn)
{
    // get the token from the string that we're tring to tab-complete
    std::cout<<"\n";
    std::vector<std::string> comTokens;
    boost::algorithm::split_regex(comTokens, toReturn, boost::regex("\\s+"));

    // if the user hasn't entered anything, simply list the available commands
    if(toReturn.empty()) {
        com_help();
        std::cout<<"ts$> "<<toReturn;
        return;
    }

    // else process which bit to tab-complete
    std::string tabCompleted;
    size_t len;

    if(comTokens.size() > 1) {
        auto toBeCompleted(comTokens[comTokens.size()-1]);

        // make sure the working path is correctly formatted
        auto wd(workingPath);

        // if relative then append a slash to working path
        if(*toBeCompleted.begin() != '/') {

            // only append path seperator if wd isn't already root
            // which by definition, is '/'
            if(wd != "/") {
                wd.append("/");
            }

            // append the bit that we want to tab-complete
            (void)wd.append(toBeCompleted);
        } else { // absolute
            // path begins with a '/' so must be absolute, thus
            // make workng path the actual token
            wd = toBeCompleted;
        }

        // run the tab-completion algorithm and get the tab-completed string back
        tabCompleted = tabCompleteknoxcryptEntry(theBfs, wd);
        len = (boost::filesystem::path(toBeCompleted).filename()).string().length();

    } else {
        tabCompleted = tabCompleteCommand(comTokens[0]);
        len = comTokens[0].length();
    }

    // how long is the original fname length prior to tab-completion?
    // Subtract from the original string this many characyers
    std::string copy(toReturn.begin(), toReturn.end() - len);
    copy.swap(toReturn);
    cursorPos -= len;

    // now append tab completed bit
    toReturn.append(tabCompleted);

    // updated cursor position based on returned string
    cursorPos += tabCompleted.length();

    // after effect of pressing tab, need to print out prompt
    // and where we were with toReturn again
    std::cout<<"ts$> "<<toReturn;
}

/// gets a user-inputted string until return is entered
/// will use getChar to process characters one by one so that individual
/// key handlers can be created
std::string getInputString(knoxcrypt::CoreFS &theBfs, std::string const &workingPath)
{
    std::string toReturn("");

    // disable echo and prevent need to press enter for getchar flush
    setupTerminal();

    int cursorPos(0);
    while(1) {
        char c = getchar();
        if((int)c == 10) { // enter
            std::cout<<std::endl;
            break;
        }
        if((int)c == 127 || (int)c == 8) { // delete / backspace

            handleBackspace(cursorPos, toReturn);

        } else if((int)c == 9) { // tab

            handleTabKey(theBfs, workingPath, cursorPos, toReturn);

        } else { // print out char to screen and push into string vector
            std::cout<<c;
            ++cursorPos;
            toReturn.push_back(c);
        }
    }
    return toReturn;
}

/// indefinitely loops over user input expecting commands
int loop(knoxcrypt::CoreFS &theBfs)
{
    std::string currentPath("/");
    while (1) {
        std::string commandStr;
        std::cout<<"ts$> ";
        commandStr = getInputString(theBfs, currentPath);
        try {
            parse(theBfs, commandStr, currentPath);
        } catch (...) {
            std::cout<<"Some error occurred!"<<std::endl;
        }
    }
    return 0;
}

void populateCommands()
{
    {
        CommandDescriptor command("ls","list entries in current folder","ls");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("pwd","display current working dir","pwd");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("mkdir","create folder","mkdir <folderName>");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("push","change to folder","push <folderName>");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("pop","change to previous folder","pop");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("cd","change to folder","cd <folderName>");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("rm","remove entry","rm <entryName>");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("add","add a file or folder to current working dir","add <file:///path/to/thing>");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("extract","extract a file or folder","extract <entryName> <file:///place/to/extract>");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("help","list available commands","help");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("quit","exit the shell","quit");
        g_availableCommands.push_back(command);
    }
    {
        CommandDescriptor command("exit","exit the shell","exit");
        g_availableCommands.push_back(command);
    }
}

int main(int argc, char *argv[])
{
    // parse the program options
    bool magic = false;
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message")
        ("imageName", po::value<std::string>(), "knoxcrypt image path")
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

    populateCommands();

    // Setup a core knoxcrypt io object which stores highlevel info about accessing
    // the knoxcrypt image
    auto io(std::make_shared<knoxcrypt::CoreIO>());
    io->useBlockCache = true;
    io->path = vm["imageName"].as<std::string>().c_str();
    io->encProps.password = knoxcrypt::utility::getPassword("knoxcrypt password: ");
    io->rootBlock = magic ? atoi(knoxcrypt::utility::getPassword("magic number: ").c_str()) : 0;

    // Obtain the initialization vector from the first 8 bytes
    // and the number of xtea rounds from the ninth byte
    knoxcrypt::detail::readImageIVAndRounds(io);

    // Obtain the number of blocks in the image by reading the image's block count
    long const amount = knoxcrypt::detail::CIPHER_BUFFER_SIZE / 100000;
    auto f(std::bind(&knoxcrypt::cipherCallback, std::placeholders::_1, amount));
    io->ccb = f;
    knoxcrypt::ContainerImageStream stream(io, std::ios::in | std::ios::binary);

    // compare password hashes
    uint8_t hashRecovered[32];
    knoxcrypt::detail::getPassHash(stream, hashRecovered);
    uint8_t hashEntered[32];
    knoxcrypt::utility::sha256((char*)io->encProps.password.c_str(), hashEntered);
    if(!knoxcrypt::utility::compareTwoHashes(hashEntered, hashRecovered)) {
        std::cout<<"Incorrect password"<<std::endl;
        exit(0);
    }

    io->blocks = knoxcrypt::detail::getBlockCount(stream);

    printf("Counting allocated blocks. Please wait...\n");

    io->freeBlocks = io->blocks - knoxcrypt::detail::getNumberOfAllocatedBlocks(stream);
    io->blockBuilder = std::make_shared<knoxcrypt::FileBlockBuilder>(io);

    printf("Finished counting allocated blocks.\n");

    stream.close();

    // Create the basic file system
    knoxcrypt::CoreFS theBfs(io);
    return loop(theBfs);
}
