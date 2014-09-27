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

/// For recursively copying a physical disk location to a TeaSafeFolder

#ifndef TeaSafe_UTILITY_RECURSIVE_FOLDER_ADDER_HPP__
#define TeaSafe_UTILITY_RECURSIVE_FOLDER_ADDER_HPP__

#include "teasafe/TeaSafe.hpp"
#include "teasafe/EntryType.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>

#include <fstream>
#include <sstream>
#include <vector>

namespace teasafe
{

    namespace utility
    {

        inline
        void recursiveAdd(TeaSafe &theBfs,
                          std::string const &teaPath,
                          std::string const &fsPath,
                          std::function<void(std::string)> const &callback)
        {
            boost::filesystem::path p(fsPath);
            boost::filesystem::directory_iterator itr(p);
            boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
            for ( ; itr != end_itr; ++itr ) {
                boost::filesystem::path tp(teaPath);
                boost::filesystem::path fs(p);
                fs /= itr->path().filename();
                tp /= itr->path().filename();
                if ( boost::filesystem::is_directory(itr->status()) ) {
                    std::stringstream ss;
                    ss << "Adding "<<tp<<"...";
                    callback(ss.str());
                    theBfs.addFolder(tp.string());
                    recursiveAdd(theBfs, tp.string(), fs.string(), callback);
                } else {
                    std::stringstream ss;
                    ss << "Adding "<<tp<<"...";
                    callback(ss.str());
                    theBfs.addFile(tp.string());
                    teasafe::TeaSafeFileDevice device = theBfs.openFile(tp.string(), teasafe::OpenDisposition::buildWriteOnlyDisposition());
                    std::ifstream in(fs.string().c_str(), std::ios_base::binary);
                    boost::iostreams::copy(in, device);
                }
            }
        }
    }
}

#endif // TeaSafe_UTILITY_RECURSIVE_FOLDER_ADDER_HPP__
