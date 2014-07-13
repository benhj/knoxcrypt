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

/// A visitor for extracting fles and folders

#ifndef TeaSafe_UTILITY_FOLDER_EXTRACTION_VISITOR_HPP__
#define TeaSafe_UTILITY_FOLDER_EXTRACTION_VISITOR_HPP__

#include "teasafe/EntryInfo.hpp"
#include "teasafe/TeaSafe.hpp"
#include "teasafe/TeaSafeFileDevice.hpp"
#include "utility/TeaSafeFolderVisitor.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/iostreams/copy.hpp>

namespace teasafe
{

    namespace utility
    {

        class FolderExtractionVisitor : public TeaSafeFolderVisitor
        {
          public:
            FolderExtractionVisitor(TeaSafe &theBfs,
                                    std::string teaPath,
                                    std::string fsPath)
            : m_theBfs(theBfs)
            , m_teaPath(teaPath)
            , m_fsPath(fsPath)
            {
            }


            virtual void enterFolder(EntryInfo const &it)
            {
                // get the parent folder
                boost::filesystem::path fsLoc(m_fsPath);
                fsLoc /= it.filename();
                boost::filesystem::path teaLoc(m_teaPath);
                teaLoc /= it.filename();
                std::cout<<"Extracting folder "<<fsLoc<<"..."<<std::endl;
                boost::filesystem::create_directory(fsLoc);
                m_fsPath = fsLoc.string();
                m_teaPath = teaLoc.string();
            }

            virtual void enterFile(EntryInfo const &it)
            {
                boost::filesystem::path fsLoc(m_fsPath);
                fsLoc /= it.filename();
                boost::filesystem::path teaLoc(m_teaPath);
                teaLoc /= it.filename();

                std::cout<<"Extracting file "<<fsLoc<<"..."<<std::endl;
                teasafe::TeaSafeFileDevice device = m_theBfs.openFile(teaLoc.string(), teasafe::OpenDisposition::buildReadOnlyDisposition());
                device.seek(0, std::ios_base::beg);
                std::ofstream out(fsLoc.string().c_str(), std::ios_base::binary);
                boost::iostreams::copy(device, out);
            }

            virtual void exitFolder(EntryInfo const &)
            {
                boost::filesystem::path fsLoc(m_fsPath);
                fsLoc = fsLoc.parent_path();
                boost::filesystem::path teaLoc(m_teaPath);
                teaLoc = teaLoc.parent_path();
                m_fsPath = fsLoc.string();
                m_teaPath = teaLoc.string();
            }

          private:
            FolderExtractionVisitor(); // not required
            TeaSafe &m_theBfs;
            std::string m_teaPath;
            std::string m_fsPath;
        };

    }

}

#endif // TeaSafe_UTILITY_FOLDER_EXTRACTION_VISITOR_HPP__
