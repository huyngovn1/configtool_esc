#include "fourwayif.h"

FourWayIF::FourWayIF() = default;

QByteArray FourWayIF::appendCRC(QByteArray packet)
{
    const uint16_t crc = makeCRC(packet);
    packet.append(static_cast<char>((crc >> 8) & 0xff));
    packet.append(static_cast<char>(crc & 0xff));
    return packet;
}

QByteArray FourWayIF::makeFourWayWriteCommand(const QByteArray sendbuffer,
                                             int buffer_size, uint16_t address)
{
    if (buffer_size < 0 || buffer_size > 256)
        return {};
    const int payloadSize = buffer_size == 0 ? 256 : buffer_size;
    if (sendbuffer.size() != payloadSize)
        return {};

    QByteArray packet;
    packet.append(static_cast<char>(0x2f));
    packet.append(static_cast<char>(0x3b));
    packet.append(static_cast<char>((address >> 8) & 0xff));
    packet.append(static_cast<char>(address & 0xff));
    packet.append(static_cast<char>(payloadSize & 0xff));
    packet.append(sendbuffer);
    return appendCRC(packet);
}

bool FourWayIF::checkCRC(const QByteArray data, uint16_t buffer_length)
{
    if (buffer_length <= 2 || buffer_length > data.size())
        return false;
    const uint16_t crc = makeCRC(data.left(buffer_length - 2));
    return static_cast<uint8_t>((crc >> 8) & 0xff)
            == static_cast<uint8_t>(data.at(buffer_length - 2))
        && static_cast<uint8_t>(crc & 0xff)
            == static_cast<uint8_t>(data.at(buffer_length - 1));
}

QByteArray FourWayIF::makeReadCommand(uint8_t command, int buffer_size, uint16_t address)
{
    if (buffer_size < 0 || buffer_size > 256)
        return {};

    QByteArray packet;
    packet.append(static_cast<char>(0x2f));
    packet.append(static_cast<char>(command));
    packet.append(static_cast<char>((address >> 8) & 0xff));
    packet.append(static_cast<char>(address & 0xff));
    packet.append(static_cast<char>(0x01));
    packet.append(static_cast<char>(buffer_size & 0xff));
    return appendCRC(packet);
}

QByteArray FourWayIF::makeFourWayReadCommand(int buffer_size, uint16_t address)
{
    return makeReadCommand(0x3a, buffer_size, address);
}

QByteArray FourWayIF::makeFourWayReadEEPROMCommand(int buffer_size, uint16_t address)
{
    return makeReadCommand(0x3d, buffer_size, address);
}

uint16_t FourWayIF::makeCRC(const QByteArray data)
{
    // CRC-16/XMODEM: polynomial 0x1021, initial value 0, no reflection.
    // QByteArray stores char, which may be signed. Convert before shifting.
    uint16_t crc = 0;
    for (const char byte : data) {
        crc ^= static_cast<uint16_t>(static_cast<uint8_t>(byte)) << 8;
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc & 0x8000) ? static_cast<uint16_t>((crc << 1) ^ 0x1021)
                                 : static_cast<uint16_t>(crc << 1);
    }
    return crc;
}

QByteArray FourWayIF::makeFourWayCommand(uint8_t cmd, uint8_t device_num)
{
    QByteArray packet;
    packet.append(static_cast<char>(0x2f));
    packet.append(static_cast<char>(cmd));
    packet.append(static_cast<char>(0x00));
    packet.append(static_cast<char>(0x00));
    packet.append(static_cast<char>(0x01));
    packet.append(static_cast<char>(device_num));
    return appendCRC(packet);
}

bool FourWayIF::ACK_required()
{
    return ack_required;
}

bool FourWayIF::ACK_received()
{
    return ack_received;
}

void FourWayIF::set_Ack_req(char ackreq)
{
    ack_req = ackreq;
}
