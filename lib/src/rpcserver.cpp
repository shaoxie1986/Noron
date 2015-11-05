#include "rpcpeer.h"
#include <QTcpServer>
#include <QSet>
#include <QThread>

#include "rpctcpsocketserver_p.h"
#include "rpcserver.h"
#include "rpcjsondataserializer.h"

RpcServer::RpcServer(qint16 port, QObject *parent) : RpcHubBase(parent), m_isMultiThread(false)
{
    serverSocket = new RpcTcpSocketServer(this);

    serverSocket->listen(QHostAddress::Any, port);
    serverSocket->setObjectName("serverSocket");

    setSerializer(new RpcJsonDataSerializer(this));

    connect(serverSocket, &RpcTcpSocketServer::newIncomingConnection, this, &RpcServer::server_newIncomingConnection);
}

QSet<RpcPeer *> RpcServer::peers()
{
    return _peers;
}

int RpcServer::typeId() const
{
    return m_typeId;
}

bool RpcServer::isMultiThread() const
{
    return m_isMultiThread;
}

void RpcServer::server_newIncomingConnection(qintptr socketDescriptor)
{
    const QMetaObject *metaObject = QMetaType::metaObjectForType(m_typeId);
    QObject *o = metaObject->newInstance();
    RpcPeer *peer = qobject_cast<RpcPeer*>(o);

    if(!peer){
        qWarning("PEER IS INCORRECT!!!");
        return;
    }
    peer->setHub(new RpcHub(serializer(), this));
    if (!peer->hub()->setSocketDescriptor(socketDescriptor)) {
        delete peer;
        return;
    }

    peer->hub()->setValidateToken(validateToken());
    peer->hub()->addSharedObject(peer);
    foreach (RpcPeer *o, _classes.values())
        peer->hub()->addSharedObject(o);

    QThread *thread;

    if(isMultiThread()){
        thread = new QThread(this);
        peer->moveToThread(thread);
        thread->start();

        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    }

    connect(peer->hub(), &RpcHubBase::disconnected, //this, &RpcServer::peer_disconnected);
            [=] () {
        if(isMultiThread())
            thread->exit();
        emit peerDisconnected(peer);
        peer->deleteLater();
    } );

    _peers.insert(peer);
    emit peerConnected(peer);
}

void RpcServer::setTypeId(int typeId)
{
    if (m_typeId == typeId)
        return;

    m_typeId = typeId;
    emit typeIdChanged(typeId);
}

void RpcServer::setIsMultiThread(bool isMultiThread)
{
    if (m_isMultiThread == isMultiThread)
        return;

    m_isMultiThread = isMultiThread;
    emit isMultiThreadChanged(isMultiThread);
}

void RpcServer::peer_disconnected()
{
    RpcPeer *peer = qobject_cast<RpcPeer*>(sender());

    _peers.remove(peer);
    peer->deleteLater();
}

qlonglong RpcServer::invokeOnPeer(QString sender, QString methodName,
                                  QVariant val0, QVariant val1,
                                  QVariant val2, QVariant val3,
                                  QVariant val4, QVariant val5,
                                  QVariant val6, QVariant val7,
                                  QVariant val8, QVariant val9)
{
    foreach(RpcPeer *peer, _peers)
        peer->hub()->invokeOnPeer(sender, methodName,
                                  val0, val1, val2, val3, val4,
                                  val5, val6, val7, val8, val9);

    return 0;
}
