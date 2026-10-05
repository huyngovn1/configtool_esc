#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "backend/escsession.h"
#include <QCloseEvent>
#include <QDateTime>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMessageBox>
#include <QSaveFile>
#include <QSerialPortInfo>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_session(new EscSession)
{
    ui->setupUi(this);
    ui->rawView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    ui->logView->setMaximumBlockCount(1000);
    ui->portCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    ui->portCombo->setMinimumContentsLength(18);
    m_session->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_session, &QObject::deleteLater);
    connect(m_session, &EscSession::busyChanged, this, [this](bool busy) {
        m_busy = busy;
        updateControls();
    });
    connect(m_session, &EscSession::connectedChanged, this, [this](bool connected) {
        m_connected = connected;
        ui->linkLabel->setText(connected ? tr("Đã nhận diện ESC") : tr("Chưa kết nối"));
        if (!connected) {
            m_loaded.clear();
            m_model = SettingsModel();
            m_dirty = false;
            ui->rawView->clear();
            ui->deviceLabel->setText(tr("Thiết bị: —     Firmware: —     EEPROM: —"));
            ui->settingsHint->setText(tr("Kết nối và đọc ESC để chỉnh thông số."));
        }
        updateControls();
    });
    connect(m_session, &EscSession::deviceInfo, this,
            [this](const QString &firmware, const QString &chip, quint32 address) {
        ui->deviceLabel->setText(tr("Thiết bị: %1     Firmware: %2     EEPROM: 0x%3")
            .arg(chip, firmware).arg(address, 4, 16, QLatin1Char('0')));
    });
    connect(m_session, &EscSession::settingsRead, this, &MainWindow::loadSettings);
    connect(m_session, &EscSession::logMessage, this, &MainWindow::appendLog);
    connect(m_session, &EscSession::errorOccurred, this, [this](const QString &error) {
        appendLog(tr("Lỗi: %1").arg(error));
        setStatus(tr("Lỗi: %1").arg(error));
    });
    connect(m_session, &EscSession::writeFinished, this, [this](bool ok, const QString &message) {
        if (ok) {
            m_loaded = m_model.bytes();
            m_dirty = false;
        }
        setStatus(message);
        appendLog(message);
        updateControls();
    });
    m_workerThread.start();

    connect(ui->refreshButton, &QPushButton::clicked, this, &MainWindow::refreshPorts);
    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::toggleConnection);
    connect(ui->demoButton, &QPushButton::clicked, this, &MainWindow::toggleDemo);
    connect(ui->readButton, &QPushButton::clicked, this, &MainWindow::readSettings);
    connect(ui->writeButton, &QPushButton::clicked, this, &MainWindow::writeSettings);
    connect(ui->exportButton, &QPushButton::clicked, this, &MainWindow::exportBackup);
    connect(ui->revertButton, &QPushButton::clicked, this, [this] { loadSettings(m_loaded); });
    connect(ui->clearLogButton, &QPushButton::clicked, ui->logView, &QPlainTextEdit::clear);
    connect(ui->modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this] { updateControls(); });
    connect(ui->pwmSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) { editValue(24, value); });
    connect(ui->startupSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) { editValue(25, value); });
    connect(ui->polesSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) { editValue(27, value); });
    connect(ui->beepSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
            [this](int value) { editValue(30, value); });
    connect(ui->brakeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [this](int value) { editValue(28, value); });
    connect(ui->reverseCheck, &QCheckBox::toggled, this,
            [this](bool value) { editValue(17, value); });
    connect(ui->bidirectionalCheck, &QCheckBox::toggled, this,
            [this](bool value) { editValue(18, value); });
    connect(ui->sineCheck, &QCheckBox::toggled, this,
            [this](bool value) { editValue(19, value); });
    refreshPorts();
    setStatus(tr("Sẵn sàng. Chọn cổng COM hoặc xem thử giao diện."));
    appendLog(tr("Basic ESC Config 0.1 — giao diện và dự án riêng."));
}

MainWindow::~MainWindow()
{
    QMetaObject::invokeMethod(m_session, "disconnectDevice", Qt::BlockingQueuedConnection);
    m_workerThread.quit();
    m_workerThread.wait();
    delete ui;
}

void MainWindow::refreshPorts()
{
    const QString previous = ui->portCombo->currentData().toString();
    ui->portCombo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto &port : ports) {
        QString label = port.portName();
        if (!port.description().isEmpty())
            label += QStringLiteral(" · ") + port.description();
        ui->portCombo->addItem(label, port.portName());
    }
    const int oldIndex = ui->portCombo->findData(previous);
    if (oldIndex >= 0)
        ui->portCombo->setCurrentIndex(oldIndex);
    if (ports.isEmpty())
        ui->portCombo->addItem(tr("Không có cổng COM"), QString());
    appendLog(tr("Tìm thấy %1 cổng serial.").arg(ports.size()));
    updateControls();
}

