#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaMethod>
#include <QCryptographicHash>

#include "rpchub.h"
#include "rpcpeer.h"
#include "rpcjsondataserializer.h"

RpcHub::RpcHub(QObject *parent) : RpcHubBase(parent),
    _isTransaction(false), requestId(0)
{
    _socket = new QTcpSocket(this);

    connect(_socket, &QIODevice::readyRead, this, &RpcHub::socket_onReadyRead);
    connect(_socket, &QTcpSocket::disconnected, this, &RpcHub::socket_disconnected);
    connect(_socket, &QTcpSocket::connected, this, &RpcHub::socket_connected);

    setSerializer(new RpcJsonDataSerializer(this));
}

RpcHub::RpcHub(RpcSerializerBase *serializer, QObject *parent) : RpcHubBase(parent),
    _isTransaction(false), requestId(0)
{
    _socket = new QTcpSocket(this);

    connect(_socket, &QIODevice::readyRead, this, &RpcHub::socket_onReadyRead);
    connect(_socket, &QTcpSocket::disconnected, this, &RpcHub::socket_disconnected);
    connect(_socket, &QTcpSocket::connected, this, &RpcHub::socket_connected);

    setSerializer(serializer);
}

void RpcHub::connectToServer(QString address, qint16 port)
{
    if(!address.isNull())
        setServerAddress(address);

    if(port)
        setPort(port);

    _socket->connectToHost(this->serverAddress(), this->port());
}

void RpcHub::disconnectFromServer()
{
    setAutoReconnect(false);
    _socket->disconnectFromHost();
}

bool RpcHub::setSocketDescriptor(qintptr socketDescriptor)
{
    return _socket->setSocketDescriptor(socketDescriptor);
}

void RpcHub::timerEvent(QTimerEvent *)
{
    if(_socket->state() == QAbstractSocket::UnconnectedState){
        connectToServer();
    }else if(_socket->state() == QAbstractSocket::ConnectedState){
        killTimer(reconnectTimerId);

        sync();
    }
}

void RpcHub::sync()
{
    beginTransaction();
    foreach (QObject *o, _classes) {

        int pcount = o->metaObject()->propertyCount();
        for(int i = 0; i < pcount; i++){
            QMetaProperty p = o->metaObject()->property(i);

            if(!p.isUser())
                continue;

            QString w = p.name();
            w[0] = w[0].toUpper();
            invokeOnPeer(o->metaObject()->className(),
                         "set" + w,
                         p.read(o));
        }
    }
    commit();
}

void RpcHub::beginTransaction()
{
    _isTransaction = true;
}

bool RpcHub::isTransaction() const
{
    return _isTransaction;
}

void RpcHub::rollback()
{
    _buffer.clear();
    _isTransaction = false;
}

void RpcHub::commit()
{
    _socket->write(QJsonDocument::fromVariant(_buffer).toJson());
    _socket->flush();
    _isTransaction = false;
    _buffer.clear();
}


void RpcHub::addToMap(QVariantMap *map, QVariant var, int index)
{
    QString i = QString::number(index);

    if(var != QVariant()){
        map->insert("val" + i, var);
        //        map->insert("type" + i, var.typeName());
    }
}

void RpcHub::procMap(QVariantMap map)
{
    bool ok;
    qlonglong id = map[ID].toLongLong(&ok);

    if(!ok){
        qWarning("An injection detected. The id '" + map[ID].toString().toLatin1() + "' is not numeric");
        return;
    }

    if(map[MAP_TYPE] == MAP_TYPE_RESPONSE){
        if(_calls[id]){
            _calls[id]->returnData = map[MAP_RETURN_VALUE];
            _calls[id]->returnToCaller();
        }
        return;
    }

    if(!validateToken().isNull())
        if(!checkValidateToken(&map)){
            qWarning("Map token validate is invalid!");
            return;
        }

    QObject *target = _classes[map[CLASS_NAME].toString()];

    if(!target){
        qWarning("There are no '" + map[CLASS_NAME].toString().toLatin1() + "' service");
        return;
    }

    QMetaMethod method;
    QGenericReturnArgument returnArgument;

    QByteArray methodName = map.value(METHOD_NAME).toByteArray();
    if(methodName == "")
        return;

    QVariant returnData = QVariant("");

    // find method
    for(int i = 0; i < target->metaObject()->methodCount(); i++){
        method = target->metaObject()->method(i);
        if(method.name() == methodName){
            const char* type = QVariant::typeToName(method.returnType());
            QVariant::Type returnType = QVariant::nameToType(type);
            returnData.convert(returnType);
            returnArgument = QGenericReturnArgument(type, &returnData);

            if(QString(type) == "void")
                returnData = QVariant(QVariant::Invalid);

            break;
        }
    }

    if(method.name().isEmpty())
        return;

    QList<QGenericArgument> args;
    for(int i = 0; i < 10; i++){
        QString indexString = QString::number(i);
        if(!map.contains("val" + indexString))
            continue;

        const void *data = map["val" + indexString].data();
        const char *name = map["val" + indexString].typeName();
        args << QGenericArgument(name, data);
    }

    QString lockName = map[CLASS_NAME].toString() + "::" + methodName;

    _locks.insert(lockName);

    if(returnData.type() == QVariant::Invalid)
        ok = QMetaObject::invokeMethod(
                    target,
                    methodName.constData(),
                    Qt::DirectConnection,
                    args.value(0, QGenericArgument() ),
                    args.value(1, QGenericArgument() ),
                    args.value(2, QGenericArgument() ),
                    args.value(3, QGenericArgument() ),
                    args.value(4, QGenericArgument() ),
                    args.value(5, QGenericArgument() ),
                    args.value(6, QGenericArgument() ),
                    args.value(7, QGenericArgument() ),
                    args.value(8, QGenericArgument() ),
                    args.value(9, QGenericArgument() ));
    else
        ok = QMetaObject::invokeMethod(
                    target,
                    methodName.constData(),
                    Qt::DirectConnection,
                    returnArgument,
                    args.value(0, QGenericArgument() ),
                    args.value(1, QGenericArgument() ),
                    args.value(2, QGenericArgument() ),
                    args.value(3, QGenericArgument() ),
                    args.value(4, QGenericArgument() ),
                    args.value(5, QGenericArgument() ),
                    args.value(6, QGenericArgument() ),
                    args.value(7, QGenericArgument() ),
                    args.value(8, QGenericArgument() ),
                    args.value(9, QGenericArgument() ));

    if(!ok)
        qWarning("Invoke " + method.name() + " on " + map[CLASS_NAME].toString().toLatin1() + " faild");
    else
        response(id, map[CLASS_NAME].toString(),
                 returnData.type() == QVariant::Invalid ? QVariant() : returnData);

    _locks.remove(lockName);
}

