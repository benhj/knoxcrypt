/*
  The MIT License (MIT)

  Copyright (c) 2013 Ben H.D. Jones

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
  the Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

bfs::CoreBFSIO createTestIO(boost::filesystem::path const &testPath)
{
    bfs::CoreBFSIO io;
    io.path = testPath.string();
    io.blocks = 2048;
    io.password = "abcd1234";
    io.rootBlock = 0;
    return io;
}

inline boost::filesystem::path buildImage(boost::filesystem::path const &path, long const blockCount)
{
    std::string testImage(boost::filesystem::unique_path().string());
    boost::filesystem::path testPath = path / testImage;
    bfs::CoreBFSIO io = createTestIO(testPath);
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