bool MainWindow::allowDiscard()
{
    if (!m_dirty || m_demo)
        return true;
    return QMessageBox::question(this, tr("Thay đổi chưa ghi"),
        tr("Bỏ các thay đổi chưa ghi lên ESC?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes;
}

void MainWindow::toggleConnection()
{
    if (m_busy || m_demo)
        return;
    if (m_connected) {
        if (!allowDiscard())
            return;
        m_busy = true;
        updateControls();
        setStatus(tr("Đang ngắt kết nối…"));
        QMetaObject::invokeMethod(m_session, "disconnectDevice", Qt::QueuedConnection);
        return;
    }
    const QString port = ui->portCombo->currentData().toString();
    if (port.isEmpty())
        return;
    m_busy = true;
    updateControls();
    setStatus(tr("Đang kết nối và đọc ESC…"));
    QMetaObject::invokeMethod(m_session, "connectDevice", Qt::QueuedConnection,
        Q_ARG(QString, port), Q_ARG(int, ui->modeCombo->currentIndex()),
        Q_ARG(int, ui->channelSpin->value() - 1));
}

void MainWindow::toggleDemo()
{
    if (m_connected || m_busy)
        return;
    m_demo = !m_demo;
    if (m_demo) {
        ui->deviceLabel->setText(tr("Thiết bị: DỮ LIỆU MẪU     Firmware: mô phỏng     Không kết nối phần cứng"));
        ui->linkLabel->setText(tr("Đang xem thử"));
        loadSettings(SettingsModel::demoBytes());
        appendLog(tr("Chế độ xem thử: thao tác chỉ áp dụng trong ứng dụng."));
    } else {
        m_model = SettingsModel();
        m_loaded.clear();
        m_dirty = false;
        ui->rawView->clear();
        ui->deviceLabel->setText(tr("Thiết bị: —     Firmware: —     EEPROM: —"));
        ui->linkLabel->setText(tr("Chưa kết nối"));
        ui->settingsHint->setText(tr("Kết nối và đọc ESC để chỉnh thông số."));
        setStatus(tr("Đã thoát chế độ xem thử."));
    }
    updateControls();
}

void MainWindow::readSettings()
{
    if (m_busy || !allowDiscard())
        return;
    if (m_demo) {
        loadSettings(SettingsModel::demoBytes());
        return;
    }
    if (!m_connected)
        return;
    m_busy = true;
    updateControls();
    setStatus(tr("Đang đọc cấu hình từ ESC…"));
    QMetaObject::invokeMethod(m_session, "readSettings", Qt::QueuedConnection);
}

void MainWindow::loadSettings(const QByteArray &data)
{
    // Copy before assignment: revert can pass m_loaded itself.
    const QByteArray incoming = data;
    m_loaded = incoming;
    m_dirty = false;
    QString error;
    m_loading = true;
    const bool valid = m_model.load(incoming, &error);
    if (valid) {
        ui->pwmSpin->setValue(m_model.value(24));
        ui->startupSpin->setValue(m_model.value(25));
        ui->polesSpin->setValue(m_model.value(27));
        ui->beepSpin->setValue(m_model.value(30));
        ui->brakeCombo->setCurrentIndex(m_model.value(28));
        ui->reverseCheck->setChecked(m_model.value(17) == 1);
        ui->bidirectionalCheck->setChecked(m_model.value(18) == 1);
        ui->sineCheck->setChecked(m_model.value(19) == 1);
        ui->settingsHint->setText(m_demo
            ? tr("XEM THỬ — dữ liệu mẫu để thiết kế giao diện; không ghi vào ESC.")
            : tr("Đã đọc cấu hình • EEPROM v%1 • Firmware %2.%3")
                .arg(m_model.value(1)).arg(m_model.value(3)).arg(m_model.value(4)));
        setStatus(m_demo ? tr("Đang xem thử giao diện.") : tr("Đã đọc cấu hình từ ESC."));
    } else {
        ui->settingsHint->setText(tr("Chỉ xem dữ liệu — %1").arg(error));
        appendLog(tr("Không cho phép chỉnh cấu hình này: %1").arg(error));
        setStatus(tr("Đã nhận dữ liệu; cấu trúc chưa hỗ trợ chỉnh sửa."));
    }
    m_loading = false;
    updateRawView();
    updateControls();
}

void MainWindow::editValue(int offset, int value)
{
    if (m_loading || m_busy || (!m_connected && !m_demo))
        return;
    QString error;
    if (!m_model.setValue(offset, value, &error)) {
        appendLog(tr("Thông số không hợp lệ: %1").arg(error));
        return;
    }
    m_dirty = m_model.bytes() != m_loaded;
    updateRawView();
    updateControls();
}

void MainWindow::writeSettings()
{
    if (m_busy || !m_model.isEditable() || !m_dirty)
        return;
    if (m_demo) {
        m_loaded = m_model.bytes();
        m_dirty = false;
        appendLog(tr("Đã áp dụng trong bản xem thử. Không gửi lệnh serial."));
        setStatus(tr("Đã áp dụng thay đổi cho dữ liệu mẫu."));
        updateControls();
        return;
    }
    if (!m_connected)
        return;
    m_busy = true;
    updateControls();
    setStatus(tr("Đang ghi cấu hình và đọc lại để kiểm tra…"));
    QMetaObject::invokeMethod(m_session, "writeSettings", Qt::QueuedConnection,
        Q_ARG(QByteArray, m_model.bytes()));
}

void MainWindow::exportBackup()
{
    if (m_loaded.size() != 48)
        return;
    const QString name = QStringLiteral("%1-%2.bin")
        .arg(m_demo ? QStringLiteral("DEMO") : QStringLiteral("esc-backup"),
             QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss")));
    const QString path = QFileDialog::getSaveFileName(this, tr("Sao lưu 48 byte đã đọc"),
        name, tr("EEPROM binary (*.bin)"));
    if (path.isEmpty())
        return;
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(m_loaded) != m_loaded.size()
        || !file.commit()) {
        QMessageBox::warning(this, tr("Không thể lưu"), file.errorString());
        return;
    }
    appendLog(tr("Đã sao lưu 48 byte: %1").arg(path));
    setStatus(tr("Đã lưu bản sao dữ liệu đã đọc."));
}

void MainWindow::updateRawView()
{
    const QByteArray bytes = m_model.bytes().isEmpty() ? m_loaded : m_model.bytes();
    QString text;
    for (int offset = 0; offset < bytes.size(); offset += 16) {
        text += QStringLiteral("%1   %2\n").arg(offset, 2, 16, QLatin1Char('0'))
            .arg(QString::fromLatin1(bytes.mid(offset, 16).toHex(' ').toUpper()));
    }
    ui->rawView->setPlainText(text);
}

void MainWindow::updateControls()
{
    const bool offline = !m_connected && !m_busy && !m_demo;
    const bool editable = !m_busy && (m_connected || m_demo) && m_model.isEditable();
    ui->portCombo->setEnabled(offline);
    ui->modeCombo->setEnabled(offline);
    ui->refreshButton->setEnabled(offline);
    ui->channelSpin->setEnabled(offline && ui->modeCombo->currentIndex() != 0);
    ui->connectButton->setEnabled(!m_busy && !m_demo &&
        (m_connected || !ui->portCombo->currentData().toString().isEmpty()));
    ui->connectButton->setText(m_busy ? tr("Đang xử lý…")
        : (m_connected ? tr("Ngắt kết nối") : tr("Kết nối")));
    ui->demoButton->setEnabled(!m_connected && !m_busy);
    ui->demoButton->setText(m_demo ? tr("Thoát xem thử") : tr("Xem thử giao diện"));
    ui->settingsPanel->setEnabled(editable);
    ui->readButton->setEnabled(!m_busy && (m_connected || m_demo));
    ui->readButton->setText(m_demo ? tr("Nạp lại mẫu") : tr("Đọc ESC"));
    ui->revertButton->setEnabled(editable && m_dirty);
    ui->exportButton->setEnabled(!m_busy && m_loaded.size() == 48);
    ui->writeButton->setEnabled(editable && m_dirty);
    ui->writeButton->setText(m_demo ? tr("Áp dụng bản thử") : tr("Ghi ESC"));
    ui->dirtyLabel->setText(m_loaded.isEmpty() ? tr("Chưa có dữ liệu")
        : (m_dirty ? tr("Có thay đổi chưa ghi")
        : (m_demo ? tr("Dữ liệu mẫu") : tr("Khớp bản đã đọc"))));
}

void MainWindow::appendLog(const QString &message)
{
    ui->logView->appendPlainText(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss  ")) + message);
}

void MainWindow::setStatus(const QString &message)
{
    statusBar()->showMessage(message);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_busy) {
        setStatus(tr("Đợi giao tiếp hiện tại hoàn tất rồi đóng ứng dụng."));
        event->ignore();
        return;
    }
    if (!allowDiscard()) {
        event->ignore();
        return;
    }
    event->accept();
}
