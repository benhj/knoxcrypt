 #ifndef LOADERTHREAD_H
#define LOADERTHREAD_H

#include "teasafe/CoreTeaSafeIO.hpp"
#include "teasafe/TeaSafe.hpp"
#include "utility/EventType.hpp"

#include <boost/shared_ptr.hpp>

#include <QThread>

namespace teasafe {
    class TeaSafe;
}

typedef boost::shared_ptr<teasafe::TeaSafe> SharedTeaSafe;

class LoaderThread : public QThread
{
    Q_OBJECT
public:
    explicit LoaderThread(QObject *parent = 0);

    void setSharedIO(teasafe::SharedCoreIO const &io);

    SharedTeaSafe getTeaSafe();

protected:
    void run();

signals:

    void finishedLoadingSignal();

private:
    teasafe::SharedCoreIO m_io;
    SharedTeaSafe m_teaSafe;

    void loadTSImage();

};

#endif // LOADERTHREAD_H
