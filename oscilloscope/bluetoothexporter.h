#ifndef BLUETOOTHEXPORTER_H
#define BLUETOOTHEXPORTER_H

#include <QObject>
#include <QDBusPendingCallWatcher>

/*!
 * \class BluetoothExporter
 * \brief Sends a file to a paired Bluetooth device via OBEX OPP.
 *
 * Calls the BlueZ obexd daemon over D-Bus (org.bluez.obex.Client1).
 * All D-Bus calls are asynchronous — the UI remains responsive during transfer.
 * Connect to transferStarted() and transferFinished() to update the UI.
 */
class BluetoothExporter : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothExporter(QObject *parent = nullptr);

    Q_INVOKABLE void exportFile(const QString &filePath);

signals:
    void transferStarted();
    void transferFinished(bool success, const QString &message);

private slots:
    void onSessionCreated(QDBusPendingCallWatcher *watcher);
    void onFileSent(QDBusPendingCallWatcher *watcher);

private:
    static constexpr char kTargetMac[] = "";   // TODO: set paired device MAC
    QString m_sessionPath;                     // object path returned by CreateSession
};

#endif
