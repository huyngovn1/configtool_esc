#include "escsession.h"
#include "serialframes.h"
#include "memoryaddress.h"
#include "../model/settingsmodel.h"

#include <QElapsedTimer>
#include <QSerialPort>

namespace {
constexpr int SettingsSize = 48;
constexpr int IdentitySize = 32;
constexpr int ReadSize = SettingsSize + IdentitySize;
constexpr int TransactionTimeout = 1500;

bool isEditedByte(int index)
{
    switch (index) {
    case 17: case 18: case 19: case 24: case 25: case 27: case 28: case 30:
        return true;
    default:
        return false;
    }
}

QString hexByte(unsigned value)
{
    return QStringLiteral("0x%1").arg(value, 2, 16, QLatin1Char('0')).toUpper();
}

QString timeoutError(const QString &operation, int received)
{
    return QStringLiteral("%1 timed out (%2 bytes received).").arg(operation).arg(received);
}
}

EscSession::EscSession(QObject *parent) : QObject(parent) {}

EscSession::~EscSession()
{
    if (m_serial && m_serial->isOpen())
        m_serial->close();
}

void EscSession::setBusy(bool busy)
{
    if (m_busy != busy) {
        m_busy = busy;
        emit busyChanged(busy);
    }
}

void EscSession::closePort()
{
    if (m_serial && m_serial->isOpen() && m_fourWayActive) {
        QByteArray ignored;
        QString error;
        // Also leave passthrough after a failed handshake/read when possible.
        // This does not reboot the FC or send a throttle command.
        if (!exchangeFourWay(m_fourway.makeFourWayCommand(0x34, 0), ignored, error, 500))
            emit logMessage(QStringLiteral("4way exit did not acknowledge: %1").arg(error));
    }
    if (m_serial && m_serial->isOpen())
        m_serial->close();
    m_connected = false;
    m_fourWayActive = false;
    m_originalSettings.clear();
    m_eepromAddress = 0;
    m_addressShift = 0;
    m_chip.clear();
    m_firmware.clear();
    emit connectedChanged(false);
}

void EscSession::failAndClose(const QString &error)
{
    closePort();
    emit errorOccurred(error);
    emit logMessage(error);
}

bool EscSession::sendPacket(const QByteArray &packet, QString &error)
{
    if (!m_serial || !m_serial->isOpen()) {
        error = QStringLiteral("Serial port is not open.");
        return false;
    }
    if (packet.isEmpty()) {
        error = QStringLiteral("Cannot send an empty protocol packet.");
        return false;
    }
    m_serial->clear(QSerialPort::Input);
    if (m_serial->write(packet) != packet.size()) {
        error = QStringLiteral("Serial write failed: %1").arg(m_serial->errorString());
        return false;
    }
    QElapsedTimer timer;
    timer.start();
    while (m_serial->bytesToWrite() > 0 && timer.elapsed() < 1000) {
        if (!m_serial->waitForBytesWritten(50)
            && m_serial->error() != QSerialPort::TimeoutError
            && m_serial->error() != QSerialPort::NoError) {
            error = QStringLiteral("Serial write failed: %1").arg(m_serial->errorString());
            return false;
        }
    }
    if (m_serial->bytesToWrite() > 0) {
        error = QStringLiteral("Serial write timed out.");
        return false;
    }
    return true;
}

bool EscSession::receiveMore(QByteArray &buffer, int timeoutMs, QString &error)
{
    if (m_serial->bytesAvailable() == 0 && !m_serial->waitForReadyRead(qMax(1, timeoutMs))) {
        if (m_serial->error() != QSerialPort::TimeoutError
            && m_serial->error() != QSerialPort::NoError) {
            error = QStringLiteral("Serial read failed: %1").arg(m_serial->errorString());
            return false;
        }
    }
    buffer.append(m_serial->readAll());
    if (buffer.size() > 2048) {
        error = QStringLiteral("Response exceeded the allowed packet size.");
        return false;
    }
    return true;
}

