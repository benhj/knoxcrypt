#include "ExtractorThread.h"
#include "utility/ExtractToPhysical.hpp"

ExtractorThread::ExtractorThread(QObject *parent)
    : QThread(parent)
{

}

void ExtractorThread::addWorkFunction(ExtractorThread::WorkFunction const &wf)
{
    m_workQueue.push(wf);
}

void ExtractorThread::run()
{
    while(true) {
        WorkFunction f;
        m_workQueue.wait_and_pop(f);
        emit startedSignal();
        f();
        emit finishedSignal();
    }
}
