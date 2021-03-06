#include "client.h"

#include <QMetaObject>
#include <QMetaMethod>

QT_BEGIN_NAMESPACE

Client::Client(QObject *parent) : NoronPeer(parent)
{

}

Client::Client(NoronHub *hub, QObject *parent) : NoronPeer(parent)
{
    if(hub)
        setHub(hub);
}

int Client::getRandomNumber(const QObject *obj, const QMetaMethod *callbackMethod)
{
    qlonglong id = invokeOnPeer("getRandomNumberSlot");
    
    if(id){
        NoronRemoteCall<int> *call = new NoronRemoteCall<int>(const_cast<QObject *>(obj), const_cast<QMetaMethod *>(callbackMethod));
        addCall(id, call);
    }
    
}

#if __cplusplus >= 201103L
int Client::getRandomNumber(std::function<void(int)> callbackFunction)
{
    qlonglong id = invokeOnPeer("getRandomNumberSlot");
    
    if(id){
        NoronRemoteCall<int> *call = new NoronRemoteCall<int>(callbackFunction);
        addCall(id, call);
    }
    
}
#endif

int Client::getRandomNumber(const QObject *obj, char *callbackSlot)
{
    qlonglong id = invokeOnPeer("getRandomNumberSlot");
    
    if(id){
        NoronRemoteCall<int> *call = new NoronRemoteCall<int>(const_cast<QObject *>(obj), callbackSlot);
        addCall(id, call);
    }
    
}

int Client::getRandomNumber()
{
    qlonglong id = invokeOnPeer("getRandomNumberSlot");
    
    if(id){
        NoronRemoteCallBase *call = new NoronRemoteCallBase(NoronRemoteCallBase::EventLoop);
        addCall(id, call);
        call->eventLoop->exec();
        int ret = call->returnData.value<int>();
        removeCall(id);
        delete call;
        return ret;
    }
    
}

#ifdef QT_QML_LIB
int Client::getRandomNumber(QJSValue callbackFunction)
{
    qlonglong id = invokeOnPeer("getRandomNumberSlot");
    
    if(id){
        NoronRemoteCall<int> *call = new NoronRemoteCall<int>(callbackFunction);
        addCall(id, call);
    }
    
}
#endif

int Client::getRandomNumberAsync()
{
    qlonglong id = invokeOnPeer("getRandomNumberSlot");
    if(id){
        NoronRemoteCallBase *call = new NoronRemoteCallBase(NoronRemoteCallBase::EventLoop);
        addCall(id, call);
    }
    
}

int Client::getRandomNumberSlot()
{
    int ret;
    emit getRandomNumberSignal(&ret);
    return ret;
    
}

int Client::getRandomNumberWithTimeout(int timeout, const QObject *obj, char *callbackSlot)
{
    qlonglong id = invokeOnPeer("getRandomNumberWithTimeoutSlot", timeout);
    
    if(id){
        NoronRemoteCall<int> *call = new NoronRemoteCall<int>(const_cast<QObject *>(obj), callbackSlot);
        addCall(id, call);
    }
    
}

int Client::getRandomNumberWithTimeout(int timeout)
{
    qlonglong id = invokeOnPeer("getRandomNumberWithTimeoutSlot", timeout);
    
    if(id){
        NoronRemoteCallBase *call = new NoronRemoteCallBase(NoronRemoteCallBase::EventLoop);
        addCall(id, call);
        call->eventLoop->exec();
        int ret = call->returnData.value<int>();
        removeCall(id);
        delete call;
        return ret;
    }
    
}

int Client::getRandomNumberWithTimeout(int timeout, const QObject *obj, const QMetaMethod *callbackMethod)
{
    qlonglong id = invokeOnPeer("getRandomNumberWithTimeoutSlot", timeout);
    
    if(id){
        NoronRemoteCall<int> *call = new NoronRemoteCall<int>(const_cast<QObject *>(obj), const_cast<QMetaMethod *>(callbackMethod));
        addCall(id, call);
    }
    
}

#if __cplusplus >= 201103L
int Client::getRandomNumberWithTimeout(int timeout, std::function<void(int)> callbackFunction)
{
    qlonglong id = invokeOnPeer("getRandomNumberWithTimeoutSlot", timeout);
    
    if(id){
        NoronRemoteCall<int> *call = new NoronRemoteCall<int>(callbackFunction);
        addCall(id, call);
    }
    
}
#endif

#ifdef QT_QML_LIB
int Client::getRandomNumberWithTimeout(int timeout, QJSValue callbackFunction)
{
    qlonglong id = invokeOnPeer("getRandomNumberWithTimeoutSlot", timeout);
    
    if(id){
        NoronRemoteCall<int> *call = new NoronRemoteCall<int>(callbackFunction);
        addCall(id, call);
    }
    
}
#endif

int Client::getRandomNumberWithTimeoutAsync(int timeout)
{
    qlonglong id = invokeOnPeer("getRandomNumberWithTimeoutSlot", timeout);
    if(id){
        NoronRemoteCallBase *call = new NoronRemoteCallBase(NoronRemoteCallBase::EventLoop);
        addCall(id, call);
    }
    
}

int Client::getRandomNumberWithTimeoutSlot(int timeout)
{
    int ret;
    emit getRandomNumberWithTimeoutSignal(timeout, &ret);
    return ret;
    
}


QT_END_NAMESPACE
