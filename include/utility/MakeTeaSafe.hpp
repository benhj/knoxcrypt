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

#include "teasafe/ContainerImageStream.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/FileBlock.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/CompoundFolder.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "teasafe/detail/DetailFileBlock.hpp"
#include "utility/EventType.hpp"
#include "utility/PassHasher.hpp"

#include <boost/optional.hpp>
#include <boost/signals2.hpp>

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <memory>


namespace teasafe
{
    typedef boost::optional<uint64_t> OptionalMagicPart;

    class MakeTeaSafe
    {
      public:

        MakeTeaSafe(SharedCoreIO const &io,
                    bool const sparse = false, // sparse images not created by default
                    OptionalMagicPart const &omp = OptionalMagicPart())
            : m_io(io)
            , m_omp(omp)
            , m_sparse(sparse)
            , m_writeSignal(std::make_shared<WriteSignal>())
        {
        }

        void buildImage()
        {
            this->doBuildImage(m_io);
        }

        virtual void registerSignalHandler(std::function<void(EventType)> const &f)
        {
            m_writeSignal->connect(f);
        }

      private:
        SharedCoreIO m_io;       // io
        OptionalMagicPart m_omp; // for creating a magic partition
        bool m_sparse;           // should a sparse image be created?

        /// to notify image-writing process
        typedef boost::signals2::signal<void(EventType)> WriteSignal;
        typedef std::shared_ptr<WriteSignal> SharedWriteSignal;
        SharedWriteSignal m_writeSignal;

        MakeTeaSafe(); // not required

        void buildBlockBytes(uint64_t const fsSize, uint8_t sizeBytes[8])
        {
            detail::convertUInt64ToInt8Array(fsSize, sizeBytes);
        }

        void buildFileCountBytes(uint64_t const fileCount, uint8_t sizeBytes[8])
        {
            detail::convertUInt64ToInt8Array(fileCount, sizeBytes);
        }

        void broadcastEvent(EventType const &event)
        {
            (*m_writeSignal)(event);
        }

        void writeOutFileSpaceBytes(SharedCoreIO const &io, ContainerImageStream &out)
        {

            broadcastEvent(EventType::ImageBuildStart);
            for (uint64_t i(0); i < io->blocks ; ++i) {
                detail::writeBlock(io, out, i);
                broadcastEvent(EventType::ImageBuildUpdate);
            }
            broadcastEvent(EventType::ImageBuildEnd);
        }

        void zeroOutBits(std::vector<uint8_t> &bitMapData)
        {
            uint8_t byte(0);
            for (int i = 0; i < 8; ++i) {
                detail::setBitInByte(byte, i, false);
            }
            bitMapData.push_back(byte);
        }

        /**
         *
         * @param blocks
         */
        void createVolumeBitMap(uint64_t const blocks, ContainerImageStream &out)
        {
            //
            // each block will be represented by a bit. If allocated this
            // bit will be set to 1. If not allocated it will be set to 0
            // So we need blocks bits
            // Store the bits in uint8_t. Each of these is a byte so we need
            // blocks divided by 8 to get the number of bytes required.
            // All initialized to zero
            //
            uint64_t bytesRequired = blocks / uint64_t(8);
            std::vector<uint8_t> bitMapData;
            for (uint64_t b = 0; b < bytesRequired; ++b) {
                zeroOutBits(bitMapData);
            }
            (void)out.write((char*)&bitMapData.front(), bytesRequired);
        }

        /**
         * @brief build the file system image
         *
         * The first 8 bytes will represent the number of blocks in the FS
         * The next blocks bits will represent the volume bit map
         * The next 8 bytes will represent the total number of files
         * The next data will be metadata computed as a fraction of the fs
         * size and number of blocks
         * The remaining bytes will be reserved for actual file data 512 byte blocks
         *
         * @param imageName the name of the image
         * @param blocks the number of blocks in the file system
         */
        void doBuildImage(SharedCoreIO const &io)
        {
            //
            // write out initial IV and header.
            // Note, the header will store extra metainfo about other needed
            // stuff such as the number of rounds used by xtea and other
            // as-of-yet, undecided info
            //
            {
                broadcastEvent(EventType::IVWriteEvent);
                uint8_t ivBytes[8];
                detail::convertUInt64ToInt8Array(io->iv, ivBytes);
                std::ofstream ivout(io->path.c_str(), std::ios::out | std::ios::binary);
                (void)ivout.write((char*)ivBytes, 8);
                detail::convertUInt64ToInt8Array(io->iv2, ivBytes);
                (void)ivout.write((char*)ivBytes, 8);
                detail::convertUInt64ToInt8Array(io->iv3, ivBytes);
                (void)ivout.write((char*)ivBytes, 8);
                detail::convertUInt64ToInt8Array(io->iv4, ivBytes);
                (void)ivout.write((char*)ivBytes, 8);

                // broadcastEvent(EventType::RoundsWriteEvent);
                // for small bits of info like rounds and cipher type, only
                // need 1 byte representations since we'll never have over 255
                // ciphers (for example)
                (void)ivout.write((char*)&io->rounds, 1);
                for(int i = 0; i < 7; ++i) {
                    (void)ivout.write((char*)&io->cipher, 1);
                }

                ivout.flush();
                ivout.close();
            }

            //
            // store the number of blocks in the first 8 bytes of the superblock
            //
            uint8_t sizeBytes[8];
            buildBlockBytes(io->blocks, sizeBytes);
            // write out size, and volume bitmap bytes
            ContainerImageStream out(io, std::ios::out | std::ios::app | std::ios::binary);

            // write out an encrypted hash of the password; will be compared
            // with that entered when reading back in so we know if an
            // incorrect password has been entered
            out.seekp(detail::beginning() - detail::PASS_HASH_BYTES); // seek past iv + header bytes
            uint8_t passHash[32];
            utility::sha256(io->password, passHash);
            out.write((char*)passHash, 32);

            // now seek past iv, header and hash bytes before continuing to write
            out.seekp(detail::beginning()); 
            out.write((char*)sizeBytes, 8);
            createVolumeBitMap(io->blocks, out);

            // file count will always be 0 upon initialization
            uint64_t fileCount(0);
            uint8_t countBytes[8];
            buildFileCountBytes(fileCount, countBytes);

            // write out file count
            out.write((char*)countBytes, 8);

            // write out the file space bytes
            if(!m_sparse) {
                writeOutFileSpaceBytes(io, out);
            }

            out.flush();
            out.close();

            // create the root folder directory. Calling this constructor will
            // automatically set the initial root block and set the initial
            // number of entries to zero. Note, the initial root block will
            // always be block 0
            // added block builder here since can only work after bitmap created
            // fixes issue https://github.com/benhj/teasafe/issues/15
            io->blockBuilder = std::make_shared<teasafe::FileBlockBuilder>(io);
            CompoundFolder rootDir(io, "root");

            // create an extra 'magic partition' which is another root folder
            // offset to a differing file block
            if (m_omp) {
                SharedCoreIO magicIo(io);
                magicIo->rootBlock = *m_omp;
                bool const setRoot = true;
                CompoundFolder magicDir(magicIo, "root", setRoot);
            }

            broadcastEvent(EventType::ImageBuildEnd);
        }
    };
}
