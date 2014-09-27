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

/// Extracts a file or folder to a physical location

#ifndef TeaSafe_UTILITY_EXTRACT_TO_PHYSICAL_HPP__
#define TeaSafe_UTILITY_EXTRACT_TO_PHYSICAL_HPP__

#include "teasafe/TeaSafe.hpp"
#include "utility/TeaSafeFolderVisitor.hpp"
#include "utility/RecursiveFolderExtractor.hpp"
#include "utility/FolderExtractionVisitor.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/iostreams/copy.hpp>

#include <fstream>
#include <sstream>

namespace teasafe
{

    namespace utility
    {

        inline
        void extractToPhysical(teasafe::TeaSafe &theBfs,
                              std::string const &path,
                              std::string const &dst,
                              std::function<void(std::string)> callback)
        {
            std::string dstPath(dst);

            // make sure destination parent has a trailing slash on the end
            if(*dstPath.rbegin() != '/') {
                dstPath.append("/");
            }

            // append filename on to dst path
            boost::filesystem::path p(path);
            dstPath.append(p.filename().string());

            // create source and sink
            if(theBfs.fileExists(path)) {
                std::stringstream ss;
                ss << "Extracting file "<<dstPath<<"...";
                callback(dstPath);
                teasafe::TeaSafeFileDevice device = theBfs.openFile(path, teasafe::OpenDisposition::buildReadOnlyDisposition());
                device.seek(0, std::ios_base::beg);
                std::ofstream out(dstPath.c_str(), std::ios_base::binary);
                boost::iostreams::copy(device, out);
            } else if(theBfs.folderExists(path)) {
                boost::filesystem::create_directory(dstPath);
                FolderExtractionVisitor visitor(theBfs, path, dstPath, callback);
                recursiveExtract(visitor, theBfs, path);
            }
        }
    }
}

#endif // TeaSafe_UTILITY_EXTRACT_TO_PHYSICAL_HPP__

