#include "ContainerBuilderThread.h"

#include <boost/make_shared.hpp>

ContainerBuilderThread::ContainerBuilderThread(QObject *parent)
    : QThread(parent)
    , m_imageBuilder()
{
}

void ContainerBuilderThread::setSharedIO(teasafe::SharedCoreIO const &io)
{
    TeaLock lock(m_teaMutex);
    m_io = io;
    m_imageBuilder = boost::make_shared<teasafe::MakeTeaSafe>(m_io);

    // register progress callback for imager
    boost::function<void(teasafe::EventType)> fb(boost::bind(&ContainerBuilderThread::imagerCallback, this, _1, m_io->blocks));
    m_imageBuilder->registerSignalHandler(fb);

}

void ContainerBuilderThread::run()
{
    this->buildTSImage();
}

void ContainerBuilderThread::buildTSImage()
{

}

void ContainerBuilderThread::imagerCallback(teasafe::EventType eventType, long const amount)
{
    if(eventType == teasafe::EventType::ImageBuildStart) {
        emit blockCountSignal(amount);
    }
    if(eventType == teasafe::EventType::ImageBuildUpdate) {
        emit blockWrittenSignal();
    }
    if(eventType == teasafe::EventType::IVWriteEvent) {

    }
    if(eventType == teasafe::EventType::RoundsWriteEvent) {

    }
}
