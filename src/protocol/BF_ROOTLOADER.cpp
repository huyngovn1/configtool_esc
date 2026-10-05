#include "BF_ROOTLOADER.h"

BF_ROOTLOADER::BF_ROOTLOADER() = default;

bool BF_ROOTLOADER::checkCRC(const QByteArray pBuff, uint16_t length)
{
    if (length < 2 || length > pBuff.size())
        return false;

    makeCRC(pBuff, static_cast<uint16_t>(length - 2));
    return calculated_crc_low_byte == static_cast<uint8_t>(pBuff.at(length - 2))
        && calculated_crc_high_byte == static_cast<uint8_t>(pBuff.at(length - 1));
}

QByteArray BF_ROOTLOADER::setAddress(uint16_t address)
{
    QByteArray packet;
    packet.append(static_cast<char>(0xff));
    packet.append(static_cast<char>(0x00));
    packet.append(static_cast<char>((address >> 8) & 0xff));
    packet.append(static_cast<char>(address & 0xff));
    return sendBuffer(packet);
}

QByteArray BF_ROOTLOADER::setBufferSize(uint16_t size)
{
    if (size > 256)
        return {};

    QByteArray packet;
    packet.append(static_cast<char>(0xfe));
    packet.append(static_cast<char>(0x00));
    // Unlike READ_FLASH, SET_BUFFER uses a separate high size byte for 256.
    // This fixes the supplied helper's zero-length encoding for full blocks.
    packet.append(static_cast<char>((size == 0 || size == 256) ? 0x01 : 0x00));
    packet.append(static_cast<char>(size & 0xff));
    return sendBuffer(packet);
}

QByteArray BF_ROOTLOADER::writeFlash()
{
    return sendBuffer(QByteArray::fromHex("0101"));
}

QByteArray BF_ROOTLOADER::readFlash(uint8_t size)
{
    QByteArray packet;
    packet.append(static_cast<char>(0x03));
    packet.append(static_cast<char>(size));
    return sendBuffer(packet);
}

QByteArray BF_ROOTLOADER::sendBuffer(QByteArray inbuffer)
{
    if (inbuffer.isEmpty() || inbuffer.size() > 256)
        return {};

    makeCRC(inbuffer, static_cast<uint16_t>(inbuffer.size()));
    inbuffer.append(static_cast<char>(calculated_crc_low_byte));
    inbuffer.append(static_cast<char>(calculated_crc_high_byte));
    return inbuffer;
}

void BF_ROOTLOADER::makeCRC(const QByteArray pBuff, uint16_t length)
{
    calculated_crc_low_byte = 0;
    calculated_crc_high_byte = 0;
    if (length > pBuff.size())
        return;

    // CRC-16/ARC: polynomial 0x8005 reflected to 0xa001, initial value 0.
    // Explicit byte extraction avoids host endianness and packed-union issues.
    uint16_t crc = 0;
    for (uint16_t i = 0; i < length; ++i) {
        crc ^= static_cast<uint8_t>(pBuff.at(i));
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc & 1) ? static_cast<uint16_t>((crc >> 1) ^ 0xa001)
                            : static_cast<uint16_t>(crc >> 1);
    }
    calculated_crc_low_byte = static_cast<uint8_t>(crc & 0xff);
    calculated_crc_high_byte = static_cast<uint8_t>((crc >> 8) & 0xff);
}

bool BF_ROOTLOADER::ACK_required()
{
    return ack_required;
}

bool BF_ROOTLOADER::ACK_received()
{
    return ack_received;
}

void BF_ROOTLOADER::set_Ack_req(char ackreq)
{
    ack_req = ackreq;
}
