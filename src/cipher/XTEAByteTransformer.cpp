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

// an implementation of XTEA

#include "teasafe/detail/DetailTeaSafe.hpp"

#include "cipher/scrypt/crypto_scrypt.hpp"
#include "cipher/XTEAByteTransformer.hpp"

#include <boost/progress.hpp>

#include <vector>

namespace teasafe { namespace cipher
{

    namespace detail
    {
        // the xtea encipher algorithm as found on wikipedia. Use this to encrypt
        // a sequence of numbers. The original plain-text is then xor-ed with this
        // sequence
        void encipher(unsigned int num_rounds, uint32_t v[2], uint32_t const key[4])
        {
            unsigned int i;
            uint32_t v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
            for (i=0; i < num_rounds; i++) {
                v0 += ((v1 << 4 ^ v1 >> 5) + v1) ^(sum + key[sum & 3]);
                sum += delta;
                v1 += ((v0 << 4 ^ v0 >> 5) + v0) ^(sum + key[(sum>>11) & 3]);
            }
            v[0]=v0; v[1]=v1;
        }

        // helper code found here:
        // http://codereview.stackexchange.com/questions/2050/codereview-tiny-encryption-algorithm-for-arbitrary-sized-data
        void convertBytesAndEncipher(unsigned int num_rounds, unsigned char * buffer, uint32_t const key[4])
        {
            uint32_t datablock[2];

            datablock[0] = (buffer[0] << 24) | (buffer[1] << 16)  | (buffer[2] << 8) | (buffer[3]);
            datablock[1] = (buffer[4] << 24) | (buffer[5] << 16)  | (buffer[6] << 8) | (buffer[7]);

            encipher(num_rounds, datablock, key);

            buffer[0] = static_cast<unsigned char>((datablock[0] >> 24) & 0xFF);
            buffer[1] = static_cast<unsigned char>((datablock[0] >> 16) & 0xFF);
            buffer[2] = static_cast<unsigned char>((datablock[0] >> 8) & 0xFF);
            buffer[3] = static_cast<unsigned char>((datablock[0]) & 0xFF);
            buffer[4] = static_cast<unsigned char>((datablock[1] >> 24) & 0xFF);
            buffer[5] = static_cast<unsigned char>((datablock[1] >> 16) & 0xFF);
            buffer[6] = static_cast<unsigned char>((datablock[1] >> 8) & 0xFF);
            buffer[7] = static_cast<unsigned char>((datablock[1]) & 0xFF);
        }
    }


    // prob a bad idea all these static vars; still for the sake of optimizing.. :-)
    static bool g_init = false;

    // used for holding the hash-generated key
    static uint32_t g_key[4];

    // used to optimize crypto process. This will store
    // the first 256MB of cipher stream
    static std::vector<char> g_bigCipherBuffer;

    // the size of the cipher buffer (prefer a #define rather than a const
    // for minimal memory footprint and minimal time required to instantiate).
#define CIPHER_BUFFER_SIZE 270000000


    XTEAByteTransformer::XTEAByteTransformer(std::string const &password,
                                             uint64_t const iv,
                                             unsigned int const rounds)
        : IByteTransformer()
        , m_iv(iv)
        , m_rounds(rounds) // note, the suggested xtea rounds is 64 in the literature
    {
        //
        // The following key generation algorithm uses scrypt, with N = 2^20; r = 8; p = 1
        //
        if (!g_init) {
            unsigned char temp[16];

            std::cout<<"Generating key...\n"<<std::endl;
            uint8_t salt[8];
            teasafe::detail::convertUInt64ToInt8Array(iv, salt);
            ::crypto_scrypt((uint8_t*)password.c_str(), password.size(), salt, 8,
                            1048576, 8, 1, temp, 16);
            std::cout<<"Key generated.\n"<<std::endl;

            int c = 0;

            for (int i = 0; i < 16; i += 4) {
                unsigned char buf[4];
                buf[0] = temp[i];
                buf[1] = temp[i + 1];
                buf[2] = temp[i + 2];
                buf[3] = temp[i + 3];
                g_key[c] = (buf[0] << 24) | (buf[1] << 16)  | (buf[2] << 8) | (buf[3]);
                ++c;
            }
            buildBigCipherBuffer();
            g_init = true;
        }
    }

    XTEAByteTransformer::~XTEAByteTransformer()
    {
    }

