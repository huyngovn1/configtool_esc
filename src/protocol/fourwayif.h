#ifndef FOURWAYIF_H
#define FOURWAYIF_H

#include <QByteArray>
#include <cstdint>

// Wire-compatible helpers adapted from the four files supplied by the user.
// See PROVENANCE.md. These helpers have no UI or serial-port dependencies.
class FourWayIF
{
public:
    FourWayIF();

    // Address is already in the target's wire units. No implicit /4 conversion.
    // Length must be 1..256 (or 0 meaning 256) and match the write payload.
    // Invalid lengths/payloads return an empty array.
    QByteArray makeFourWayWriteCommand(const QByteArray sendbuffer, int buffer_size,
                                      uint16_t address);
    QByteArray makeFourWayReadCommand(int buffer_size, uint16_t address);
    QByteArray makeFourWayReadEEPROMCommand(int buffer_size, uint16_t address);
    QByteArray makeFourWayCommand(uint8_t cmd, uint8_t device_num);
    uint16_t makeCRC(const QByteArray data);
    bool checkCRC(const QByteArray data, uint16_t buffer_length);
    bool ACK_required();
    bool ACK_received();
    void set_Ack_req(char ackreq);

    uint16_t eeprom_address = 0; // Assigned after device identification.
    uint16_t firmware_start = 0;
    uint8_t connected_motor = 0;
    bool memory_divider_required_four = false;
    bool ack_required = true;
    bool ack_received = false;
    bool passthrough_started = false;
    bool ESC_connected = false;
    bool direct = false;
    uint8_t ack_type = 1;

private:
    QByteArray makeReadCommand(uint8_t command, int buffer_size, uint16_t address);
    QByteArray appendCRC(QByteArray packet);
    char ack_req = 1;
};

#endif // FOURWAYIF_H
