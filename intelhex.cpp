#include "intelhex.h"

#include <limits>

namespace {

bool setError(QString *error,
              int lineNumber,
              const QString &message)
{
    if (error) {
        if (lineNumber > 0) {
            *error = QStringLiteral("Dòng HEX %1: %2")
                         .arg(lineNumber)
                         .arg(message);
        } else {
            *error = message;
        }
    }

    return false;
}

bool isHexCharacter(char value)
{
    return (value >= '0' && value <= '9')
    || (value >= 'A' && value <= 'F')
        || (value >= 'a' && value <= 'f');
}

quint16 readWord(const QByteArray &data, int position)
{
    return quint16(
        (quint16(quint8(data.at(position))) << 8)
        | quint16(quint8(data.at(position + 1))));
}

} // namespace

IntelHex::IntelHex() = default;

bool IntelHex::load(const QByteArray &hexText,
                    QString *error)
{
    m_memory.clear();

    if (error)
        error->clear();

    const QList<QByteArray> lines = hexText.split('\n');

    quint32 addressBase = 0;
    bool eofFound = false;

    for (int lineIndex = 0;
         lineIndex < lines.size();
         ++lineIndex) {

        QByteArray line = lines.at(lineIndex).trimmed();

        if (line.isEmpty())
            continue;

        const int lineNumber = lineIndex + 1;

        if (eofFound) {
            return setError(
                error,
                lineNumber,
                QStringLiteral(
                    "Có dữ liệu xuất hiện sau EOF record."));
        }

        if (!line.startsWith(':')) {
            return setError(
                error,
                lineNumber,
                QStringLiteral("Thiếu ký tự ':' ở đầu dòng."));
        }

        line.remove(0, 1);

        if (line.size() < 10 || (line.size() % 2) != 0) {
            return setError(
                error,
                lineNumber,
                QStringLiteral(
                    "Độ dài dòng HEX không hợp lệ."));
        }

        for (char character : line) {
            if (!isHexCharacter(character)) {
                return setError(
                    error,
                    lineNumber,
                    QStringLiteral(
                        "Có ký tự không phải hệ hexadecimal."));
            }
        }

        const QByteArray record =
            QByteArray::fromHex(line);

        if (record.size() < 5) {
            return setError(
                error,
                lineNumber,
                QStringLiteral("Record quá ngắn."));
        }

        const int byteCount =
            int(quint8(record.at(0)));

        if (record.size() != byteCount + 5) {
            return setError(
                error,
                lineNumber,
                QStringLiteral(
                    "Số byte dữ liệu không khớp byte count."));
        }

        quint8 checksum = 0;

        for (char value : record) {
            checksum =
                quint8(checksum + quint8(value));
        }

        if (checksum != 0) {
            return setError(
                error,
                lineNumber,
                QStringLiteral("Checksum không hợp lệ."));
        }

        const quint16 recordAddress =
            readWord(record, 1);

        const quint8 recordType =
            quint8(record.at(3));

        const QByteArray data =
            record.mid(4, byteCount);

        switch (recordType) {
        case 0x00: {
            const quint64 absoluteStart =
                quint64(addressBase)
                + quint64(recordAddress);

            const quint64 absoluteEnd =
                absoluteStart
                + quint64(data.size());

            if (absoluteEnd
                > quint64(
                      std::numeric_limits<quint32>::max())
                      + 1ULL) {
                return setError(
                    error,
                    lineNumber,
                    QStringLiteral(
                        "Địa chỉ vượt quá 32-bit."));
            }

            for (int i = 0;
                 i < data.size();
                 ++i) {

                const quint32 address =
                    quint32(absoluteStart
                            + quint64(i));

                const quint8 value =
                    quint8(data.at(i));

                const auto existing =
                    m_memory.constFind(address);

                if (existing != m_memory.constEnd()
                    && existing.value() != value) {
                    return setError(
                        error,
                        lineNumber,
                        QStringLiteral(
                            "Dữ liệu chồng lấn khác nhau "
                            "tại địa chỉ 0x%1.")
                            .arg(address,
                                 8,
                                 16,
                                 QLatin1Char('0')));
                }

                m_memory.insert(address, value);
            }

            break;
        }

        case 0x01:
            if (byteCount != 0) {
                return setError(
                    error,
                    lineNumber,
                    QStringLiteral(
                        "EOF record phải có độ dài 0."));
            }

            eofFound = true;
            break;

        case 0x02:
            if (data.size() != 2) {
                return setError(
                    error,
                    lineNumber,
                    QStringLiteral(
                        "Extended Segment Address "
                        "phải có 2 byte."));
            }

            addressBase =
                quint32(readWord(data, 0)) << 4;
            break;

        case 0x03:
            if (data.size() != 4) {
                return setError(
                    error,
                    lineNumber,
                    QStringLiteral(
                        "Start Segment Address "
                        "phải có 4 byte."));
            }

            // Không dùng khi flash qua bootloader.
            break;

        case 0x04:
            if (data.size() != 2) {
                return setError(
                    error,
                    lineNumber,
                    QStringLiteral(
                        "Extended Linear Address "
                        "phải có 2 byte."));
            }

            addressBase =
                quint32(readWord(data, 0)) << 16;
            break;

        case 0x05:
            if (data.size() != 4) {
                return setError(
                    error,
                    lineNumber,
                    QStringLiteral(
                        "Start Linear Address "
                        "phải có 4 byte."));
            }

            // Không dùng khi flash qua bootloader.
            break;

        default:
            return setError(
                error,
                lineNumber,
                QStringLiteral(
                    "Record type 0x%1 chưa hỗ trợ.")
                    .arg(recordType,
                         2,
                         16,
                         QLatin1Char('0')));
        }
    }

    if (!eofFound) {
        return setError(
            error,
            0,
            QStringLiteral(
                "Không tìm thấy EOF record."));
    }

    if (m_memory.isEmpty()) {
        return setError(
            error,
            0,
            QStringLiteral(
                "File HEX không chứa dữ liệu firmware."));
    }

    return true;
}

bool IntelHex::isEmpty() const
{
    return m_memory.isEmpty();
}

quint32 IntelHex::lowestAddress() const
{
    if (m_memory.isEmpty())
        return 0;

    return m_memory.firstKey();
}

quint32 IntelHex::highestAddress() const
{
    if (m_memory.isEmpty())
        return 0;

    return m_memory.lastKey();
}

const QMap<quint32, quint8> &
IntelHex::memory() const
{
    return m_memory;
}

QByteArray IntelHex::range(
    quint32 startAddress,
    quint32 length,
    quint8 emptyValue) const
{
    if (length
        > quint32(std::numeric_limits<int>::max())) {
        return {};
    }

    QByteArray output;

    output.fill(
        static_cast<char>(emptyValue),
        static_cast<int>(length));

    for (quint32 index = 0;
         index < length;
         ++index) {

        const auto found =
            m_memory.constFind(startAddress + index);

        if (found != m_memory.constEnd()) {
            output[int(index)] =
                char(found.value());
        }
    }

    return output;
}