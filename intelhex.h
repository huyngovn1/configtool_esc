#ifndef INTELHEX_H
#define INTELHEX_H

#include <QByteArray>
#include <QMap>
#include <QString>

class IntelHex
{
public:
    IntelHex();
    bool load(const QByteArray &hexText,QString *error = nullptr);
    bool isEmpty() const;
    quint32 lowestAddress() const;
    quint32 highestAddress() const;
    const QMap<quint32, quint8> &memory() const;
    QByteArray range(quint32 startAddress,quint32 length,quint8 emptyValue = 0xFF) const;
private:QMap<quint32, quint8> m_memory;
};
#endif // INTELHEX_H