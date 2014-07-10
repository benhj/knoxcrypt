#ifndef CONTAINERBUILDERTHREAD_H
#define CONTAINERBUILDERTHREAD_H

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/TeaSafe.hpp"
#include "utility/EventType.hpp"
#include "utility/MakeTeaSafe.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <QThread>

namespace teasafe
{
    class TeaSafe;
}

typedef boost::shared_ptr<teasafe::TeaSafe> SharedTeaSafe;

class ContainerBuilderThread : public QThread
{
    Q_OBJECT
public:
    explicit ContainerBuilderThread(QObject *parent = 0);

    void setSharedIO(teasafe::SharedCoreIO const &io);

    SharedTeaSafe getTeaSafe();

protected:
  void run();

signals:

  void finishedBuildingSignal();
  void blockCountSignal(long);
  void blockWrittenSignal();

private:
  teasafe::SharedCoreIO m_io;
  SharedTeaSafe m_teaSafe;
  typedef boost::mutex TeaMutex;
  typedef boost::lock_guard<TeaMutex> TeaLock;
  mutable TeaMutex m_teaMutex;
  boost::shared_ptr<teasafe::MakeTeaSafe> m_imageBuilder;

  void buildTSImage();

  void imagerCallback(teasafe::EventType eventType, long const amount);

};

#endif // CONTAINERBUILDERTHREAD_H
