#include "ContainerBuilderThread.h"

#include <QDebug>

#include <memory>

ContainerBuilderThread::ContainerBuilderThread(QObject *parent)
    : QThread(parent)
    , m_imageBuilder()
{
}

void ContainerBuilderThread::setSharedIO(knoxcrypt::SharedCoreIO const &io)
{
    TeaLock lock(m_teaMutex);
    m_io = io;
    bool const sparseImage = true; // TODO: option to change this
    m_imageBuilder = std::make_shared<knoxcrypt::Makeknoxcrypt>(m_io, sparseImage);

    // register progress callback for imager
    std::function<void(knoxcrypt::EventType)> fb(std::bind(&ContainerBuilderThread::imagerCallback,
                                                         this, std::placeholders::_1, m_io->blocks));
    m_imageBuilder->registerSignalHandler(fb);

}

Sharedknoxcrypt ContainerBuilderThread::getknoxcrypt()
{
    TeaLock lock(m_teaMutex);
    return std::make_shared<knoxcrypt::knoxcrypt>(m_io);
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

void ContainerBuilderThread::imagerCallback(knoxcrypt::EventType eventType, long const amount)
{
    static long val(0);
    if(eventType == knoxcrypt::EventType::ImageBuildStart) {
        emit blockCountSignal(amount);
        emit setProgressLabelSignal("Building image...");
    }
    if(eventType == knoxcrypt::EventType::ImageBuildUpdate) {
        emit blockWrittenSignal(++val);
    }
    if(eventType == knoxcrypt::EventType::ImageBuildEnd) {
        val = 0;
        emit finishedBuildingSignal();
        emit closeProgressSignal();
    }
    if(eventType == knoxcrypt::EventType::IVWriteEvent) {

    }
    if(eventType == knoxcrypt::EventType::RoundsWriteEvent) {

    }
}
