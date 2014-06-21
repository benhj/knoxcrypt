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
