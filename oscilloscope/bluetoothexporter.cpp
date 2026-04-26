#include "bluetoothexporter.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QDBusPendingCall>
#include <QDBusPendingReply>

// ── D-Bus type registration ───────────────────────────────────────────────────
// BlueZ GetManagedObjects returns a{oa{sa{sv}}}.
// InterfacesAdded signal carries a{sa{sv}} for the new object's interfaces.
using ManagedObjectMap = QMap<QDBusObjectPath, QMap<QString, QVariantMap>>;
using InterfaceDict    = QMap<QString, QVariantMap>;
Q_DECLARE_METATYPE(ManagedObjectMap)
Q_DECLARE_METATYPE(InterfaceDict)

static constexpr int kScanTimeoutMs = 15'000;

BluetoothExporter::BluetoothExporter(QObject *parent)
    : QObject(parent)
{
    qDBusRegisterMetaType<ManagedObjectMap>();
    qDBusRegisterMetaType<InterfaceDict>();

    m_scanTimer.setSingleShot(true);
    connect(&m_scanTimer, &QTimer::timeout, this, &BluetoothExporter::stopScan);
}

// ── Adapter discovery ─────────────────────────────────────────────────────────

QString BluetoothExporter::findAdapterPath() const
{
    QDBusInterface manager("org.bluez", "/",
                           "org.freedesktop.DBus.ObjectManager",
                           QDBusConnection::systemBus());
    QDBusReply<ManagedObjectMap> reply = manager.call("GetManagedObjects");
    if (!reply.isValid()) return {};

    for (auto it = reply.value().cbegin(); it != reply.value().cend(); ++it) {
        if (it.value().contains("org.bluez.Adapter1"))
            return it.key().path();
    }
    return {};
}

// ── Scan ──────────────────────────────────────────────────────────────────────

void BluetoothExporter::startScan()
{
    m_seenAddresses.clear();

    m_adapterPath = findAdapterPath();
    if (m_adapterPath.isEmpty()) {
        qWarning("BluetoothExporter: no Bluetooth adapter found");
        emit scanStopped();
        return;
    }

    // Pre-populate devices BlueZ already knows about from prior scans.
    QDBusInterface manager("org.bluez", "/",
                           "org.freedesktop.DBus.ObjectManager",
                           QDBusConnection::systemBus());
    QDBusReply<ManagedObjectMap> existing = manager.call("GetManagedObjects");
    if (existing.isValid()) {
        for (auto it = existing.value().cbegin(); it != existing.value().cend(); ++it)
            emitIfDevice(it.value());
    }

    // Hook InterfacesAdded so we get new devices live as the scan runs.
    QDBusConnection::systemBus().connect(
        "org.bluez", "/",
        "org.freedesktop.DBus.ObjectManager",
        "InterfacesAdded",
        this,
        SLOT(onInterfacesAdded(QDBusObjectPath, QMap<QString,QVariantMap>)));

    QDBusInterface adapter("org.bluez", m_adapterPath,
                           "org.bluez.Adapter1",
                           QDBusConnection::systemBus());
    adapter.asyncCall("StartDiscovery");

    m_scanTimer.start(kScanTimeoutMs);
}

void BluetoothExporter::stopScan()
{
    m_scanTimer.stop();

    QDBusConnection::systemBus().disconnect(
        "org.bluez", "/",
        "org.freedesktop.DBus.ObjectManager",
        "InterfacesAdded",
        this,
        SLOT(onInterfacesAdded(QDBusObjectPath, QMap<QString,QVariantMap>)));

    if (!m_adapterPath.isEmpty()) {
        QDBusInterface adapter("org.bluez", m_adapterPath,
                               "org.bluez.Adapter1",
                               QDBusConnection::systemBus());
        adapter.asyncCall("StopDiscovery");
    }

    // Final sweep — names that resolved during the scan but weren't emitted yet.
    QDBusInterface manager("org.bluez", "/",
                           "org.freedesktop.DBus.ObjectManager",
                           QDBusConnection::systemBus());
    QDBusReply<ManagedObjectMap> reply = manager.call("GetManagedObjects");
    if (reply.isValid()) {
        for (auto it = reply.value().cbegin(); it != reply.value().cend(); ++it)
            emitIfDevice(it.value());
    }

    emit scanStopped();
}

