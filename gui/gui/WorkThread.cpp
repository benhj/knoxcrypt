#include "WorkThread.h"
#include "utility/ExtractToPhysical.hpp"

WorkThread::WorkThread(QObject *parent)
    : QThread(parent)
{
}

void WorkThread::addWorkFunction(WorkFunction const &wf)
{
    m_workQueue.push(wf);
}

void WorkThread::run()
{
    while(true) {
        WorkFunction f;
        m_workQueue.wait_and_pop(f);
        emit startedSignal();
        f();
        emit finishedSignal();
    }
}
