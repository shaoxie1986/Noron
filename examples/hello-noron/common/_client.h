#ifndef CLIENT_H
#define CLIENT_H

#include <QtCore/QObject>
#include <QtCore/qglobal.h>

#if __cplusplus >= 201103L
#   include <functional>
#endif

#ifdef QT_QML_LIB
#   include <QJSValue>
#endif

#include <NoronAbstractHub>
#include <NoronPeer>


QT_BEGIN_NAMESPACE

class Client : public NoronPeer
{
    Q_OBJECT
    
    

public:
    Q_INVOKABLE Client(QObject *parent = 0);
    Client(NoronAbstractHub *hub, QObject *parent = 0);

public slots:
    int getRandomNumber(const QObject *obj, const QMetaMethod *callbackMethod);
    #if __cplusplus >= 201103L
    int getRandomNumber(std::function<void(int)> callbackFunction);
    #endif
    int getRandomNumber(const QObject *obj, char *callbackSlot);
    int getRandomNumber();
    #ifdef QT_QML_LIB
    int getRandomNumber(QJSValue callbackFunction);
    #endif
    int getRandomNumberAsync();
    int getRandomNumberSlot();
    int getRandomNumberWithTimeout(int timeout, const QObject *obj, char *callbackSlot);
    int getRandomNumberWithTimeout(int timeout);
    int getRandomNumberWithTimeout(int timeout, const QObject *obj, const QMetaMethod *callbackMethod);
    #if __cplusplus >= 201103L
    int getRandomNumberWithTimeout(int timeout, std::function<void(int)> callbackFunction);
    #endif
    #ifdef QT_QML_LIB
    int getRandomNumberWithTimeout(int timeout, QJSValue callbackFunction);
    #endif
    int getRandomNumberWithTimeoutAsync(int timeout);
    int getRandomNumberWithTimeoutSlot(int timeout);

signals:
    int getRandomNumberSignal(, int returnValue);
    int getRandomNumberWithTimeoutSignal(int timeout, , int returnValue);

};

QT_END_NAMESPACE

#endif // CLIENT_H
