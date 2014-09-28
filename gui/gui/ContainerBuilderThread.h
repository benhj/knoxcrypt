#ifndef CONTAINERBUILDERTHREAD_H
#define CONTAINERBUILDERTHREAD_H

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/TeaSafe.hpp"
#include "utility/EventType.hpp"
#include "utility/MakeTeaSafe.hpp"

#include <QThread>
#include <memory>
#include <mutex>

namespace teasafe
{
    class TeaSafe;
}

typedef std::shared_ptr<teasafe::TeaSafe> SharedTeaSafe;

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
  void blockWrittenSignal(long);
  void setProgressLabelSignal(QString);
  void closeProgressSignal();

private:
  teasafe::SharedCoreIO m_io;
  SharedTeaSafe m_teaSafe;
  typedef std::mutex TeaMutex;
  typedef std::lock_guard<TeaMutex> TeaLock;
  mutable TeaMutex m_teaMutex;
  std::shared_ptr<teasafe::MakeTeaSafe> m_imageBuilder;

  void buildTSImage();

  void imagerCallback(teasafe::EventType eventType, long const amount);

};

#endif // CONTAINERBUILDERTHREAD_H
