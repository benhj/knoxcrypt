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

/// Copies a file or folder from a physical location to a parent teasafe location

#ifndef TeaSafe_UTILITY_COPY_FROM_PHYSICAL_HPP__
#define TeaSafe_UTILITY_COPY_FROM_PHYSICAL_HPP__

#include "teasafe/TeaSafe.hpp"
#include "utility/RecursiveFolderAdder.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/function.hpp>

namespace teasafe
{

    namespace utility
    {

        inline
        void copyFromPhysical(teasafe::TeaSafe &theBfs,
                              std::string const &teaPath,
                              std::string const &fsPath,
                              boost::function<void(std::string)> const &callback)
        {

            boost::filesystem::path p(fsPath);
            std::string addPath(teaPath);

            if(*addPath.rbegin() != '/') {
                addPath.append("/");
            }

            (void)addPath.append(p.filename().string());

            if ( boost::filesystem::is_directory(p)) {
                theBfs.addFolder(addPath);
                recursiveAdd(theBfs, addPath, p.string(), callback);
            } else {

                std::stringstream ss;
                ss << "Adding "<<addPath<<"...";
                callback(addPath);

                theBfs.addFile(addPath);
                // create a stream to read resource from and a device to write to
                std::ifstream in(fsPath.c_str(), std::ios_base::binary);
                teasafe::TeaSafeFileDevice device = theBfs.openFile(addPath, teasafe::OpenDisposition::buildWriteOnlyDisposition());
                boost::iostreams::copy(in, device);
            }
        }
    }
}

#endif // TeaSafe_UTILITY_COPY_FROM_PHYSICAL_HPP__

