#ifndef BFS_TEST_HELPERS_HPP__
#define BFS_TEST_HELPERS_HPP__

#include "bfs/CoreBFSIO.hpp"
#include "bfs/MakeBFS.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

#include <string>
#include <vector>

int const HELLO_IT = 1000;
int const BIG_SIZE = HELLO_IT * 13;
int const A_STRING_SIZE = 998;

int testFailures = 0;
int passedPoints = 0;
std::vector<std::string> failingTestPoints;

inline boost::filesystem::path buildImage(boost::filesystem::path const &path, long const blockCount)
{
    std::string testImage(boost::filesystem::unique_path().string());
    boost::filesystem::path testPath = path / testImage;
    bfs::CoreBFSIO io;
    io.path = testPath.string();
    io.blocks = blockCount;
    io.password = "abcd1234";
    bfs::MakeBFS bfs(io);
    return testPath;
}

std::string createLargeStringToWrite(std::string const &val="Hello, World!")
{
    std::string theString("");
    for (int i = 0; i < HELLO_IT; ++i) {
        theString.append(val);
    }
    return theString;
}

std::string createAString()
{
    std::string theString("");
    for (int i = 0; i < A_STRING_SIZE; ++i) {
        theString.append("a");
    }
    return theString;
}

#define ASSERT_EQUAL(A, B, C)                                          \
    if(A == B) {                                                       \
        std::cout<<boost::format("%1% %|100t|%2%\n") % C % "passed";   \
        ++passedPoints;                                                \
    } else {                                                           \
        std::cout<<boost::format("%1% %|100t|%2%\n") % C % "failed";   \
        ++testFailures;                                                \
        failingTestPoints.push_back(C);                                \
    }

#endif
