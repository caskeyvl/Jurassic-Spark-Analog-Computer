#ifndef BLUETOOTHEXPORTER_H
#define BLUETOOTHEXPORTER_H

#include <QObject>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QMap>
#include <QSet>
#include <QString>
#include <QTimer>
#include <QVariant>

/*!
 * \class BluetoothExporter
 * \brief Discovers nearby Bluetooth devices and sends a file via OBEX OPP.
 *
 * Uses BlueZ over D-Bus (system bus for adapter/discovery, session bus for OBEX).
 * No prior pairing required — OBEX OPP works with any discoverable device.
 *
 * Typical QML flow:
 *   1. Call startScan() — emits deviceFound() for each device as discovered.
 *   2. scanStopped() fires when the scan times out (or stopScan() is called).
 *   3. Call sendFile(address, filePath) — emits transferStarted(), then
 *      transferFinished(success, message) when done.
 */
class BluetoothExporter : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothExporter(QObject *parent = nullptr);

    /*! Starts a 15-second Bluetooth device discovery.
     *  Emits deviceFound() for each device seen.  Emits scanStopped() when done. */
    Q_INVOKABLE void startScan();

    /*! Stops an in-progress scan early. */
    Q_INVOKABLE void stopScan();

    /*! Sends \a filePath to \a address via OBEX OPP.  No pairing needed. */
    Q_INVOKABLE void sendFile(const QString &address, const QString &filePath);

signals:
    void deviceFound(const QString &name, const QString &address);
    void scanStopped();

    void transferStarted();
    void transferFinished(bool success, const QString &message);

private slots:
    void onInterfacesAdded(const QDBusObjectPath &path,
                           const QMap<QString, QVariantMap> &interfaces);
    void onSessionCreated(QDBusPendingCallWatcher *watcher);
    void onFileSent(QDBusPendingCallWatcher *watcher);
    void onTransferPropertiesChanged(const QString &iface,
                                     const QVariantMap &changed,
                                     const QStringList &invalidated);

private:
    QString findAdapterPath() const;
    void    emitIfDevice(const QMap<QString, QVariantMap> &interfaces);
    void    cleanupTransfer(bool success, const QString &message);

    QString m_adapterPath;
    QString m_sessionPath;
    QString m_transferPath;
    QTimer  m_scanTimer;
    QSet<QString> m_seenAddresses;
};

#endif