bool EscSession::exchangeMsp(quint8 command, QByteArray &payload, QString &error)
{
    QByteArray request = QByteArrayLiteral("$M<");
    request.append(char(0));
    request.append(char(command));
    request.append(char(command)); // XOR of zero-length payload and command
    if (!sendPacket(request, error))
        return false;
    QByteArray received;
    bool echoHandled = false;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < TransactionTimeout) {
        if (!receiveMore(received, qMin(50, int(TransactionTimeout - timer.elapsed())), error))
            return false;
        QByteArray frame;
        const auto result = SerialFrames::takeMsp(received, request, echoHandled, frame, error);
        if (result == SerialFrames::Result::Invalid)
            return false;
        if (result != SerialFrames::Result::Complete)
            continue;
        unsigned checksum = 0;
        for (int i = 3; i < frame.size() - 1; ++i)
            checksum ^= SerialFrames::byte(frame, i);
        if (checksum != SerialFrames::byte(frame, frame.size() - 1)) {
            error = QStringLiteral("MSP response checksum mismatch.");
            return false;
        }
        if (SerialFrames::byte(frame, 4) != command) {
            error = QStringLiteral("MSP response command does not match the request.");
            return false;
        }
        if (frame.at(2) == '!') {
            error = QStringLiteral("Flight controller rejected MSP command %1.").arg(hexByte(command));
            return false;
        }
        if (!received.isEmpty()) {
            error = QStringLiteral("Unexpected trailing data in MSP response.");
            return false;
        }
        payload = frame.mid(5, int(SerialFrames::byte(frame, 3)));
        return true;
    }
    error = timeoutError(QStringLiteral("MSP handshake"), received.size());
    return false;
}

bool EscSession::exchangeFourWay(const QByteArray &request, QByteArray &payload,
                                 QString &error, int timeoutMs)
{
    if (!sendPacket(request, error))
        return false;
    QByteArray received;
    bool echoHandled = false;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeoutMs) {
        if (!receiveMore(received, qMin(50, int(timeoutMs - timer.elapsed())), error))
            return false;
        QByteArray frame;
        const auto result = SerialFrames::takeFourWay(received, request, echoHandled, frame, error);
        if (result == SerialFrames::Result::Invalid)
            return false;
        if (result != SerialFrames::Result::Complete)
            continue;
        if (!m_fourway.checkCRC(frame, quint16(frame.size()))) {
            error = QStringLiteral("4way response CRC mismatch.");
            return false;
        }
        if (frame.mid(1, 3) != request.mid(1, 3)) {
            error = QStringLiteral("4way response command/address does not match the request.");
            return false;
        }
        const auto ack = SerialFrames::byte(frame, frame.size() - 3);
        if (ack != 0) {
            error = QStringLiteral("4way command %1 failed, ACK %2.")
                        .arg(hexByte(SerialFrames::byte(request, 1)), hexByte(ack));
            return false;
        }
        if (!received.isEmpty()) {
            error = QStringLiteral("Unexpected trailing data in 4way response.");
            return false;
        }
        payload = frame.mid(5, frame.size() - 8);
        return true;
    }
    error = timeoutError(QStringLiteral("4way command"), received.size());
    return false;
}

bool EscSession::exchangeDirect(const QByteArray &request, int replySize,QByteArray &reply, QString &error, int timeoutMs)
{
    if (!sendPacket(request, error))
        return false;
    QByteArray received;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < timeoutMs) {
        if (!receiveMore(received, qMin(50, int(timeoutMs - timer.elapsed())), error))
            return false;
        const auto result = SerialFrames::takeDirect(received, request, replySize,
                                                     m_directEcho, reply, error);
        if (result == SerialFrames::Result::Invalid)
            return false;
        if (result != SerialFrames::Result::Complete)
            continue;
        if (!received.isEmpty()) {
            error = QStringLiteral("Unexpected trailing data in bootloader response.");
            return false;
        }
        if (reply.isEmpty() || reply.back() != char(0x30)) {
            error = QStringLiteral("Bootloader did not return ACK 0x30.");
            return false;
        }
        if (replySize > 1 && !m_rootloader.checkCRC(reply, quint16(replySize - 1))) {
            error = QStringLiteral("Bootloader response CRC mismatch.");
            return false;
        }
        return true;
    }
    error = timeoutError(QStringLiteral("Bootloader command"), received.size());
    return false;
}

