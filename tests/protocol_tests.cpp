#include "protocol/BF_ROOTLOADER.h"
#include "protocol/BF_ROOTLOADER.h" // Header guard regression.
#include "protocol/fourwayif.h"
#include "protocol/fourwayif.h"

#include <iostream>

namespace {
int failures = 0;

void expect(bool condition, const char *name)
{
    if (!condition) {
        std::cerr << "FAIL: " << name << '\n';
        ++failures;
    }
}

void expectPacket(const QByteArray &actual, const char *hex, const char *name)
{
    const QByteArray expected = QByteArray::fromHex(hex);
    if (actual != expected) {
        std::cerr << "FAIL: " << name << "\n  expected: " << expected.toHex().constData()
                  << "\n  received: " << actual.toHex().constData() << '\n';
        ++failures;
    }
}
} // namespace

int main()
{
    BF_ROOTLOADER direct;
    FourWayIF four;

    // Fixed packet vectors independently calculated from the supplied wire
    // formats. The conventional "123456789" check values also cover each CRC.
    expectPacket(direct.setAddress(0x7c00), "ff007c0010d4", "direct address");
    expectPacket(direct.setBufferSize(48), "fe00003031fc", "direct buffer 48");
    expectPacket(direct.setBufferSize(256), "fe0001003078", "direct buffer 256");
    expectPacket(direct.setBufferSize(0), "fe0001003078", "direct buffer 256 alias");
    expectPacket(direct.writeFlash(), "0101c050", "direct write");
    expectPacket(direct.readFlash(48), "033000e4", "direct read 48");
    expectPacket(direct.readFlash(0), "030000f0", "direct read 256");
    expectPacket(direct.sendBuffer(QByteArray::fromHex("00807fff55aa")),
                 "00807fff55aa9715", "direct high-bit bytes");
    expectPacket(direct.sendBuffer("123456789"), "3132333435363738393dbb",
                 "CRC-16/ARC check value 0xbb3d, low byte first");

    expect(four.makeCRC("123456789") == 0x31c3, "CRC-16/XMODEM check value 0x31c3");
    expectPacket(four.makeFourWayCommand(0x37, 0), "2f3700000100a800", "4-way init");
    expectPacket(four.makeFourWayReadCommand(48, 0x7c00), "2f3a7c000130b26e",
                 "4-way read");
    expectPacket(four.makeFourWayReadCommand(256, 0x7e00), "2f3a7e0001006955",
                 "4-way read 256 and unchanged wire address");
    expectPacket(four.makeFourWayReadCommand(0, 0x7e00), "2f3a7e0001006955",
                 "4-way read zero sentinel");
    expectPacket(four.makeFourWayReadEEPROMCommand(8, 0xf800), "2f3df80001087528",
                 "4-way EEPROM read");
    expectPacket(four.makeFourWayWriteCommand(QByteArray::fromHex("00807fff55aa"),
                                             6, 0x7c00),
                 "2f3b7c000600807fff55aa567a", "4-way write high-bit bytes");

    const QByteArray directReply = QByteArray::fromHex("00807fff55aa971530");
    expect(direct.checkCRC(directReply, 8), "direct reply excludes final ACK");
    expect(!direct.checkCRC(directReply, 9), "direct CRC rejects included ACK");
    expect(!direct.checkCRC(QByteArray(), 0), "direct CRC rejects empty");
    expect(!direct.checkCRC(QByteArray(1, '\0'), 1), "direct CRC rejects short");
    expect(!direct.checkCRC(directReply, 65535), "direct CRC rejects out of bounds");
    // The public void API must also be safe when asked to read past the input.
    direct.makeCRC(QByteArray(1, '\0'), 65535);

    const QByteArray fourPacket = QByteArray::fromHex("2f3b7c000600807fff55aa567a");
    expect(four.checkCRC(fourPacket, static_cast<uint16_t>(fourPacket.size())),
           "4-way CRC accepts fixed high-bit packet");
    expect(!four.checkCRC(QByteArray(), 0), "4-way CRC rejects empty");
    expect(!four.checkCRC(QByteArray(2, '\0'), 2), "4-way CRC rejects short");
    expect(!four.checkCRC(fourPacket, 65535), "4-way CRC rejects out of bounds");
    for (int i = 0; i < fourPacket.size(); ++i) {
        QByteArray damaged = fourPacket;
        damaged[i] = static_cast<char>(static_cast<unsigned char>(damaged.at(i)) ^ 1u);
        expect(!four.checkCRC(damaged, static_cast<uint16_t>(damaged.size())),
               "4-way CRC detects every one-bit packet mutation");
    }
    for (int i = 0; i < directReply.size() - 1; ++i) {
        QByteArray damaged = directReply;
        damaged[i] = static_cast<char>(static_cast<unsigned char>(damaged.at(i)) ^ 1u);
        expect(!direct.checkCRC(damaged, 8),
               "direct CRC detects every one-bit reply mutation");
    }

    QByteArray allBytes;
    for (int i = 0; i < 256; ++i)
        allBytes.append(static_cast<char>(i));
    const QByteArray direct256 = direct.sendBuffer(allBytes);
    const QByteArray four256 = four.makeFourWayWriteCommand(allBytes, 256, 0x7c00);
    expect(direct256.size() == 258 && direct256.left(256) == allBytes,
           "direct full 256-byte payload preserved");
    expect(direct256.right(2) == QByteArray::fromHex("d3ba"),
           "direct full 256-byte CRC vector");
    expect(four256.size() == 263 && four256.at(4) == '\0'
               && four256.mid(5, 256) == allBytes,
           "4-way full 256-byte payload and zero length byte");
    expect(four256.right(2) == QByteArray::fromHex("55df"),
           "4-way full 256-byte CRC vector");
    expect(four.makeFourWayWriteCommand(allBytes, 0, 0x7c00) == four256,
           "4-way write zero sentinel equals 256");
    expect(direct.setBufferSize(257).isEmpty(), "direct rejects oversized buffer");
    expect(direct.sendBuffer({}).isEmpty(), "direct rejects empty payload");
    expect(direct.sendBuffer(QByteArray(257, '\0')).isEmpty(),
           "direct rejects oversized payload");
    expect(four.makeFourWayReadCommand(-1, 0).isEmpty(), "4-way rejects negative read");
    expect(four.makeFourWayReadCommand(257, 0).isEmpty(), "4-way rejects oversized read");
    expect(four.makeFourWayReadEEPROMCommand(257, 0).isEmpty(),
           "4-way rejects oversized EEPROM read");
    expect(four.makeFourWayWriteCommand(allBytes, 255, 0).isEmpty(),
           "4-way rejects payload/length mismatch");
    expect(four.makeFourWayWriteCommand({}, 0, 0).isEmpty(),
           "4-way zero sentinel requires 256 actual bytes");
    expect(four.makeFourWayWriteCommand({}, -1, 0).isEmpty(),
           "4-way rejects negative write");
    expect(four.makeFourWayWriteCommand(QByteArray(257, '\0'), 257, 0).isEmpty(),
           "4-way rejects oversized write");

    expect(direct.ACK_required() && !direct.ACK_received(), "direct initial ACK state");
    expect(four.ACK_required() && !four.ACK_received(), "4-way initial ACK state");
    direct.ack_received = true;
    four.ack_received = true;
    expect(direct.ACK_received() && four.ACK_received(), "ACK getters return state");
    direct.set_Ack_req(0);
    four.set_Ack_req(0);
    expect(direct.eeprom_address == 0 && direct.connected_motor == 0
               && !direct.memory_divider_required_four && !direct.ESC_connected,
           "direct initialized metadata");
    expect(four.eeprom_address == 0 && four.firmware_start == 0 && !four.direct
               && !four.memory_divider_required_four && !four.ESC_connected,
           "4-way initialized metadata");

    if (failures == 0)
        std::cout << "All protocol regression checks passed.\n";
    return failures == 0 ? 0 : 1;
}
