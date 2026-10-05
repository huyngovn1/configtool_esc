#include "settingsmodel.h"

#include <array>
#include <utility>

namespace {
struct FieldRange
{
    int offset;
    int minimum;
    int maximum;
};

// Do not narrow these to a slider's preferred steps: existing odd PWM
// frequencies and motor pole counts must survive opening and saving a file.
constexpr std::array<FieldRange, 8> editableFields{{
    {17, 0, 1},   // Reversed direction.
    {18, 0, 1},   // Bidirectional mode.
    {19, 0, 1},   // Sine startup.
    {24, 1, 255}, // PWM frequency in kHz.
    {25, 0, 255}, // Startup power: raw stored percentage.
    {27, 1, 255}, // Motor poles.
    {28, 0, 2},   // 0: off, 1: brake on stop, 2: active brake.
    {30, 0, 11},  // Beep volume.
}};

bool fail(QString *error, const QString &message)
{
    if (error)
        *error = message;
    return false;
}

void clearError(QString *error)
{
    if (error)
        error->clear();
}
} // namespace

bool SettingsModel::load(QByteArray bytes, QString *error)
{
    // Clear first so a failed read cannot leave an earlier ESC's settings
    // available for a later write.
    m_bytes.clear();
    m_editable = false;
    clearError(error);

    if (bytes.size() != 48)
        return fail(error, QStringLiteral("Expected exactly 48 settings bytes; received %1.")
                               .arg(bytes.size()));
    if (static_cast<unsigned char>(bytes.at(0)) != 1)
        return fail(error, QStringLiteral("Unsupported settings marker; expected 1."));

    const int version = static_cast<unsigned char>(bytes.at(1));
    if (version < 1 || version > 3)
        return fail(error, QStringLiteral("Unsupported EEPROM version %1; supported versions are 1 to 3.")
                               .arg(version));

    m_bytes = std::move(bytes);
    for (const auto &field : editableFields) {
        const int storedValue = value(field.offset);
        if (storedValue < field.minimum || storedValue > field.maximum)
            return fail(error, QStringLiteral("Unknown value %1 at settings byte %2. Data is preserved; editing is disabled.")
                                   .arg(storedValue).arg(field.offset));
    }

    m_editable = true;
    return true;
}

bool SettingsModel::isEditable() const
{
    return m_editable;
}

QByteArray SettingsModel::bytes() const
{
    return m_bytes;
}

int SettingsModel::value(int offset) const
{
    if (offset < 0 || offset >= m_bytes.size())
        return -1;
    return static_cast<unsigned char>(m_bytes.at(offset));
}

bool SettingsModel::setValue(int offset, int newValue, QString *error)
{
    clearError(error);
    if (!m_editable)
        return fail(error, QStringLiteral("Load a supported settings record before editing."));

    for (const auto &field : editableFields) {
        if (field.offset != offset)
            continue;
        if (newValue < field.minimum || newValue > field.maximum)
            return fail(error, QStringLiteral("Value for byte %1 must be between %2 and %3.")
                                   .arg(offset).arg(field.minimum).arg(field.maximum));
        m_bytes[offset] = static_cast<char>(newValue);
        return true;
    }
    return fail(error, QStringLiteral("Settings byte %1 is read-only in this editor.").arg(offset));
}

QByteArray SettingsModel::demoBytes()
{
    QByteArray bytes(48, '\0');
    bytes[0] = 1;
    bytes[1] = 3;
    bytes[24] = 24;
    bytes[25] = 100;
    bytes[27] = 14;
    bytes[30] = 5;
    return bytes;
}