    void
    XTEAByteTransformer::buildBigCipherBuffer()
    {
        std::cout<<"Building big xtea cipher stream buffer. Please wait..."<<std::endl;
        std::vector<char> in;
        in.resize(CIPHER_BUFFER_SIZE);
        g_bigCipherBuffer.resize(CIPHER_BUFFER_SIZE);
        uint64_t div = CIPHER_BUFFER_SIZE / 100000;
        boost::progress_display pd(div);
        for(uint64_t i = 0;i<div;++i) {
            doTransform((&in.front()) + (i * 100000), (&g_bigCipherBuffer.front()) + (i*100000), 0, 100000);
            ++pd;
        }
        std::cout<<"\nBuilt big xtea cipher stream buffer.\n"<<std::endl;
    }

    void
    XTEAByteTransformer::doTransform(char *in, char *out, std::ios_base::streamoff startPosition, long length) const
    {
        // big cipher buffer has been initialized
        if (g_init) {
            // prefer to use cipher buffer
            if ((startPosition + length) < CIPHER_BUFFER_SIZE) {
                for (long j = 0; j < length; ++j) {
                    out[j] = in[j] ^ g_bigCipherBuffer[j + startPosition];
                }
                return;
            }
        }

        // how many blocks required? defaults to 1, if length greater
        // than 8 bytes then more blocks are needed
        long blocksRequired = 0;

        std::ios_base::streamoff const startPositionOffset = startPosition % 8;

        // the counter used to determine where we start in the cipher stream
        uint64_t startRD = (startPosition - startPositionOffset);
        uint64_t ctr = (startRD / 8) + m_iv;

        // encipher the initial 64 bit counter.
        UIntVector buf;
        buf.resize(8);
        cypherBytes(ctr, buf);

        // c is a counter used to store which byte is currently being encrypted
        long c = 0;

        // if the length is > 8, we need to encrypt in multiples of 8
        if (length > 8) {

            // compute how many blocks of size 8 bytes will be encrypted
            long remainder = length % 8;
            long roundedDown = length - remainder;
            blocksRequired += (roundedDown / 8);

            // encrypt blocksRequired times 8 byte blocks of data
            doSubTransformations(in,                         // input buffer
                                 out,                        // output buffer
                                 startPosition,              // seeked-to position
                                 startPositionOffset,        // 8-byte block offset
                                 blocksRequired,             // number of iterations
                                 c,                          // stream position
                                 ctr,                        // the CTR counter
                                 buf);                       // the cipher text buffer

            // encrypt the final block which is of length remainder bytes
            if (remainder > 0) {
                doSubTransformations(in, out, startPosition, startPositionOffset, 1, c, ctr, buf, remainder);
            }

            return;
        }

        // else the length < 8 so just encrypt length bytes
        doSubTransformations(in, out, startPosition, startPositionOffset, 1, c, ctr, buf, length);

    }

    void
    XTEAByteTransformer::doXOR(char *in,
                               char *out,
                               std::ios_base::streamoff const startPositionOffset,
                               long &c,
                               uint64_t &ctr,
                               int const bytesToEncrypt,
                               UIntVector &buf) const
    {
        // now xor plain with key stream
        int k = 0;
        for (int j = startPositionOffset; j < bytesToEncrypt + startPositionOffset; ++j) {
            if (j >= 8) {

                // iterate cipher counter and update cipher buffer
                if(j == 8) { cypherBytes(ctr, buf); }

                out[c] = in[c] ^ buf[k];
                ++k;

            } else { out[c] = in[c] ^ buf[j]; }
            ++c;
        }
    }

    void
    XTEAByteTransformer::cypherBytes(uint64_t &ctr, UIntVector &buf) const
    {
        teasafe::detail::convertUInt64ToInt8Array(ctr, &buf.front());
        detail::convertBytesAndEncipher(m_rounds, &buf.front(), g_key);
        ++ctr;
    }

    void
    XTEAByteTransformer::doSubTransformations(char *in,
                                              char *out,
                                              std::ios_base::streamoff const startPosition,
                                              std::ios_base::streamoff const startPositionOffset,
                                              long const blocksOfSize8BeingTransformed,
                                              long &c,
                                              uint64_t &ctr,
                                              UIntVector &buf,
                                              int const bytesToEncrypt) const
    {
        // transform  blocks
        for (long i = 0; i < blocksOfSize8BeingTransformed; ++i) {

            // now XOR plain with key stream.
            this->doXOR(in, out, startPositionOffset, c, ctr, bytesToEncrypt, buf);

            // cipher buffer needs updating if the start position is 0 in which
            // case it won't have been updated by the XOR loop
            if(startPositionOffset == 0) { cypherBytes(ctr, buf); }
        }
    }
}
}
