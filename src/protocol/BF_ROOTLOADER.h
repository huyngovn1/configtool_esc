#ifndef BF_ROOTLOADER_H
#define BF_ROOTLOADER_H

#include <QByteArray>
#include <cstdint>

// Wire-compatible helpers adapted from the four files supplied by the user.
// See PROVENANCE.md. These helpers have no UI or serial-port dependencies.
class BF_ROOTLOADER
{
public:
    BF_ROOTLOADER();

    // Address is already in the target's wire units. Conversion belongs to the
    // device/session layer, because different MCUs use different units.
    QByteArray setAddress(uint16_t address);
    // Valid lengths: 1..256; 0 is accepted as an API alias for 256 bytes.
    // SET_BUFFER encodes 256 as high size byte 1, low size byte 0.
    // Invalid lengths return an empty array.
    QByteArray setBufferSize(uint16_t size);
    QByteArray writeFlash();
    // Valid payload lengths: 1..256. Invalid lengths return an empty array.
    QByteArray sendBuffer(QByteArray inbuffer);
    // A zero size means 256 bytes, matching the supplied implementation.
    QByteArray readFlash(uint8_t size);
    // Calculate over the first length bytes. Invalid bounds reset cached CRC.
    void makeCRC(const QByteArray pBuff, uint16_t length);
    bool checkCRC(const QByteArray pBuff, uint16_t length);
    bool ACK_required();
    bool ACK_received();
    void set_Ack_req(char ackreq);

    uint16_t eeprom_address = 0; // Assigned after device identification.
    uint8_t connected_motor = 0;
    bool memory_divider_required_four = false;
    bool ack_required = true;
    bool ack_received = false;
    bool passthrough_started = false;
    bool ESC_connected = false;
    uint8_t ack_type = 1;

private:
    char ack_req = 1;
    uint8_t calculated_crc_low_byte = 0;
    uint8_t calculated_crc_high_byte = 0;
};

#endif // BF_ROOTLOADER_H