bool EscSession::selectProfile(quint8 flashCode, QString &error)
{
    m_addressShift = 0;
    m_firmwareStart = 0x1000;
    switch (flashCode) {
    case 0x2b:
        m_chip = QStringLiteral("G071 / flash code 0x2B");
        m_eepromAddress = 0x7e00;
        m_addressShift = 2;
        break;
    case 0x1f:
        m_chip = QStringLiteral("F051/F053 / flash code 0x1F");
        m_eepromAddress = 0x7c00;
        break;
    case 0x35:
        m_chip = QStringLiteral("F3 / flash code 0x35");
        m_eepromAddress = 0xf800;
        break;
    case 0x15:

        if (m_mode != 0) {
            m_chip = QStringLiteral("NXP / flash code 0x15");
            m_eepromAddress = 0xe000;
            m_firmwareStart = 0x4000;
            break;
        }
        Q_FALLTHROUGH();
    default:
        error = QStringLiteral("Unsupported ESC flash code %1; EEPROM access was not attempted.").arg(hexByte(flashCode));
        return false;
    }
    return true;
}

bool EscSession::startDirect(QString &error)
{
    const QByteArray init = QByteArray::fromHex("0000000000000000000000000d424c48656c69f47d");
    if (!sendPacket(init, error))
        return false;
    QByteArray received;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < TransactionTimeout) {
        if (!receiveMore(received, qMin(50, int(TransactionTimeout - timer.elapsed())), error))
            return false;
        if (received.isEmpty())
            continue;
        const int compared = qMin(received.size(), init.size());
        const bool couldBeEcho = received.left(compared) == init.left(compared);
        if (couldBeEcho && received.size() < init.size())
            continue;
        const bool echo = received.startsWith(init);
        const int offset = echo ? init.size() : 0;
        if (received.size() < offset + 9)
            continue;
        if (received.size() != offset + 9 || received.mid(offset, 3) != QByteArrayLiteral("471")
            || received.at(offset + 8) != char(0x30)) {
            error = QStringLiteral("Invalid bootloader initialization response.");
            return false;
        }
        m_directEcho = echo;
        emit logMessage(echo ? QStringLiteral("Direct adapter uses local echo."): QStringLiteral("Direct adapter has no local echo."));
        return selectProfile(quint8(received.at(offset + 4)), error);
    }
    error = timeoutError(QStringLiteral("Bootloader initialization"), received.size());
    return false;
}

bool EscSession::startFourWay(int channel, QString &error)
{
    QByteArray payload;
    if (m_mode == 1) {
        // Match the reference app: read motor info, then enable 4way passthrough.
        if (!exchangeMsp(0x68, payload, error))
            return false;
        if (!exchangeMsp(0xf5, payload, error))
            return false;
        m_fourWayActive = true;
        if (payload.isEmpty() || SerialFrames::byte(payload, 0) == 0
            || channel >= int(SerialFrames::byte(payload, 0))) {
            error = QStringLiteral("The flight controller did not expose the selected ESC channel.");
            return false;
        }
    } else {
        m_fourWayActive = true;
    }
    if (!exchangeFourWay(m_fourway.makeFourWayCommand(0x37, quint8(channel)), payload, error))
        return false;
    if (payload.size() != 4) {
        error = QStringLiteral("Expected a 4-byte ESC identity response from 4way.");
        return false;
    }
    return selectProfile(quint8(payload.at(1)), error);
}