void BluetoothExporter::onInterfacesAdded(const QDBusObjectPath &,
                                          const QMap<QString, QVariantMap> &interfaces)
{
    emitIfDevice(interfaces);
}

void BluetoothExporter::emitIfDevice(const QMap<QString, QVariantMap> &interfaces)
{
    if (!interfaces.contains("org.bluez.Device1"))
        return;

    const QVariantMap &props = interfaces.value("org.bluez.Device1");
    const QString address = props.value("Address").toString();
    if (address.isEmpty() || m_seenAddresses.contains(address))
        return;

    // Require a resolved human-readable name — MAC address isn't useful to users.
    // Phones are often discovered via BLE advertisement before Classic BT inquiry
    // fills in the Class property, so we don't filter on Class here.  OBEX will
    // return an error naturally if the device can't receive files.
    const QString name = props.value("Name").toString();
    if (name.isEmpty() || name == address)
        return;

    m_seenAddresses.insert(address);
    emit deviceFound(name, address);
}

// ── OBEX file send ────────────────────────────────────────────────────────────

void BluetoothExporter::sendFile(const QString &address, const QString &filePath)
{
    emit transferStarted();

    QDBusInterface client("org.bluez.obex", "/org/bluez/obex",
                          "org.bluez.obex.Client1",
                          QDBusConnection::sessionBus());

    QVariantMap args;
    args["Target"] = "opp";

    QDBusPendingCall call = client.asyncCall("CreateSession", address, args);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    watcher->setProperty("filePath", filePath);
    connect(watcher, &QDBusPendingCallWatcher::finished,
            this, &BluetoothExporter::onSessionCreated);
}

void BluetoothExporter::onSessionCreated(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        emit transferFinished(false, reply.error().message());
        return;
    }

    m_sessionPath = reply.value().path();
    const QString filePath = watcher->property("filePath").toString();

    QDBusInterface session("org.bluez.obex", m_sessionPath,
                           "org.bluez.obex.ObjectPush1",
                           QDBusConnection::sessionBus());

    QDBusPendingCall call = session.asyncCall("SendFile", filePath);
    QDBusPendingCallWatcher *sendWatcher = new QDBusPendingCallWatcher(call, this);
    connect(sendWatcher, &QDBusPendingCallWatcher::finished,
            this, &BluetoothExporter::onFileSent);
}

void BluetoothExporter::onFileSent(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<QDBusObjectPath> reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        cleanupTransfer(false, reply.error().message());
        return;
    }

    // SendFile returns a transfer object path.  The D-Bus call succeeding only
    // means the transfer was queued — the remote device may still reject it.
    // Monitor the transfer's Status property to get the real outcome.
    m_transferPath = reply.value().path();
    QDBusConnection::sessionBus().connect(
        "org.bluez.obex",
        m_transferPath,
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        this,
        SLOT(onTransferPropertiesChanged(QString,QVariantMap,QStringList)));
}

void BluetoothExporter::onTransferPropertiesChanged(const QString &iface,
                                                     const QVariantMap &changed,
                                                     const QStringList &)
{
    if (iface != "org.bluez.obex.Transfer1" || !changed.contains("Status"))
        return;

    const QString status = changed.value("Status").toString();
    if (status == "complete")
        cleanupTransfer(true, "File transferred successfully.");
    else if (status == "error")
        cleanupTransfer(false, "Transfer rejected or failed.");
    // "queued" / "active" — still in progress, keep waiting
}

void BluetoothExporter::cleanupTransfer(bool success, const QString &message)
{
    if (!m_transferPath.isEmpty()) {
        QDBusConnection::sessionBus().disconnect(
            "org.bluez.obex",
            m_transferPath,
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            this,
            SLOT(onTransferPropertiesChanged(QString,QVariantMap,QStringList)));
        m_transferPath.clear();
    }

    QDBusInterface client("org.bluez.obex", "/org/bluez/obex",
                          "org.bluez.obex.Client1",
                          QDBusConnection::sessionBus());
    client.asyncCall("RemoveSession", QDBusObjectPath(m_sessionPath));

    emit transferFinished(success, message);
}