bool RpcHub::response(qlonglong id, QString senderName, QVariant returnValue)
{
    QVariantMap map;
    map[MAP_TYPE] = MAP_TYPE_RESPONSE;
    map[ID] = QVariant(id);
    map[CLASS_NAME] = senderName;

    if(returnValue != QVariant())
        map[MAP_RETURN_VALUE] = returnValue;

    int res = _socket->write(serializer()->serialize(map));

    return 0 != res;
}

void RpcHub::addValidateToken(QVariantMap *map)
{
    QByteArray s;

    map->insert(MAP_TOKEN_ITEM, QVariant(""));

    QMapIterator<QString, QVariant> i(*map);
    while (i.hasNext()) {
        i.next();
        s.append(i.key() + ": " + i.value().toString() + "*");
    }

    map->insert(MAP_TOKEN_ITEM, QVariant(MD5(s + validateToken())));
}

bool RpcHub::checkValidateToken(QVariantMap *map)
{
    QString token = map->value(MAP_TOKEN_ITEM).toString();
    map->insert(MAP_TOKEN_ITEM, QVariant(""));

    QByteArray s;

    QMapIterator<QString, QVariant> i(*map);
    while (i.hasNext()) {
        i.next();
        s.append(i.key() + ": " + i.value().toString() + "*");
    }

    return token == MD5(s + validateToken());
}

QString RpcHub::MD5(QString text)
{
    return MD5(text.toLocal8Bit());
}

QString RpcHub::MD5(QByteArray text)
{
    return QString(QCryptographicHash::hash(text, QCryptographicHash::Md5).toHex());
}

void RpcHub::socket_connected()
{
    setIsConnected(true);
}

void RpcHub::socket_disconnected()
{
    setIsConnected(false);

    if(autoReconnect()){
        connectToServer();
        reconnectTimerId = startTimer(500);
    }
}

void RpcHub::socket_onReadyRead()
{
    QByteArray buffer = _socket->readAll();
    //multi chunck support
    buffer = "[" + buffer.replace("}\n{", "},{") + "]";

    QVariant var = serializer()->deserialize(buffer);

    if(var.type() == QVariant::Map)
        procMap(var.toMap());

    if(var.type() == QVariant::List){
        QVariantList list = var.toList();
        foreach (QVariant map, list){
            procMap(map.toMap());
        }
    }
}

qlonglong RpcHub::invokeOnPeer(QString sender, QString methodName,
                               QVariant val0, QVariant val1,
                               QVariant val2, QVariant val3,
                               QVariant val4, QVariant val5,
                               QVariant val6, QVariant val7,
                               QVariant val8, QVariant val9)
{
    if(_locks.contains(sender + "::" + methodName))
        return 0;

    if(requestId++ >= LONG_LONG_MAX - 1)
        requestId = 0;

    QVariantMap map;
    map[METHOD_NAME] = methodName;
    map[MAP_TYPE] = MAP_TYPE_REQUEST;
    map[ID] = QVariant(requestId);
    map[CLASS_NAME] = sender;

    addToMap(&map, val0, 0);
    addToMap(&map, val1, 1);
    addToMap(&map, val2, 2);
    addToMap(&map, val3, 3);
    addToMap(&map, val4, 4);
    addToMap(&map, val5, 5);
    addToMap(&map, val6, 6);
    addToMap(&map, val7, 7);
    addToMap(&map, val8, 8);
    addToMap(&map, val9, 9);

    if(!validateToken().isNull())
        addValidateToken(&map);

    if(_isTransaction){
        _buffer.append(map);
        return 0;
    }else{
        int res = _socket->write(serializer()->serialize(map));

        if(res == 0)
            return 0;
        else
            return requestId;
    }
}
