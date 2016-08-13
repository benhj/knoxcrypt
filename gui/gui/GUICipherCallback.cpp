#include "GUICipherCallback.h"

GUICipherCallback::GUICipherCallback(QObject *parent) :
    QObject(parent)
{
}

void GUICipherCallback::cipherCallback(knoxcrypt::EventType eventType, long const amount)
{
    static long value(0);
    if (eventType == knoxcrypt::EventType::BigCipherBuildBegin) {
        emit openProgressSignal();
        emit setMaximumProgressSignal(amount);
        emit setProgressLabelSignal("Building cipher...");
    }
    if (eventType == knoxcrypt::EventType::CipherBuildUpdate) {
        emit updateProgressSignal(value++);
    }
    if (eventType == knoxcrypt::EventType::BigCipherBuildEnd) {
        value = 0;
        emit closeProgressSignal();
    }
    if (eventType == knoxcrypt::EventType::KeyGenEnd) {
        value = 0;
        emit closeProgressSignal();
    }
}
