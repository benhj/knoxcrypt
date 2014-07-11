#ifndef GUICIPHERCALLBACK_H
#define GUICIPHERCALLBACK_H

#include "utility/EventType.hpp"

#include <QObject>

class GUICipherCallback : public QObject
{
    Q_OBJECT
public:
    explicit GUICipherCallback(QObject *parent = 0);
    void cipherCallback(teasafe::EventType eventType, long const amount);

signals:

    void setMaximumProgressSignal(long);
    void setProgressLabelSignal(QString);
    void updateProgressSignal(long);
    void closeProgressSignal();

public slots:

};

#endif // GUICIPHERCALLBACK_H
