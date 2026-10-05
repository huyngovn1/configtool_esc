#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "model/settingsmodel.h"

namespace Ui { class MainWindow; }
class EscSession;
class QCloseEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void refreshPorts();
    void toggleConnection();
    void toggleDemo();
    void readSettings();
    void writeSettings();
    void exportBackup();
    void loadSettings(const QByteArray &data);
    void editValue(int offset, int value);
    void updateControls();
    void updateRawView();
    void appendLog(const QString &message);
    void setStatus(const QString &message);
    bool allowDiscard();

    Ui::MainWindow *ui;
    QThread m_workerThread;
    EscSession *m_session;
    SettingsModel m_model;
    QByteArray m_loaded;
    bool m_connected = false;
    bool m_busy = false;
    bool m_demo = false;
    bool m_loading = false;
    bool m_dirty = false;
};
#endif
