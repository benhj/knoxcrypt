#ifndef EXTRACTORTHREAD_H
#define EXTRACTORTHREAD_H

#include "teasafe/TeaSafe.hpp"
#include "utility/ConcurrentQueue.hpp"

#include <QThread>

#include <boost/function.hpp>

class ExtractorThread : public QThread
{
    Q_OBJECT
        public:

    typedef boost::function<void()> WorkFunction;
    typedef teasafe::utility::ConcurrentQueue<WorkFunction> WorkQueue;

    explicit ExtractorThread(QObject *parent = 0);

    void addWorkFunction(WorkFunction const&);

  signals:
    void startedSignal();
    void finishedSignal();

    public slots:

    protected:
    void run();

  private:
    mutable WorkQueue m_workQueue;

};

#endif // EXTRACTORTHREAD_H
