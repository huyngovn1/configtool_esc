#pragma once

#include <QByteArray>
#include <QString>

// A conservative editor for the 48-byte AM32 settings record used by the
// supplied configurator. Unexposed bytes are opaque and remain untouched.
class SettingsModel
{
public:
    // Unsupported records clear the previous state. A structurally valid
    // record with an unknown exposed value is retained, but cannot be edited.
    bool load(QByteArray bytes, QString *error = nullptr);
    bool isEditable() const;
    QByteArray bytes() const;
    int value(int offset) const;
    bool setValue(int offset, int value, QString *error = nullptr);

    // Synthetic preview data, not a device-specific factory default.
    static QByteArray demoBytes();

private:
    QByteArray m_bytes;
    bool m_editable = false;
};
