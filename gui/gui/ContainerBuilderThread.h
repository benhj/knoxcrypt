#ifndef CONTAINERBUILDERTHREAD_H
#define CONTAINERBUILDERTHREAD_H

#include "knoxcrypt/CoreknoxcryptIO.hpp"
#include "knoxcrypt/knoxcrypt.hpp"
#include "utility/EventType.hpp"
#include "utility/Makeknoxcrypt.hpp"

#include <QThread>
#include <memory>
#include <mutex>

namespace knoxcrypt
{
    class knoxcrypt;
}

typedef std::shared_ptr<knoxcrypt::knoxcrypt> Sharedknoxcrypt;

class ContainerBuilderThread : public QThread
{
    Q_OBJECT
public:
    explicit ContainerBuilderThread(QObject *parent = 0);

    void setSharedIO(knoxcrypt::SharedCoreIO const &io);

    Sharedknoxcrypt getknoxcrypt();

protected:
  void run();

signals:

  void finishedBuildingSignal();
  void blockCountSignal(long);
  void blockWrittenSignal(long);
  void setProgressLabelSignal(QString);
  void closeProgressSignal();

private:
  knoxcrypt::SharedCoreIO m_io;
  Sharedknoxcrypt m_knoxcrypt;
  typedef std::mutex TeaMutex;
  typedef std::lock_guard<TeaMutex> TeaLock;
  mutable TeaMutex m_teaMutex;
  std::shared_ptr<knoxcrypt::Makeknoxcrypt> m_imageBuilder;

  void buildTSImage();

  void imagerCallback(knoxcrypt::EventType eventType, long const amount);

};

#endif // CONTAINERBUILDERTHREAD_H
