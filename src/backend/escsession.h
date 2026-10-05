#ifndef BASIC_ESC_ESCSESSION_H
#define BASIC_ESC_ESCSESSION_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include "../protocol/BF_ROOTLOADER.h"
#include "../protocol/fourwayif.h"

class QSerialPort;

// Move this object to a QThread before invoking its slots. The serial port is
// created there on first connection. All waits are bounded and run in that thread.
class EscSession final : public QObject
{
    Q_OBJECT
public:
    explicit EscSession(QObject *parent = nullptr);
    ~EscSession() override;

public slots:
    void connectDevice(QString port, int mode, int channel);
    void disconnectDevice();
    void readSettings();
    void writeSettings(QByteArray settings);

signals:
    void busyChanged(bool busy);
    void connectedChanged(bool connected);
    void logMessage(QString message);
    void deviceInfo(QString firmware, QString chip, quint32 eepromAddress);
    void settingsRead(QByteArray settings);
    void writeFinished(bool success, QString message);
    void errorOccurred(QString message);

private:
    bool sendPacket(const QByteArray &packet, QString &error);
    bool receiveMore(QByteArray &buffer, int timeoutMs, QString &error);
    bool exchangeMsp(quint8 command, QByteArray &payload, QString &error);
    bool exchangeFourWay(const QByteArray &request, QByteArray &payload,
                         QString &error, int timeoutMs = 1500);
    bool exchangeDirect(const QByteArray &request, int replySize,
                        QByteArray &reply, QString &error, int timeoutMs = 1500);
    bool startDirect(QString &error);
    bool startFourWay(int channel, QString &error);
    bool selectProfile(quint8 flashCode, QString &error);
    bool fetchSettings(QByteArray &settings, QString &error);
    bool writeDirect(const QByteArray &settings, QString &error);
    bool prepareDirectBuffer(int size, QString &error);
    bool validateWrite(const QByteArray &settings, QString &error) const;
    void setBusy(bool busy);
    void closePort();
    void failAndClose(const QString &error);

    QSerialPort *m_serial = nullptr;
    BF_ROOTLOADER m_rootloader;
    FourWayIF m_fourway;
    bool m_busy = false;
    bool m_connected = false;
    bool m_directEcho = false;
    bool m_fourWayActive = false;
    int m_mode = 0;
    quint16 m_eepromAddress = 0;
    quint8 m_addressShift = 0;
    QString m_chip;
    QString m_firmware;
    QByteArray m_originalSettings;
};

#endif
