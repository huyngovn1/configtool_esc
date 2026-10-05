#include "model/settingsmodel.h"

#include <QCoreApplication>
#include <QDebug>

#include <array>
#include <cstdlib>

namespace {
int failures = 0;

void check(bool condition, const char *message)
{
    if (!condition) {
        qCritical().noquote() << message;
        ++failures;
    }
}

QByteArray record(int version)
{
    auto bytes = SettingsModel::demoBytes();
    bytes[1] = static_cast<char>(version);
    // Exercise preservation of opaque fields and bytes above signed-char 127.
    for (int i = 2; i < bytes.size(); ++i) {
        if (i != 17 && i != 18 && i != 19 && i != 24 && i != 25 &&
            i != 27 && i != 28 && i != 30)
            bytes[i] = static_cast<char>((i * 13 + 127) & 255);
    }
    bytes[24] = 49; // A valid odd frequency must not round to a UI step.
    bytes[25] = static_cast<char>(201);
    bytes[27] = 15; // Nor an odd pole count.
    bytes[28] = 2;
    return bytes;
}

void testPreservation()
{
    constexpr std::array<int, 8> offsets{{17, 18, 19, 24, 25, 27, 28, 30}};
    constexpr std::array<int, 8> values{{1, 1, 1, 255, 255, 255, 1, 11}};
    for (int version = 1; version <= 3; ++version) {
        SettingsModel model;
        QString error = QStringLiteral("stale error");
        const auto original = record(version);
        check(model.load(original, &error), "Known version should load.");
        check(error.isEmpty(), "Successful load should clear the old error.");
        check(model.isEditable(), "Known fields should be editable.");
        check(model.bytes() == original, "Loading must preserve all bytes exactly.");
        check(model.value(25) == 201, "Raw values must be unsigned.");

        auto expected = original;
        for (std::size_t i = 0; i < offsets.size(); ++i) {
            check(model.setValue(offsets[i], values[i], &error), "Exposed value should be editable.");
            expected[offsets[i]] = static_cast<char>(values[i]);
            check(model.bytes() == expected, "Each edit must change only its selected byte.");
        }
        // QByteArray implicit sharing must not expose mutable model storage.
        auto externalCopy = model.bytes();
        externalCopy[0] = 0;
        check(model.bytes() == expected, "Modifying returned bytes must not mutate the model.");
    }
}

void testRejectedEdits()
{
    SettingsModel model;
    const auto original = record(3);
    check(model.load(original), "Test record should load.");
    for (int offset = -1; offset <= 48; ++offset) {
        if (offset == 17 || offset == 18 || offset == 19 || offset == 24 ||
            offset == 25 || offset == 27 || offset == 28 || offset == 30)
            continue;
        QString error;
        check(!model.setValue(offset, 0, &error), "Unexposed byte edit must be rejected.");
        check(!error.isEmpty(), "Rejected edit should explain its failure.");
        check(model.bytes() == original, "Rejected edit must leave the record unchanged.");
    }
    constexpr std::array<std::array<int, 3>, 8> ranges{{
        {{17, 0, 1}}, {{18, 0, 1}}, {{19, 0, 1}}, {{24, 1, 255}},
        {{25, 0, 255}}, {{27, 1, 255}}, {{28, 0, 2}}, {{30, 0, 11}},
    }};
    for (const auto &range : ranges) {
        for (const int invalid : {range[1] - 1, range[2] + 1}) {
            check(!model.setValue(range[0], invalid), "Out-of-range field edit must fail.");
            check(model.bytes() == original, "Out-of-range edit must preserve the record.");
        }
        for (const int boundary : {range[1], range[2]}) {
            check(model.setValue(range[0], boundary), "Inclusive field boundary should be accepted.");
            check(model.value(range[0]) == boundary, "Accepted boundary must be stored without conversion.");
            check(model.load(original), "Restore fixture after boundary test.");
        }
    }
    check(model.value(-1) == -1 && model.value(48) == -1, "Invalid read offsets should return -1.");
}

void testMalformedRecords()
{
    SettingsModel model;
    const auto original = record(3);
    const auto assertRejected = [&](const QByteArray &invalid) {
        check(model.load(original), "Load prior record for stale-state test.");
        QString error;
        check(!model.load(invalid, &error), "Malformed record should be rejected.");
        check(!error.isEmpty(), "Malformed load should report an error.");
        check(!model.isEditable(), "Malformed record must disable editing.");
        check(model.bytes().isEmpty(), "Malformed record must clear previous bytes.");
        check(!model.setValue(17, 1), "No edit may follow a malformed load.");
    };
    for (int length : {0, 1, 47, 49, 96})
        assertRejected(QByteArray(length, '\1'));
    for (int marker : {0, 2, 255}) {
        auto bytes = original;
        bytes[0] = static_cast<char>(marker);
        assertRejected(bytes);
    }
    for (int version : {0, 4, 128, 255}) {
        auto bytes = original;
        bytes[1] = static_cast<char>(version);
        assertRejected(bytes);
    }
}

void testUnknownValues()
{
    constexpr std::array<std::array<int, 2>, 7> invalidValues{{
        {{17, 2}}, {{18, 255}}, {{19, 2}}, {{24, 0}},
        {{27, 0}}, {{28, 3}}, {{30, 12}},
    }};
    SettingsModel model;
    for (const auto &invalid : invalidValues) {
        check(model.load(record(1)), "Load prior record before unknown-value test.");
        auto bytes = record(3);
        bytes[invalid[0]] = static_cast<char>(invalid[1]);
        QString error;
        check(!model.load(bytes, &error), "Unknown exposed value should prevent supported load.");
        check(!model.isEditable(), "Unknown exposed value must lock the whole record.");
        check(model.bytes() == bytes, "Unknown incoming values must be retained exactly, not normalized.");
        check(model.value(invalid[0]) == invalid[1], "Unknown values should remain inspectable.");
        check(!model.setValue(17, 1), "No field may be edited in an unknown record.");
        check(model.bytes() == bytes, "Rejected edit must not alter the unknown record.");
        check(!error.isEmpty(), "Unknown values should report why editing is disabled.");
    }
}
} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    testPreservation();
    testRejectedEdits();
    testMalformedRecords();
    testUnknownValues();
    SettingsModel demo;
    check(demo.load(SettingsModel::demoBytes()), "Synthetic demo should be a recognized record.");
    if (failures == 0)
        qInfo() << "All settings preservation and validation tests passed.";
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
