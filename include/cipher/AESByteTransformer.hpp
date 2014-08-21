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

// uses the crypto++ libraries for AES encryption

#ifndef TeaSafe_CIPHER_AES_BYTE_TRANSFORMER_HPP__
#define TeaSafe_CIPHER_AES_BYTE_TRANSFORMER_HPP__

#include "cipher/IByteTransformer.hpp"
#include "utility/EventType.hpp"
#include <iostream>
#include <string>

namespace teasafe { namespace cipher
{

    class AESByteTransformer : public IByteTransformer
    {
      public:
        AESByteTransformer(std::string const &password,
                            uint64_t const iv);

        void init();

        ~AESByteTransformer();

      private:

        mutable std::string m_password;
        mutable uint64_t m_iv;

        AESByteTransformer(); // not required

        void doEncrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const;
        void doDecrypt(char *in, char *out, std::ios_base::streamoff startPosition, long length) const;
    };
}
}


#endif // TeaSafe_CIPHER_XTEA_BYTE_TRANSFORMER_HPP__
