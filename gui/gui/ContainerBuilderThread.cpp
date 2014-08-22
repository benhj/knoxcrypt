#include "ContainerBuilderThread.h"

#include <boost/make_shared.hpp>

#include <QDebug>

ContainerBuilderThread::ContainerBuilderThread(QObject *parent)
    : QThread(parent)
    , m_imageBuilder()
{
}

void ContainerBuilderThread::setSharedIO(teasafe::SharedCoreIO const &io)
{
    TeaLock lock(m_teaMutex);
    m_io = io;
    bool const sparseImage = true; // TODO: option to change this
    m_imageBuilder = boost::make_shared<teasafe::MakeTeaSafe>(m_io, sparseImage);

    // register progress callback for imager
    boost::function<void(teasafe::EventType)> fb(boost::bind(&ContainerBuilderThread::imagerCallback, this, _1, m_io->blocks));
    m_imageBuilder->registerSignalHandler(fb);

}

SharedTeaSafe ContainerBuilderThread::getTeaSafe()
{
    TeaLock lock(m_teaMutex);
    return boost::make_shared<teasafe::TeaSafe>(m_io);
}

void ContainerBuilderThread::run()
{
    this->buildTSImage();
}

void ContainerBuilderThread::buildTSImage()
{
    TeaLock lock(m_teaMutex);
    m_imageBuilder->buildImage();
}

void ContainerBuilderThread::imagerCallback(teasafe::EventType eventType, long const amount)
{
    static long val(0);
    if(eventType == teasafe::EventType::ImageBuildStart) {
        emit blockCountSignal(amount);
        emit setProgressLabelSignal("Building image...");
    }
    if(eventType == teasafe::EventType::ImageBuildUpdate) {
        emit blockWrittenSignal(++val);
    }
    if(eventType == teasafe::EventType::ImageBuildEnd) {
        val = 0;
        emit finishedBuildingSignal();
        emit closeProgressSignal();
    }
    if(eventType == teasafe::EventType::IVWriteEvent) {

    }
    if(eventType == teasafe::EventType::RoundsWriteEvent) {

    }
}
