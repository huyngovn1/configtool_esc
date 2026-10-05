#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextStream>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    // Windows' offscreen platform does not enumerate installed fonts.
    // Register a system font for screenshots; production uses the native platform.
#ifdef Q_OS_WIN
    QFontDatabase::addApplicationFont(QString::fromLocal8Bit(qgetenv("WINDIR"))
                                     + QStringLiteral("/Fonts/segoeui.ttf"));
    QFontDatabase::addApplicationFont(QString::fromLocal8Bit(qgetenv("WINDIR"))
                                     + QStringLiteral("/Fonts/segoeuib.ttf"));
#endif
    app.setFont(QFont(QStringLiteral("Segoe UI"), 10));
    MainWindow window;
    window.show();
    app.processEvents();
    int failures = 0;
    auto check = [&](bool ok, const char *message) {
        if (!ok) {
            QTextStream(stderr) << "FAIL: " << message << '\n';
            ++failures;
        }
    };
    auto demo = window.findChild<QPushButton *>(QStringLiteral("demoButton"));
    auto write = window.findChild<QPushButton *>(QStringLiteral("writeButton"));
    auto revert = window.findChild<QPushButton *>(QStringLiteral("revertButton"));
    auto pwm = window.findChild<QSpinBox *>(QStringLiteral("pwmSpin"));
    auto raw = window.findChild<QPlainTextEdit *>(QStringLiteral("rawView"));
    check(demo && write && revert && pwm && raw, "UI named controls exist");
    if (failures)
        return 1;
    check(!pwm->isEnabled() && !write->isEnabled(), "offline editing disabled");
    demo->click();
    check(pwm->isEnabled() && pwm->value() == 24, "demo loads model");
    check(!write->isEnabled(), "unchanged settings cannot write");
    const QString original = raw->toPlainText();
    pwm->setValue(48);
    check(write->isEnabled() && revert->isEnabled(), "edited state enables actions");
    check(raw->toPlainText() != original, "raw preview follows edit");
    revert->click();
    check(pwm->value() == 24 && raw->toPlainText() == original, "revert restores exact bytes");
    check(!write->isEnabled(), "revert clears dirty state");
    pwm->setValue(32);
    write->click();
    check(!write->isEnabled() && pwm->value() == 32, "apply demo commits only sample");
    demo->click();
    check(!pwm->isEnabled() && raw->toPlainText().isEmpty(), "exit demo clears data");
    demo->click();
    app.processEvents();
    if (argc > 1)
        check(window.grab().save(QString::fromLocal8Bit(argv[1])), "save UI preview");
    QTextStream(stdout) << (failures ? "UI smoke failed\n" : "UI smoke passed\n");
    return failures ? 1 : 0;
}