bool EscSession::fetchSettings(QByteArray &settings, QString &error)
{
    QByteArray block;
    const quint16 address = EscMemory::identityAddress(m_eepromAddress, m_addressShift);
    if (m_mode == 0) {
        QByteArray reply;
        if (!exchangeDirect(m_rootloader.setAddress(address), 1, reply, error)
            || !exchangeDirect(m_rootloader.readFlash(ReadSize), ReadSize + 3, reply, error))
            return false;
        block = reply.left(ReadSize);
    } else {
        if (!exchangeFourWay(m_fourway.makeFourWayReadCommand(ReadSize, address), block, error))
            return false;
    }
    if (block.size() != ReadSize) {
        error = QStringLiteral("Expected 80 bytes of firmware identity and settings; received %1.").arg(block.size());
        return false;
    }
    QByteArray name = block.left(15);
    for (int i = 0; i < name.size(); ++i) {
        const unsigned value = SerialFrames::byte(name, i);
        if (value == 0 || value == 0xff) {
            name.truncate(i);
            break;
        }
        if (value < 32 || value > 126)
            name[i] = '?';
    }
    m_firmware = QString::fromLatin1(name).trimmed();
    if (m_firmware.isEmpty())
        m_firmware = QStringLiteral("Unknown firmware");
    settings = block.mid(IdentitySize, SettingsSize);
    return true;
}

void EscSession::connectDevice(QString port, int mode, int channel)
{
    if (m_busy)
        return;
    setBusy(true);
    m_channel = channel;
    closePort();
    if (mode < 0 || mode > 2 || channel < 0 || channel > 7 || port.trimmed().isEmpty()) {
        failAndClose(QStringLiteral("Select a serial port, connection mode, and ESC channel 1-8."));
        setBusy(false);
        return;
    }
    if (!m_serial)
        m_serial = new QSerialPort(this);
    m_mode = mode;
    m_directEcho = false;
    m_serial->setPortName(port.trimmed());
    m_serial->setBaudRate(mode == 0 ? 19200 : 115200);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);
    QString error;
    if (!m_serial->open(QIODevice::ReadWrite)) {
        failAndClose(QStringLiteral("Could not open %1: %2").arg(port, m_serial->errorString()));
        setBusy(false);
        return;
    }
    emit logMessage(QStringLiteral("Opened %1 at %2 baud; initializing ESC channel %3.").arg(port).arg(m_serial->baudRate()).arg(channel + 1));
    const bool initialized = mode == 0 ? startDirect(error) : startFourWay(channel, error);
    QByteArray settings;
    if (!initialized || !fetchSettings(settings, error)) {
        failAndClose(error);
        setBusy(false);
        return;
    }
    m_originalSettings = settings;
    m_connected = true;
    emit deviceInfo(m_firmware, m_chip, m_eepromAddress);
    emit connectedChanged(true);
    emit settingsRead(settings);
    emit logMessage(QStringLiteral("Connected; read and validated 80 bytes from EEPROM area."));
    setBusy(false);
}

void EscSession::disconnectDevice()
{
    if (m_busy)
        return;
    setBusy(true);
    closePort();
    emit logMessage(QStringLiteral("Disconnected."));
    setBusy(false);
}

void EscSession::readSettings()
{
    if (m_busy)
        return;
    setBusy(true);
    if (!m_connected) {
        emit errorOccurred(QStringLiteral("Connect to an ESC before reading settings."));
        setBusy(false);
        return;
    }
    m_originalSettings.clear();
    QByteArray settings;
    QString error;
    if (!fetchSettings(settings, error)) {
        failAndClose(error);
    } else {
        m_originalSettings = settings;
        emit deviceInfo(m_firmware, m_chip, m_eepromAddress);
        emit settingsRead(settings);
        emit logMessage(QStringLiteral("Settings read successfully."));
    }
    setBusy(false);
}

