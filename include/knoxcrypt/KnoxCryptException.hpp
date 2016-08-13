/*
  Copyright (c) <2013-2016>, <BenHJ>
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

#include <exception>

namespace knoxcrypt
{

    enum class KnoxCryptError { NotFound, AlreadyExists, IllegalFilename, FolderNotEmpty };

    class KnoxCryptException : public std::exception
    {
      public:
        KnoxCryptException() = delete;
        KnoxCryptException(KnoxCryptError const &error)
        : m_error(error)
        {

        }

        virtual const char* what() const throw()
        {
            if (m_error == KnoxCryptError::NotFound) {
                return "KnoxCrypt: Entry not found";
            }

            if (m_error == KnoxCryptError::AlreadyExists) {
                return "KnoxCrypt: Entry already exists";
            }

            if (m_error == KnoxCryptError::IllegalFilename) {
                return "KnoxCrypt: illegal filename";
            }

            if (m_error == KnoxCryptError::FolderNotEmpty) {
                return "KnoxCrypt: Folder not empty";
            }

            return "KnoxCrypt: Unknown error";
        }

        bool operator==(KnoxCryptException const &other) const
        {
            return m_error == other.m_error;
        }

      private:
        KnoxCryptError m_error;
    };
}
