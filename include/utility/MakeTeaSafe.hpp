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

#ifndef TeaSafe_MAKE_TeaSafe_HPP__
#define TeaSafe_MAKE_TeaSafe_HPP__

#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/FileBlock.hpp"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/TeaSafeFolder.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include "teasafe/detail/DetailFileBlock.hpp"
#include "utility/EventType.hpp"

#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signals2.hpp>

#include <string>
#include <fstream>
#include <vector>
#include <iostream>


namespace teasafe
{
    typedef boost::optional<uint64_t> OptionalMagicPart;

    class MakeTeaSafe
    {
      public:

        MakeTeaSafe(SharedCoreIO const &io, OptionalMagicPart const &omp = OptionalMagicPart())
            : m_io(io)
            , m_omp(omp)
            , m_writeSignal(boost::make_shared<WriteSignal>())
        {
        }

        void buidImage()
        {
            this->doBuildImage(m_io);
        }

        virtual void registerSignalHandler(boost::function<void(EventType)> const &f)
        {
            m_writeSignal->connect(f);
        }

      private:
        SharedCoreIO m_io;       // io
        OptionalMagicPart m_omp; // for creating a magic partition

        /// to notify image-writing process
        typedef boost::signals2::signal<void(EventType)> WriteSignal;
        typedef boost::shared_ptr<WriteSignal> SharedWriteSignal;
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

        void writeOutFileSpaceBytes(SharedCoreIO const &io, TeaSafeImageStream &out)
        {

            {
                EventType event = EventType::ImageBuildEnd;
                (*m_writeSignal)(event);
            }
            for (uint64_t i(0); i < io->blocks ; ++i) {
                std::vector<uint8_t> ints;
                ints.assign(detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META, 0);

                // write out block metadata
                uint64_t offset = detail::getOffsetOfFileBlock(i, io->blocks);
                (void)out.seekp(offset);

                // write m_bytesWritten; 0 to begin with
                uint8_t sizeDat[4];
                uint32_t size = 0;
                detail::convertInt32ToInt4Array(size, sizeDat);
                (void)out.write((char*)sizeDat, 4);

                // write m_next; begins as same as index
                uint8_t nextDat[8];
                detail::convertUInt64ToInt8Array(i, nextDat);
                (void)out.write((char*)nextDat, 8);

                // write data bytes
                (void)out.write((char*)&ints.front(), detail::FILE_BLOCK_SIZE - detail::FILE_BLOCK_META);
                {
                    EventType event = EventType::ImageBuildUpdate;
                    (*m_writeSignal)(event);
                }
            }
            {
                EventType event = EventType::ImageBuildEnd;
                (*m_writeSignal)(event);
            }
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
        void createVolumeBitMap(uint64_t const blocks, TeaSafeImageStream &out)
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
                uint8_t ivBytes[8];
                detail::convertUInt64ToInt8Array(io->iv, ivBytes);
                std::ofstream ivout(io->path.c_str(), std::ios::out | std::ios::binary);
                (void)ivout.write((char*)ivBytes, 8);

                for(int i = 0; i < 8; ++i) {
                    // note although char is smaller that io->rounds, which
                    // is an unsigned int, io->rounds should always be less than
                    // size 255 (but > 0). Perhaps a different var type therefore?
                    (void)ivout.write((char*)&io->rounds, 1);
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
            TeaSafeImageStream out(io, std::ios::out | std::ios::app | std::ios::binary);
            out.seekp(detail::beginning()); // seek past iv bytes
            out.write((char*)sizeBytes, 8);
            createVolumeBitMap(io->blocks, out);

            // file count will always be 0 upon initialization
            uint64_t fileCount(0);
            uint8_t countBytes[8];
            buildFileCountBytes(fileCount, countBytes);

            // write out file count
            out.write((char*)countBytes, 8);

            // write out the file space bytes
            writeOutFileSpaceBytes(io, out);

            out.flush();
            out.close();

            // create the root folder directory. Calling this constructor will
            // automatically set the initial root block and set the initial
            // number of entries to zero. Note, the initial root block will
            // always be block 0
            // added block builder here since can only work after bitmap created
            // fixes issue https://github.com/benhj/teasafe/issues/15
            io->blockBuilder = boost::make_shared<teasafe::FileBlockBuilder>(io);
            TeaSafeFolder rootDir(io, "root");

            // create an extra 'magic partition' which is another root folder
            // offset to a differing file block
            if (m_omp) {
                SharedCoreIO magicIo(io);
                magicIo->rootBlock = *m_omp;
                bool const setRoot = true;
                TeaSafeFolder magicDir(magicIo, "root", setRoot);
            }
        }
    };
}

#endif // TeaSafe_MAKE_TeaSafe_HPP__