bool EscSession::validateWrite(const QByteArray &settings, QString &error) const
{
    if (!m_connected || m_originalSettings.size() != SettingsSize || settings.size() != SettingsSize) {
        error = QStringLiteral("Read a complete 48-byte settings snapshot before saving.");
        return false;
    }
    SettingsModel originalModel;
    SettingsModel editedModel;
    if (!originalModel.load(m_originalSettings, &error) || !editedModel.load(settings, &error)) {
        error = QStringLiteral("Save rejected: %1").arg(error);
        return false;
    }
    for (int i = 0; i < SettingsSize; ++i) {
        if (!isEditedByte(i) && settings.at(i) != m_originalSettings.at(i)) {
            error = QStringLiteral("Save rejected: protected EEPROM byte %1 was modified.").arg(i);
            return false;
        }
    }
    return true;
}

bool EscSession::prepareDirectBuffer(int size, QString &error)
{
    const QByteArray request = m_rootloader.setBufferSize(quint16(size));
    if (!sendPacket(request, error))
        return false;
    // SET_BUFFER has no required ACK in this bootloader protocol. Consume an
    // adapter echo and optional ACK now, so neither can satisfy the data ACK.
    QByteArray received;
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < 20) {
        if (!receiveMore(received, qMin(10, int(20 - timer.elapsed())), error))
            return false;
    }
    if (m_directEcho) {
        if (!received.startsWith(request)) {
            error = QStringLiteral("Missing or invalid echo for bootloader SET_BUFFER.");
            return false;
        }
        received.remove(0, request.size());
    }
    if (!received.isEmpty() && received != QByteArray(1, char(0x30))) {
        error = QStringLiteral("Unexpected bootloader SET_BUFFER response.");
        return false;
    }
    return true;
}

bool EscSession::writeDirect(const QByteArray &settings, QString &error)
{
    QByteArray reply;
    return exchangeDirect(m_rootloader.setAddress(m_eepromAddress), 1, reply, error)&& prepareDirectBuffer(settings.size(), error)&& exchangeDirect(m_rootloader.sendBuffer(settings), 1, reply, error)&& exchangeDirect(m_rootloader.writeFlash(), 1, reply, error, 2500);
}
void EscSession::writeSettings(QByteArray settings)
{
    if (m_busy)
        return;
    setBusy(true);
    QString error;
    if (!validateWrite(settings, error)) {
        emit errorOccurred(error);
        emit writeFinished(false, error);
        setBusy(false);
        return;
    }
    emit logMessage(QStringLiteral("Writing 48 settings bytes; a fresh read must match before success."));
    QByteArray ignored;
    const bool written = m_mode == 0 ? writeDirect(settings, error): exchangeFourWay(m_fourway.makeFourWayWriteCommand(settings, SettingsSize,m_eepromAddress), ignored, error, 2500);
    // Once a write has been attempted, no previous snapshot remains valid.
    m_originalSettings.clear();
    QByteArray verified;
    const bool readBack = written && fetchSettings(verified, error);
    if (!readBack || verified != settings) {
        if (readBack) error = QStringLiteral("Read-back mismatch; settings were not confirmed. Reconnect and read before trying again.");
        const QString message = QStringLiteral("Save could not be verified: %1").arg(error);
        failAndClose(message);
        emit writeFinished(false, message);
    } else {
        m_originalSettings = verified;
        emit settingsRead(verified);
        emit writeFinished(true, QStringLiteral("Settings saved and verified by read-back."));
        emit logMessage(QStringLiteral("Settings saved; all 48 bytes match the read-back."));
    }
    setBusy(false);
}
bool EscSession::physicalToWireAddress(
    quint32 physicalAddress,
    quint16 &wireAddress,
    QString &error) const
{
    const quint32 alignment = quint32(1) << m_addressShift;
    if ((physicalAddress % alignment) != 0) {
        error = QStringLiteral("Địa chỉ 0x%1 không đúng căn chỉnh của chip.").arg(physicalAddress, 0, 16);
        return false;
    }
    const quint32 converted =physicalAddress >> m_addressShift;
    if (converted > 0xffffu) {
        error = QStringLiteral(
        "Địa chỉ firmware vượt giới hạn bootloader.");
        return false;
    }
    wireAddress = quint16(converted);
    return true;
}
bool EscSession::writeMemory(quint32 physicalAddress,const QByteArray &data,QString &error)
{
    if (data.isEmpty() || data.size() > 256) {
        error = QStringLiteral("Kích thước khối ghi phải từ 1 đến 256 byte.");
        return false;
    }
    quint16 wireAddress = 0;
    if (!physicalToWireAddress(physicalAddress,wireAddress,error)) {
        return false;
    }
    if (m_mode == 0) {
        QByteArray reply;
        return exchangeDirect(m_rootloader.setAddress(wireAddress),1,reply,error)&& prepareDirectBuffer(data.size(), error)&& exchangeDirect(m_rootloader.sendBuffer(data),1,reply,error)&& exchangeDirect(m_rootloader.writeFlash(),1,reply,error,2500);
    }
    QByteArray reply;
    return exchangeFourWay(m_fourway.makeFourWayWriteCommand(data,data.size(),wireAddress),reply,error,2500);
}

