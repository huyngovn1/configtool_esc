#include "mainwindow.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Basic ESC Config"));
    app.setApplicationVersion(QStringLiteral("0.1.0"));
    app.setOrganizationName(QStringLiteral("HUY"));
    app.setFont(QFont(QStringLiteral("Segoe UI"), 10));
    MainWindow window;
    window.show();
    return app.exec();
}
