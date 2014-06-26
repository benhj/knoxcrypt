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



#include "LoaderThread.h"
#include "teasafe/FileBlockBuilder.hpp"
#include "teasafe/TeaSafeImageStream.hpp"
#include "teasafe/detail/DetailTeaSafe.hpp"
#include <boost/make_shared.hpp>

LoaderThread::LoaderThread(QObject *parent) :
    QThread(parent),
    m_io()
{

}

void LoaderThread::setSharedIO(teasafe::SharedCoreIO const &io)
{
    m_io = io;
}

SharedTeaSafe LoaderThread::getTeaSafe()
{
    return m_teaSafe;
}

void LoaderThread::loadTSImage()
{
    // Obtain the initialization vector from the first 8 bytes
    // and the number of xtea rounds from the ninth byte
    teasafe::detail::readImageIVAndRounds(m_io);

    // Obtain the number of blocks in the image by reading the image's block count
    teasafe::TeaSafeImageStream stream(m_io, std::ios::in | std::ios::binary);
    m_io->blocks = teasafe::detail::getBlockCount(stream);
    m_io->freeBlocks = m_io->blocks - teasafe::detail::getNumberOfAllocatedBlocks(stream);
    m_io->blockBuilder = boost::make_shared<teasafe::FileBlockBuilder>(m_io);
    stream.close();

    // Create the basic file system
    m_teaSafe = boost::make_shared<teasafe::TeaSafe>(m_io);

    emit finishedLoadingSignal();
}

void LoaderThread::run()
{
    this->loadTSImage();
}
