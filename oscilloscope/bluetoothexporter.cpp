#include "bluetoothexporter.h"

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusPendingReply>

BluetoothExporter::BluetoothExporter(QObject *parent)
    : QObject(parent)
{
}

void BluetoothExporter::exportFile(const QString &filePath)
{
    emit transferStarted();

    QDBusInterface client("org.bluez.obex",
                          "/org/bluez/obex",
                          "org.bluez.obex.Client1",
                          QDBusConnection::sessionBus());
    
    QVariantMap args;
    args["Target"] = "opp";

    QDBusPendingCall call = client.asyncCall("CreateSession", kTargetMac, args); 
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    watcher->setProperty("filePath", filePath);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &BluetoothExporter::onSessionCreated);

}

void BluetoothExporter::onSessionCreated(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    if(reply.isError()){
        emit transferFinished(false, reply.error().message());
        watcher->deleteLater();
        return;
    } else {
        m_sessionPath = reply.value().path();
        const QString filePath = watcher->property("filePath").toString();
        QDBusInterface client("org.bluez.obex",
                              m_sessionPath,
                              "org.bluez.obex.ObjectPush1",
                              QDBusConnection::sessionBus());
        

        QDBusPendingCall call = client.asyncCall("SendFile", filePath); 
        watcher->deleteLater();
        QDBusPendingCallWatcher *sendWatcher = new QDBusPendingCallWatcher(call, this);
        sendWatcher->setProperty("filePath", filePath);
        connect(sendWatcher, &QDBusPendingCallWatcher::finished,
                this, &BluetoothExporter::onFileSent);
    }
}

void BluetoothExporter::onFileSent(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    if(reply.isError()) { 
        emit transferFinished(false, reply.error().message());
        watcher->deleteLater();
        return;
    } else {
        watcher->deleteLater();
        QDBusInterface client("org.bluez.obex",
                              "/org/bluez/obex",
                              "org.bluez.obex.Client1",
                              QDBusConnection::sessionBus());
        client.asyncCall("RemoveSession", QDBusObjectPath(m_sessionPath));
        emit transferFinished(true, "File transferred successfully.");
    }
}
