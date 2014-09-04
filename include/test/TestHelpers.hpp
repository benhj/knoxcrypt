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

#ifndef TeaSafe_TEST_HELPERS_HPP__
#define TeaSafe_TEST_HELPERS_HPP__

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/TeaSafeException.hpp"
#include "utility/MakeTeaSafe.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>

#include <ctime>
#include <string>
#include <vector>

int const HELLO_IT = 1000;
int const BIG_SIZE = HELLO_IT * 13;
int const A_STRING_SIZE = 998;

int testFailures = 0;
int passedPoints = 0;
std::vector<std::string> failingTestPoints;

teasafe::SharedCoreIO createTestIO(boost::filesystem::path const &testPath)
{
    teasafe::SharedCoreIO io = boost::make_shared<teasafe::CoreTeaSafeIO>();
    io->path = testPath.string();
    io->blocks = 2048;
    io->freeBlocks = 2048;
    io->password = "abcd1234";
    io->iv = uint64_t(3081342484970028645);
    io->iv2 = uint64_t(3081342484970028645);
    io->iv3 = uint64_t(3081342484970028645);
    io->iv4 = uint64_t(3081342484970028645);
    io->rounds = 64;
    io->cipher = 7; // aes
    io->rootBlock = 0;
    io->blockBuilder = boost::make_shared<teasafe::FileBlockBuilder>(io);
    return io;
}

inline boost::filesystem::path buildImage(boost::filesystem::path const &path)
{
    std::string testImage(boost::filesystem::unique_path().string());
    boost::filesystem::path testPath = path / testImage;
    teasafe::SharedCoreIO io(createTestIO(testPath));
    bool const sparse = true; // quicker testing with sparse images
    teasafe::MakeTeaSafe teasafe(io, sparse);
    teasafe.buildImage();
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

teasafe::SharedBlockBuilder testBlockBuilder()
{
    return boost::make_shared<teasafe::FileBlockBuilder>();
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
