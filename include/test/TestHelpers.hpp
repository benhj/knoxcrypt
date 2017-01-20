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

#pragma once

#include "cryptostreampp/Algorithms.hpp"
#include "knoxcrypt/CoreIO.hpp"
#include "knoxcrypt/FileBlockBuilder.hpp"
#include "knoxcrypt/KnoxCryptException.hpp"
#include "utility/MakeKnoxCrypt.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

#include <ctime>
#include <string>
#include <vector>

int const HELLO_IT = 10000;
int const BIG_SIZE = HELLO_IT * 13;
int const A_STRING_SIZE = 998;

int testFailures = 0;
int passedPoints = 0;
std::vector<std::string> failingTestPoints;

knoxcrypt::SharedCoreIO createTestIO(boost::filesystem::path const &testPath)
{
    knoxcrypt::SharedCoreIO io = std::make_shared<knoxcrypt::CoreIO>();
    io->path = testPath.string();
    io->blocks = 2048;
    io->freeBlocks = 2048;
    io->encProps.password = "abcd1234";
    io->encProps.iv = uint64_t(3081342484970028645);
    io->encProps.iv2 = uint64_t(3081342484970028645);
    io->encProps.iv3 = uint64_t(3081342484970028645);
    io->encProps.iv4 = uint64_t(3081342484970028645);
    io->rounds = 64;
    io->encProps.cipher = cryptostreampp::Algorithm::AES;
    io->rootBlock = 0;
    io->blockBuilder = std::make_shared<knoxcrypt::FileBlockBuilder>(io);
    io->useBlockCache = false;
    io->firstTimeInit = false;
    return io;
}

inline boost::filesystem::path buildImage(boost::filesystem::path const &path)
{
    std::string testImage(boost::filesystem::unique_path().string());
    boost::filesystem::path testPath = path / testImage;
    knoxcrypt::SharedCoreIO io(createTestIO(testPath));
    bool const sparse = true; // quicker testing with sparse images
    knoxcrypt::MakeKnoxCrypt kc(io, sparse);
    kc.buildImage();
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

knoxcrypt::SharedBlockBuilder testBlockBuilder()
{
    return std::make_shared<knoxcrypt::FileBlockBuilder>();
}
