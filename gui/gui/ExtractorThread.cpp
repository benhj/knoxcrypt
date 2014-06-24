#include "ExtractorThread.h"
#include "utility/ExtractToPhysical.hpp"

ExtractorThread::ExtractorThread(const teasafe::SharedTeaSafe &teaSafe,
                                 const std::string &teaPath,
                                 const std::string &fsPath,
                                 QObject *parent)
    : m_teaSafe(teaSafe)
    , m_teaPath(teaPath)
    , m_fsPath(fsPath)
    , QThread(parent)
{

}

void ExtractorThread::addWorkFunction(ExtractorThread::WorkFunction const &wf)
{
    m_workQueue.push(wf);
}

void ExtractorThread::run()
{
    emit startedSignal();
    teasafe::utility::ExtractToPhysical().extractToPhysical(*m_teaSafe, m_teaPath, m_fsPath);
    emit finishedSignal();
}
