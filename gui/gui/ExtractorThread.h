#ifndef EXTRACTORTHREAD_H
#define EXTRACTORTHREAD_H

#include "teasafe/TeaSafe.hpp"

#include <QThread>

class ExtractorThread : public QThread
{
    Q_OBJECT
public:
    ExtractorThread(teasafe::SharedTeaSafe const &teaSafe,
                    std::string const &teaPath,
                    std::string const &fsPath,
                    QObject *parent = 0);

signals:
    void startedSignal();
    void finishedSignal();

public slots:

protected:
    void run();

private:
    teasafe::SharedTeaSafe m_teaSafe;
    std::string m_teaPath;
    std::string m_fsPath;

};

#endif // EXTRACTORTHREAD_H
