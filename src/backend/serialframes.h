#ifndef BASIC_ESC_SERIALFRAMES_H
#define BASIC_ESC_SERIALFRAMES_H

#include <QByteArray>
#include <QString>

// Small, serial-port-independent parsers. Never interpret an incomplete packet.
namespace SerialFrames {
enum class Result { NeedMore, Complete, Invalid };

inline unsigned byte(const QByteArray &data, int index)
{
    return static_cast<unsigned char>(data.at(index));
}

inline Result stripOptionalEcho(QByteArray &bytes, const QByteArray &request,bool &handled, QString &error)
{
    if (handled || bytes.isEmpty())
        return bytes.isEmpty() ? Result::NeedMore : Result::Complete;
    const int compared = qMin(bytes.size(), request.size());
    if (bytes.left(compared) == request.left(compared)) {
        if (bytes.size() < request.size())
            return Result::NeedMore;
        bytes.remove(0, request.size());
        handled = true;
        return bytes.isEmpty() ? Result::NeedMore : Result::Complete;
    }
    handled = true;
    Q_UNUSED(error)
    return Result::Complete;
}

inline Result takeFourWay(QByteArray &bytes, const QByteArray &request,bool &echoHandled, QByteArray &frame, QString &error)
{
    const Result echo = stripOptionalEcho(bytes, request, echoHandled, error);
    if (echo != Result::Complete)
        return echo;
    if (byte(bytes, 0) != 0x2e) {
        error = QStringLiteral("Invalid 4way response marker.");
        return Result::Invalid;
    }
    if (bytes.size() < 5)
        return Result::NeedMore;
    const int payloadSize = byte(bytes, 4) == 0 ? 256 : int(byte(bytes, 4));
    const int frameSize = 5 + payloadSize + 1 + 2; // header, payload, ACK, CRC
    if (bytes.size() < frameSize)
        return Result::NeedMore;
    frame = bytes.left(frameSize);
    bytes.remove(0, frameSize);
    return Result::Complete;
}

inline Result takeMsp(QByteArray &bytes, const QByteArray &request,bool &echoHandled, QByteArray &frame, QString &error)
{
    const Result echo = stripOptionalEcho(bytes, request, echoHandled, error);
    if (echo != Result::Complete)
        return echo;
    const QByteArray prefix = QByteArrayLiteral("$M");
    const int prefixLength = qMin(2, int(bytes.size()));
    if (bytes.left(prefixLength) != prefix.left(prefixLength)) {
        error = QStringLiteral("Invalid MSP response marker.");
        return Result::Invalid;
    }
    if (bytes.size() < 3)
        return Result::NeedMore;
    if (bytes.at(2) != '>' && bytes.at(2) != '!') {
        error = QStringLiteral("Invalid MSP response direction.");
        return Result::Invalid;
    }
    if (bytes.size() < 5)
        return Result::NeedMore;
    const int frameSize = 6 + int(byte(bytes, 3));
    if (bytes.size() < frameSize)
        return Result::NeedMore;
    frame = bytes.left(frameSize);
    bytes.remove(0, frameSize);
    return Result::Complete;
}

inline Result takeDirect(QByteArray &bytes, const QByteArray &request,int replySize, bool echoExpected,QByteArray &frame, QString &error)
{
    const int prefixSize = echoExpected ? request.size() : 0;
    if (echoExpected) {
        const int compared = qMin(bytes.size(), request.size());
        if (bytes.left(compared) != request.left(compared)) {
            error = QStringLiteral("Direct adapter echo does not match the request.");
            return Result::Invalid;
        }
    }
    if (bytes.size() < prefixSize + replySize)
        return Result::NeedMore;
    frame = bytes.mid(prefixSize, replySize);
    bytes.remove(0, prefixSize + replySize);
    return Result::Complete;
}
} // namespace SerialFrames

#endif
