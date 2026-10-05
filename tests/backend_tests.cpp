#include "backend/escsession.h"
#include "backend/serialframes.h"
#include "backend/memoryaddress.h"

#include <QCoreApplication>
#include <iostream>

namespace {
int failures = 0;
void expect(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    using SerialFrames::Result;
    FourWayIF protocol;
    const QByteArray request = protocol.makeFourWayReadCommand(80, 0x7be0);
    QByteArray response = QByteArray::fromHex("2e3a7be050") + QByteArray(80, char(0xa5));
    response.append(char(0)); // ACK
    const quint16 crc = protocol.makeCRC(response);
    response.append(char(crc >> 8));
    response.append(char(crc & 255));

    for (bool echoed : {false, true}) {
        const QByteArray transmission = (echoed ? request : QByteArray()) + response;
        QByteArray received;
        bool echoHandled = false;
        for (int i = 0; i < transmission.size(); ++i) {
            received.append(transmission.at(i));
            QByteArray frame;
            QString error;
            const auto result = SerialFrames::takeFourWay(received, request, echoHandled, frame, error);
            if (i < transmission.size() - 1) {
                expect(result == Result::NeedMore, "fragmented 4way must wait for full CRC");
            } else {
                expect(result == Result::Complete && frame == response && received.isEmpty(),
                       "4way response extracted exactly, with and without echo");
                expect(protocol.checkCRC(frame, quint16(frame.size())), "4way payload preserves valid CRC");
            }
        }
    }

    const QByteArray mspRequest = QByteArray::fromHex("244d3c00f5f5");
    const QByteArray mspResponse = QByteArray::fromHex("244d3e01f504f0");
    for (bool echoed : {false, true}) {
        const QByteArray transmission = (echoed ? mspRequest : QByteArray()) + mspResponse;
        QByteArray received;
        bool echoHandled = false;
        for (int i = 0; i < transmission.size(); ++i) {
            received.append(transmission.at(i));
            QByteArray frame;
            QString error;
            const auto result = SerialFrames::takeMsp(received, mspRequest, echoHandled, frame, error);
            expect(i == transmission.size() - 1 ? result == Result::Complete && frame == mspResponse
                                                : result == Result::NeedMore,
                   "fragmented MSP handshake with or without echo");
        }
    }

    const QByteArray directRequest = QByteArray::fromHex("ff007c0010d4");
    QByteArray directReceived = directRequest;
    QByteArray frame;
    QString error;
    expect(SerialFrames::takeDirect(directReceived, directRequest, 1, true, frame, error) == Result::NeedMore,
           "direct request echo cannot satisfy ACK");
    directReceived.append(char(0x30));
    expect(SerialFrames::takeDirect(directReceived, directRequest, 1, true, frame, error) == Result::Complete
           && frame == QByteArray(1, char(0x30)), "direct echo stripped before ACK");
    directReceived = QByteArray::fromHex("ff007d0010d4");
    expect(SerialFrames::takeDirect(directReceived, directRequest, 1, true, frame, error) == Result::Invalid,
           "corrupt direct echo rejected");
    QByteArray invalid = QByteArray::fromHex("2f00");
    bool handled = false;
    expect(SerialFrames::takeFourWay(invalid, request, handled, frame, error) == Result::Invalid,
           "invalid 4way marker rejected");
    QByteArray large = QByteArray::fromHex("2e3a000000") + QByteArray(256, 'x') + QByteArray(3, char(0));
    handled = false;
    expect(SerialFrames::takeFourWay(large, request, handled, frame, error) == Result::Complete
           && frame.size() == 264, "4way length-zero sentinel means 256 bytes");

    expect(EscMemory::identityAddress(0x7e00, 2) == 0x7df8, "G071 identity uses shifted wire address");
    expect((EscMemory::identityAddress(0x7e00, 2) << 2) + 32 == (0x7e00 << 2),
           "G071 identity plus 32 bytes lands at EEPROM");
    expect(EscMemory::identityAddress(0x7c00, 0) == 0x7be0, "F051 identity uses byte address");
    expect(EscMemory::identityAddress(0xf800, 0) == 0xf7e0, "F3 identity uses byte address");

    // Failures before opening any port still release the UI busy state.
    EscSession session;
    int busyStarts = 0;
    int busyEnds = 0;
    int failedWrites = 0;
    QObject::connect(&session, &EscSession::busyChanged, [&](bool busy) {
        busy ? ++busyStarts : ++busyEnds;
    });
    QObject::connect(&session, &EscSession::writeFinished, [&](bool ok, const QString &) {
        if (!ok)
            ++failedWrites;
    });
    session.readSettings();
    session.writeSettings(QByteArray(48, char(0)));
    session.connectDevice(QString(), 0, 0);
    expect(busyStarts == 3 && busyEnds == 3, "all early failure paths release busy state");
    expect(failedWrites == 1, "disconnected writes report failure");

    if (!failures)
        std::cout << "Backend framing, address, and early-failure checks passed.\n";
    return failures ? 1 : 0;
}
