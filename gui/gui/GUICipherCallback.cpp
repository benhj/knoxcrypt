#include "GUICipherCallback.h"

GUICipherCallback::GUICipherCallback(QObject *parent) :
    QObject(parent)
{
}

void GUICipherCallback::cipherCallback(teasafe::EventType eventType, long const amount)
{
    static long value(0);
    if (eventType == teasafe::EventType::BigCipherBuildBegin) {
        emit openProgressSignal();
        emit setMaximumProgressSignal(amount);
        emit setProgressLabelSignal("Building cipher...");
    }
    if (eventType == teasafe::EventType::CipherBuildUpdate) {
        emit updateProgressSignal(value++);
    }
    if (eventType == teasafe::EventType::BigCipherBuildEnd) {
        value = 0;
        emit closeProgressSignal();
    }
    if (eventType == teasafe::EventType::KeyGenEnd) {
        value = 0;
        emit closeProgressSignal();
    }
}