bool EscSession::readMemory(quint32 physicalAddress,int size,QByteArray &data,QString &error)
{
    if (size < 1 || size > 256) {
        error = QStringLiteral("Kích thước khối đọc phải từ 1 đến 256 byte.");
        return false;
    }
    quint16 wireAddress = 0;
    if (!physicalToWireAddress(physicalAddress,wireAddress,error)) {
        return false;
    }
    if (m_mode == 0) {
        QByteArray reply;
        const quint8 encodedSize =size == 256 ? 0 : quint8(size);
        if (!exchangeDirect(m_rootloader.setAddress(wireAddress),1,reply,error)|| !exchangeDirect(m_rootloader.readFlash(encodedSize),size + 3,reply,error)) {
            return false;
        }
        data = reply.left(size);
    } else {
        if (!exchangeFourWay(m_fourway.makeFourWayReadCommand(size,wireAddress),data,error)) {
            return false;
        }
    }
    if (data.size() != size) {
        error = QStringLiteral("Đọc lại sai kích thước: cần %1 byte, nhận %2 byte.").arg(size).arg(data.size());
        return false;
    }
    return true;
}
bool EscSession::validateFirmware(
    const QByteArray &firmware,
    quint32 physicalStart,
    QString &error) const
{
    if (!m_connected) {
        error = QStringLiteral("ESC chưa được kết nối.");
        return false;
    }
    if (m_originalSettings.size() != SettingsSize) {
        error = QStringLiteral(
            "Chưa có bản sao 48 byte EEPROM.");
        return false;
    }
    if (firmware.isEmpty()) {
        error = QStringLiteral("Firmware không có dữ liệu.");
        return false;
    }
    if (physicalStart != m_firmwareStart) {
        error = QStringLiteral("Firmware bắt đầu tại 0x%1, nhưng chip này cần bắt đầu tại 0x%2. ""Có thể bạn đã chọn file FULL chứa bootloader.").arg(physicalStart, 0, 16).arg(m_firmwareStart, 0, 16);
        return false;
    }
    const quint32 eepromPhysical =quint32(m_eepromAddress) << m_addressShift;
    const quint64 firmwareEnd =quint64(physicalStart)+ quint64(firmware.size());
    if (firmwareEnd > quint64(eepromPhysical)) {
        error = QStringLiteral("Firmware chạm vào vùng EEPROM tại 0x%1.").arg(eepromPhysical, 0, 16);
        return false;
    }
    return true;
}

void EscSession::flashFirmware(
    QByteArray firmware,
    quint32 physicalStart)
{
    if (m_busy)
        return;
    setBusy(true);
    QString error;

    if (!validateFirmware(firmware,physicalStart,error)) {
        emit firmwareFinished(false, error);
        setBusy(false);
        return;
    }
    const quint32 eepromPhysical =
    quint32(m_eepromAddress) << m_addressShift;
    QByteArray savedSettings = m_originalSettings;
    QByteArray safetySettings = savedSettings;

    // Byte 0 bằng 0: firmware chưa được xác nhận hoàn chỉnh.
    safetySettings[0] = char(0);
    QByteArray readBack;
    emit firmwareProgress(1,QStringLiteral("Đang bật khóa an toàn EEPROM..."));
    if (!writeMemory(eepromPhysical,safetySettings,error)|| !readMemory(eepromPhysical,safetySettings.size(),readBack,error)|| readBack != safetySettings) {
    if (error.isEmpty()) {error = QStringLiteral("Không xác nhận được khóa an toàn EEPROM.");}
    const QString message =
        QStringLiteral("Dừng nạp firmware: %1").arg(error);
        emit logMessage(message);
        emit firmwareFinished(false, message);
        closePort();
        setBusy(false);
        return;
    }
    const int blockSize = 128;
    for (int offset = 0;
         offset < firmware.size();
         offset += blockSize) {
        const QByteArray block =firmware.mid(offset, blockSize);
        bool verified = false;
        QString blockError;
        for (int attempt = 1;
             attempt <= 3;
             ++attempt) {
            QByteArray verify;
            if (writeMemory(physicalStart + quint32(offset),block,blockError)&& readMemory(physicalStart + quint32(offset),block.size(),verify,blockError)&& verify == block) {
                verified = true;
                break;
            }
            if (verify.size() == block.size()&& verify != block) {blockError =QStringLiteral("Dữ liệu đọc lại không khớp.");
            }
        }
        if (!verified) {const QString message =QStringLiteral("Nạp thất bại tại địa chỉ 0x%1: %2").arg(physicalStart+ quint32(offset),0,16).arg(blockError);
            emit logMessage(message);
            emit firmwareFinished(false, message);
            // Giữ byte an toàn bằng 0.
            closePort();
            setBusy(false);
            return;
        }
        const int completed =offset + block.size();
        const int percent =5 + (completed * 90)/ firmware.size();
        emit firmwareProgress(percent,QStringLiteral("Đang nạp firmware: %1%").arg(percent));
    }

    // Chỉ khôi phục byte 0 sau khi toàn bộ firmware đã được kiểm tra.
    savedSettings[0] = char(1);
    readBack.clear();
    emit firmwareProgress(97,QStringLiteral("Đang khôi phục cấu hình ESC..."));

    if (!writeMemory(eepromPhysical,savedSettings,error)|| !readMemory(eepromPhysical,savedSettings.size(),readBack,error)|| readBack != savedSettings) {

        if (error.isEmpty()) {error = QStringLiteral("EEPROM đọc lại không khớp.");
        }

        const QString message =QStringLiteral("Firmware đã ghi nhưng chưa xác nhận được EEPROM: %1").arg(error);
        emit logMessage(message);
        emit firmwareFinished(false, message);
        closePort();
        setBusy(false);
        return;
    }

    emit firmwareProgress(100,QStringLiteral("Nạp và kiểm tra firmware hoàn tất."));
    QString resetError;
    if (m_mode == 0) {
        QByteArray resetPacket(4, char(0));
        sendPacket(resetPacket, resetError);
    } else {
        QByteArray ignored;
        exchangeFourWay(m_fourway.makeFourWayCommand(0x35,quint8(m_channel)),ignored,resetError,700);
        // Không gửi lệnh thoát 4way lần nữa sau reset.
        m_fourWayActive = false;
    }

    if (!resetError.isEmpty()) {emit logMessage(QStringLiteral("Firmware đã nạp; ESC có thể cần ngắt và cấp lại nguồn: %1").arg(resetError));
    }

    emit firmwareFinished(true,QStringLiteral("Đã nạp và kiểm tra firmware thành công."));

    closePort();
    setBusy(false);
}