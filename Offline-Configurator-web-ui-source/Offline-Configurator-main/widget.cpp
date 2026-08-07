#include "widget.h"
#include "BF_ROOTLOADER.h"
#include "defaults.h"
#include "fourwayif.h"
#include "ui_widget.h"
//#include "bluejaymelody.h"
#include "music.h"

#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFrame>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QScrollBar>
#include <QtSerialPort/QSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>
#include <QTimer>
#include <QTimerEvent>
#include <QTextStream>
#include <QThread>

namespace {

void place(QWidget *widget, int x, int y, int width, int height) {
  if (widget) {
    widget->setGeometry(x, y, width, height);
  }
}

QLabel *makeLabel(QWidget *parent, const char *name, const QString &text,
                  const char *role = "") {
  auto *label = parent->findChild<QLabel *>(name);
  if (!label) {
    label = new QLabel(parent);
    label->setObjectName(name);
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
  }
  label->setText(text);
  if (*role) {
    label->setProperty("webRole", role);
  }
  return label;
}

QFrame *makeCard(QWidget *parent, const char *name) {
  auto *card = parent->findChild<QFrame *>(name);
  if (!card) {
    card = new QFrame(parent);
    card->setObjectName(name);
    card->setProperty("webCard", true);
    card->setFrameShape(QFrame::NoFrame);
    card->lower();
  }
  return card;
}

QPushButton *makeActionButton(QWidget *parent, const char *name,
                               const QString &text) {
  auto *button = parent->findChild<QPushButton *>(name);
  if (!button) {
    button = new QPushButton(parent);
    button->setObjectName(name);
    button->setProperty("toolbarAction", true);
  }
  button->setText(text);
  return button;
}

} // namespace

Widget::Widget(QWidget *parent)
    : QWidget(parent), ui(new Ui::Widget), four_way(new FourWayIF),
      RL(new BF_ROOTLOADER),
      //  msg_console(new OutConsole),
      m_serial(new QSerialPort(this)), input_buffer(new QByteArray),
      bluejay_tune(new QByteArray), eeprom_buffer(new QByteArray),
      music_buffer(new QByteArray) {
  ui->setupUi(this);
  //ui->tabWidget->removeTab(4); // todo make these visible
  ui->tabWidget->removeTab(5);   // remove led tab for now
  this->setWindowTitle("ESC Config Tool 1.95 - for firmware version 2.19 and higher");
  installWebSkin();

  serialInfoStuff();

  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(serialInfoStuff()));
  timer->start(2000);

  //  connect(m_serial, &QSerialPort::readyRead, this, &Widget::readData);  //
  //  an interrupt for reading serial

  //   connect(msg_console, &OutConsole::getData, this, &Widget::writeData);

  //  myButton->seCheckable(true);

  hide4wayButtons(true);
  hideESCSettings(true);
  hideEEPROMSettings(true);
  ui->writeBinary->setHidden(true);
  //ui->VerifyFlash->setHidden(true);
  //ui->MusicTextEdit->setHidden(false);
  //ui->uploadMusic->setHidden(false);

  ui->passthoughButton->setHidden(true);
  ui->endPassthrough->setHidden(true);
  // ui->devFrame->setHidden(true);
}

Widget::~Widget() { delete ui; }

void Widget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  QTimer::singleShot(0, this, [this] { arrangeWebLayout(); });
}

void Widget::installWebSkin() {
  setMinimumSize(940, 720);
  resize(1000, 800);

  ui->frame_2->setMinimumHeight(96);
  ui->frame_2->setMaximumHeight(96);
  ui->frame->setMinimumHeight(620);
  ui->frame->setMaximumHeight(620);
  ui->scrollAreaWidgetContents_5->setMinimumWidth(920);

  setStyleSheet(QStringLiteral(R"(
    QWidget#Widget {
      background: #f5f7fb;
      color: #11224a;
      font-family: "Segoe UI", "Arial";
      font-size: 12px;
    }
    QScrollArea { border: none; background: transparent; }
    QScrollArea > QWidget > QWidget { background: #f5f7fb; }
    QFrame#frame_2 {
      background: #101d42;
      border: 1px solid #243660;
      border-radius: 15px;
    }
    QFrame#frame { background: transparent; border: none; }
    QLabel[webRole="brandKicker"] {
      color: #65a8ff; font-size: 9px; font-weight: 700; letter-spacing: 1.5px;
    }
    QLabel[webRole="brandTitle"] { color: white; font-size: 20px; font-weight: 700; }
    QLabel[webRole="brandSubtitle"] { color: #d4e2ff; font-size: 10px; }
    QLabel#webLogo {
      background: #3478f6; color: white; border: 1px solid #5a96ff;
      border-radius: 11px; font-size: 16px; font-weight: 800;
    }
    QRadioButton#ConnectedButton {
      color: #ffffff; background: #21325d; border: 1px solid #425681;
      border-radius: 16px; padding: 7px 12px; font-size: 11px; font-weight: 600;
    }
    QRadioButton#ConnectedButton::indicator { width: 8px; height: 8px; border-radius: 4px; background: #8b9bb8; }
    QRadioButton#ConnectedButton::indicator:checked { background: #55d39f; }
    QLabel#escStatusLabel { color: #dbe8ff; font-size: 11px; }
    QFrame#webActionSurface {
      background: #ffffff; border: 1px solid #dce4ef; border-radius: 12px;
    }
    QFrame[webCard="true"] {
      background: #ffffff; border: 1px solid #dce4ef; border-radius: 11px;
    }
    QLabel[webRole="infoCaption"] {
      color: #65738e; font-size: 8px; font-weight: 700; letter-spacing: 1.2px;
    }
    QLabel[webRole="infoValue"] { color: #102349; font-size: 13px; font-weight: 700; }
    QLabel[webRole="cardTitle"] { color: #11234b; font-size: 13px; font-weight: 700; }
    QLabel#StatusLabel { color: #65738e; font-size: 10px; padding-left: 5px; }
    QPushButton {
      min-height: 28px; padding: 0 11px; color: #102349; background: #ffffff;
      border: 1px solid #cbd7e6; border-radius: 7px; font-weight: 600;
    }
    QPushButton:hover { background: #f3f7ff; border-color: #7da8fa; }
    QPushButton:pressed { background: #e6efff; }
    QPushButton#sendButton, QPushButton#writeEEPROM {
      background: #2563eb; border-color: #2563eb; color: #ffffff;
    }
    QPushButton#sendButton:hover, QPushButton#writeEEPROM:hover { background: #1d4ed8; }
    QPushButton#writeEEPROM:disabled { background: #a8c2f5; border-color: #a8c2f5; color: #ffffff; }
    QPushButton#webResetButton { color: #b42318; }
    QPushButton#webDisconnectButton { color: #b42318; padding: 0; font-size: 17px; }
    QComboBox, QLineEdit, QSpinBox {
      min-height: 26px; padding: 1px 8px; color: #11234b; background: #ffffff;
      border: 1px solid #cbd7e6; border-radius: 6px;
    }
    QComboBox::drop-down { border: none; width: 20px; }
    QLineEdit:focus, QComboBox:focus { border-color: #4285f4; }
    QCheckBox { color: #18315e; spacing: 8px; }
    QCheckBox::indicator {
      width: 14px; height: 14px; border: 1px solid #aebaca; border-radius: 3px; background: #ffffff;
    }
    QCheckBox::indicator:checked { background: #2563eb; border-color: #2563eb; }
    QTabWidget::pane { top: -1px; border: 1px solid #dce4ef; background: #ffffff; border-radius: 10px; }
    QTabBar::tab {
      min-width: 78px; padding: 9px 13px; margin-right: 2px; color: #344563;
      background: #e9eef6; border: none; border-top-left-radius: 8px; border-top-right-radius: 8px;
      font-weight: 600;
    }
    QTabBar::tab:selected { color: #2563eb; background: #ffffff; }
    QFrame#fourWayFrame, QFrame#eepromFrame, QFrame#inputservoFrame,
    QFrame#flashFourwayFrame, QFrame#flashMotorsFrame, QFrame#connectFrameInputPage,
    QFrame#tunesFrame { background: transparent; border: none; }
    QSlider::groove:horizontal { height: 5px; background: #d9e2ef; border-radius: 2px; }
    QSlider::sub-page:horizontal { background: #4c82f7; border-radius: 2px; }
    QSlider::handle:horizontal {
      width: 15px; margin: -5px 0; background: #ffffff; border: 2px solid #2563eb; border-radius: 7px;
    }
    QLCDNumber { color: #102c61; background: #f8faff; border: 1px solid #cbd7e6; border-radius: 6px; }
    QProgressBar { border: 1px solid #cbd7e6; border-radius: 6px; text-align: center; background: #f8faff; }
    QProgressBar::chunk { background: #2563eb; border-radius: 5px; }
  )"));

  makeLabel(ui->frame_2, "webLogo", "VP");
  makeLabel(ui->frame_2, "webKicker", "VPTEK", "brandKicker");
  makeLabel(ui->frame_2, "webTitle", "VPTEK ESC Control Center", "brandTitle");
  makeLabel(ui->frame_2, "webSubtitle", "ESC configuration and firmware management", "brandSubtitle");
  ui->ConnectedButton->setText("DEVICE   Not connected");
  connect(ui->ConnectedButton, &QRadioButton::toggled, this,
          [this](bool connected) {
            ui->ConnectedButton->setText(connected ? "DEVICE   Connected" : "DEVICE   Not connected");
          });

  makeCard(ui->frame, "webActionSurface");
  const char *captions[] = {"FIRMWARE", "EEPROM", "EEPROM ADDRESS", "FLASH CODE", "CONNECTION MODE"};
  const char *values[] = {"--", "--", "--", "--", "USB / Wi-Fi"};
  for (int index = 0; index < 5; ++index) {
    const QByteArray cardName = QByteArray("webInfoCard") + QByteArray::number(index);
    const QByteArray captionName = QByteArray("webInfoCaption") + QByteArray::number(index);
    const QByteArray valueName = QByteArray("webInfoValue") + QByteArray::number(index);
    makeCard(ui->frame, cardName.constData());
    makeLabel(ui->frame, captionName.constData(), QString::fromLatin1(captions[index]), "infoCaption");
    makeLabel(ui->frame, valueName.constData(), QString::fromLatin1(values[index]), "infoValue");
  }

  ui->sendButton->setText("Connect Device");
  ui->writeEEPROM->setText("Save Settings");
  ui->saveConfigButton->setText("Export Config");
  ui->loadConfigButton->setText("Import Config");
  ui->disconnectButton->setText("×");
  ui->disconnectButton->setObjectName("webDisconnectButton");
  ui->disconnectButton->setToolTip("Disconnect device");
  ui->checkBox_2->setText("USB / Wi-Fi");
  ui->OfflineCheckBox->setText("Offline mode");
  ui->label_31->setText("Throttle rate of change");
  ui->label_32->setText("Minimum duty cycle");
  ui->label_36->setText("Active brake power");
  ui->label_18->setText("Brake on stop level");
  ui->FirmwareNameLabel->hide();
  ui->FirmwareVerionsLabel->hide();
  ui->label_15->hide();
  ui->label_41->hide();

  ui->writeEEPROM->setParent(ui->frame);
  ui->saveConfigButton->setParent(ui->frame);
  ui->loadConfigButton->setParent(ui->frame);
  ui->configFileInfo->setParent(ui->frame);
  ui->configFileInfo->hide();

  auto *resetButton = makeActionButton(ui->frame, "webResetButton", "Reset ESC");
  auto *restoreButton = makeActionButton(ui->frame, "webRestoreButton", "Restore Defaults");
  connect(resetButton, &QPushButton::clicked, this, &Widget::resetESC);
  connect(restoreButton, &QPushButton::clicked, this, &Widget::on_sendFirstEEPROM_clicked);

  makeCard(ui->eepromFrame, "webFeaturesCard");
  makeCard(ui->eepromFrame, "webMotorCard");
  makeCard(ui->eepromFrame, "webDriveCard");
  makeLabel(ui->eepromFrame, "webFeaturesTitle", "ESC Features", "cardTitle");
  makeLabel(ui->eepromFrame, "webMotorTitle", "Motor Settings", "cardTitle");
  makeLabel(ui->eepromFrame, "webDriveTitle", "Ramp / PWM / Brake", "cardTitle");

  makeCard(ui->inputservoFrame, "webServoCard");
  makeCard(ui->inputservoFrame, "webPidCard");
  makeLabel(ui->inputservoFrame, "webServoTitle", "Servo Settings", "cardTitle");
  makeLabel(ui->inputservoFrame, "webPidTitle", "Current Limit PID Control", "cardTitle");

  QTimer::singleShot(0, this, [this] { arrangeWebLayout(); });
}

void Widget::arrangeWebLayout() {
  if (!ui || !ui->frame || !ui->frame_2) {
    return;
  }

  const int headerWidth = ui->frame_2->width();
  place(findChild<QLabel *>("webLogo"), 20, 25, 40, 46);
  place(findChild<QLabel *>("webKicker"), 72, 18, 180, 14);
  place(findChild<QLabel *>("webTitle"), 72, 33, 390, 26);
  place(findChild<QLabel *>("webSubtitle"), 72, 62, 350, 16);
  place(ui->escStatusLabel, 460, 63, qMax(150, headerWidth - 670), 16);
  place(ui->ConnectedButton, qMax(710, headerWidth - 184), 27, 166, 33);

  const int width = ui->frame->width();
  place(findChild<QFrame *>("webActionSurface"), 0, 0, width, 47);

  place(ui->sendButton, 10, 8, 108, 31);
  place(ui->writeEEPROM, 124, 8, 101, 31);
  place(findChild<QPushButton *>("webResetButton"), 231, 8, 85, 31);
  place(ui->saveConfigButton, 322, 8, 97, 31);
  place(ui->loadConfigButton, 425, 8, 97, 31);
  place(findChild<QPushButton *>("webRestoreButton"), 528, 8, 127, 31);
  place(ui->disconnectButton, 661, 8, 31, 31);
  place(ui->serialSelectorBox, qMax(700, width - 242), 8, 120, 31);
  place(ui->checkBox_2, qMax(826, width - 116), 9, 108, 17);
  place(ui->OfflineCheckBox, qMax(826, width - 116), 25, 108, 17);

  const int cardY = 55;
  const int side = 2;
  const int gap = 8;
  const int cardWidth = (width - (side * 2) - (gap * 4)) / 5;
  for (int index = 0; index < 5; ++index) {
    const int x = side + index * (cardWidth + gap);
    const QByteArray cardName = QByteArray("webInfoCard") + QByteArray::number(index);
    const QByteArray captionName = QByteArray("webInfoCaption") + QByteArray::number(index);
    const QByteArray valueName = QByteArray("webInfoValue") + QByteArray::number(index);
    place(findChild<QFrame *>(cardName.constData()), x, cardY, cardWidth, 60);
    place(findChild<QLabel *>(captionName.constData()), x + 11, cardY + 11, cardWidth - 22, 12);
    place(findChild<QLabel *>(valueName.constData()), x + 11, cardY + 29, cardWidth - 22, 20);
  }
  place(ui->StatusLabel, 5, 119, width - 10, 18);
  place(ui->tabWidget, 0, 142, width, 468);

  const int tabWidth = qMax(840, ui->tab->width());
  place(ui->fourWayFrame, 7, 6, tabWidth - 14, 416);
  place(ui->connectMotor, 14, 9, 175, 27);
  place(ui->initMotor1, 194, 9, 38, 27);
  place(ui->initMotor2, 236, 9, 38, 27);
  place(ui->initMotor3, 278, 9, 38, 27);
  place(ui->initMotor4, 320, 9, 38, 27);
  place(ui->FirmwareNameLabel, 380, 9, 170, 27);
  place(ui->FirmwareVerionsLabel, 555, 9, 130, 27);
  place(ui->eepromFrame, 0, 43, tabWidth - 14, 365);

  const int eepromWidth = qMax(826, ui->eepromFrame->width());
  const int panelGap = 11;
  const int panelSide = 8;
  const int usable = eepromWidth - panelSide * 2 - panelGap * 2;
  const int featureWidth = usable * 28 / 100;
  const int motorWidth = usable * 36 / 100;
  const int driveWidth = usable - featureWidth - motorWidth;
  const int featureX = panelSide;
  const int motorX = featureX + featureWidth + panelGap;
  const int driveX = motorX + motorWidth + panelGap;

  place(findChild<QFrame *>("webFeaturesCard"), featureX, 3, featureWidth, 356);
  place(findChild<QFrame *>("webMotorCard"), motorX, 3, motorWidth, 356);
  place(findChild<QFrame *>("webDriveCard"), driveX, 3, driveWidth, 356);
  place(findChild<QLabel *>("webFeaturesTitle"), featureX + 13, 16, featureWidth - 26, 20);
  place(findChild<QLabel *>("webMotorTitle"), motorX + 13, 16, motorWidth - 26, 20);
  place(findChild<QLabel *>("webDriveTitle"), driveX + 13, 16, driveWidth - 26, 20);

  place(ui->comp_pwmCheckbox, featureX + 16, 52, featureWidth - 32, 21);
  place(ui->stuckProtectionBox, featureX + 16, 81, featureWidth - 32, 21);
  place(ui->antiStallBox, featureX + 16, 110, featureWidth - 32, 21);
  place(ui->rvCheckBox, featureX + 16, 139, featureWidth - 32, 21);
  place(ui->biDirectionCheckbox, featureX + 16, 168, featureWidth - 32, 21);
  place(ui->sinCheckBox, featureX + 16, 197, featureWidth - 32, 21);
  place(ui->thirtymsTelemBox, featureX + 16, 226, featureWidth - 32, 21);
  place(ui->hallSensorCheckbox, featureX + 16, 255, featureWidth - 32, 21);
  place(ui->disableStickCalibCheckbox, featureX + 16, 284, featureWidth - 32, 21);

  const int motorLabelX = motorX + 14;
  const int motorControlX = motorX + 118;
  const int motorControlWidth = qMax(74, motorWidth - 183);
  const int motorValueX = motorControlX + motorControlWidth + 7;
  const int motorRows[] = {49, 80, 111, 142, 173, 204, 235, 266, 297};
  place(ui->timingLabel, motorLabelX, motorRows[0], 98, 24);
  place(ui->timingAdvanceSlider, motorControlX, motorRows[0], motorControlWidth, 24);
  place(ui->timingAdvanceLCD, motorValueX, motorRows[0], 50, 24);
  place(ui->label_5, motorLabelX, motorRows[1], 98, 24);
  place(ui->motorKVSlider, motorControlX, motorRows[1], motorControlWidth, 24);
  place(ui->motorKVLCD, motorValueX, motorRows[1], 50, 24);
  place(ui->label_4, motorLabelX, motorRows[2], 98, 24);
  place(ui->motorPolesSlider, motorControlX, motorRows[2], motorControlWidth, 24);
  place(ui->motorPolesLCD, motorValueX, motorRows[2], 50, 24);
  place(ui->label, motorLabelX, motorRows[3], 98, 24);
  place(ui->startupPowerSlider, motorControlX, motorRows[3], motorControlWidth, 24);
  place(ui->startupPowerLCD, motorValueX, motorRows[3], 50, 24);
  place(ui->label_3, motorLabelX, motorRows[4], 98, 24);
  place(ui->pwmFreqSlider, motorControlX, motorRows[4], motorControlWidth, 24);
  place(ui->pwmFreqLCD, motorValueX, motorRows[4], 50, 24);
  place(ui->label_13, motorLabelX, motorRows[5], 98, 24);
  place(ui->beepVolumeSlider, motorControlX, motorRows[5], motorControlWidth, 24);
  place(ui->beepVolumeLCD, motorValueX, motorRows[5], 50, 24);
  place(ui->label_19, motorLabelX, motorRows[6], 98, 24);
  place(ui->sineStartupSlider, motorControlX, motorRows[6], motorControlWidth, 24);
  place(ui->sineRangeLCD, motorValueX, motorRows[6], 50, 24);
  place(ui->label_20, motorLabelX, motorRows[7], 98, 24);
  place(ui->sineModePowerSlider, motorControlX, motorRows[7], motorControlWidth, 24);
  place(ui->sineLcd, motorValueX, motorRows[7], 50, 24);
  place(ui->label_21, motorLabelX, motorRows[8], 98, 24);
  place(ui->runningBrakeStrength, motorControlX, motorRows[8], motorControlWidth, 24);
  place(ui->runningBrakeLcd, motorValueX, motorRows[8], 50, 24);

  const int driveLabelX = driveX + 14;
  const int driveControlX = driveX + 138;
  const int driveControlWidth = qMax(54, driveWidth - 196);
  const int driveValueX = driveControlX + driveControlWidth + 7;
  place(ui->label_31, driveLabelX, 49, 118, 24);
  place(ui->maxRocSlider, driveControlX, 49, driveControlWidth, 24);
  place(ui->rocLineEdit, driveValueX, 49, 50, 24);
  place(ui->label_32, driveLabelX, 80, 118, 24);
  place(ui->minDutySlider, driveControlX, 80, driveControlWidth, 24);
  place(ui->minDutyLineEdit, driveValueX, 80, 50, 24);
  place(ui->AutoTimingButton, driveLabelX, 116, driveWidth - 28, 21);
  place(ui->varPWMCheckBox, driveLabelX, 145, driveWidth - 28, 21);
  place(ui->autoPWM, driveLabelX, 174, driveWidth - 28, 21);
  place(ui->brakecheckbox, driveLabelX, 203, driveWidth - 28, 21);
  place(ui->activeBrakeCheckbox, driveLabelX, 232, driveWidth - 28, 21);
  place(ui->label_36, driveLabelX, 267, 118, 24);
  place(ui->activeBrakeSlider, driveControlX, 267, driveControlWidth, 24);
  place(ui->activeBrakeLineEdit, driveValueX, 267, 50, 24);
  place(ui->signalComboBox, driveLabelX, 303, 136, 24);
  place(ui->label_25, driveLabelX + 142, 303, driveWidth - 156, 24);
  place(ui->dragBrakeSlider, driveControlX, 333, driveControlWidth, 22);
  place(ui->dragBrakeLCD, driveValueX, 330, 50, 24);
  place(ui->label_18, driveLabelX, 333, 120, 22);
  ui->label_40->hide();

  const int inputWidth = qMax(820, ui->tab_4->width());
  place(ui->connectFrameInputPage, 10, 6, 315, 30);
  place(ui->inputservoFrame, 7, 42, inputWidth - 14, 378);
  const int inputFrameWidth = qMax(800, ui->inputservoFrame->width());
  const int servoWidth = (inputFrameWidth - 28) * 49 / 100;
  const int pidX = servoWidth + 20;
  place(findChild<QFrame *>("webServoCard"), 3, 3, servoWidth, 371);
  place(findChild<QFrame *>("webPidCard"), pidX, 3, inputFrameWidth - pidX - 3, 371);
  place(findChild<QLabel *>("webServoTitle"), 16, 16, servoWidth - 32, 20);
  place(findChild<QLabel *>("webPidTitle"), pidX + 13, 16, inputFrameWidth - pidX - 26, 20);

  const int servoLabelX = 16;
  const int servoControlX = 180;
  const int servoControlWidth = qMax(86, servoWidth - 250);
  const int servoValueX = servoControlX + servoControlWidth + 8;
  place(ui->label_10, servoLabelX, 52, 136, 24);
  place(ui->servoLowSlider, servoControlX, 52, servoControlWidth, 24);
  place(ui->lowThresholdLineEdit, servoValueX, 52, 54, 24);
  place(ui->label_11, servoLabelX, 83, 136, 24);
  place(ui->servoHighSlider, servoControlX, 83, servoControlWidth, 24);
  place(ui->highThresholdLineEdit, servoValueX, 83, 54, 24);
  place(ui->label_12, servoLabelX, 114, 136, 24);
  place(ui->servoNeutralSlider, servoControlX, 114, servoControlWidth, 24);
  place(ui->servoNeuralLineEdit, servoValueX, 114, 54, 24);
  place(ui->label_14, servoLabelX, 145, 136, 24);
  place(ui->servoDeadBandSlider, servoControlX, 145, servoControlWidth, 24);
  place(ui->servoDeadbandLineEdit, servoValueX, 145, 54, 24);
  place(ui->absoluteVotlageCheckbox, servoLabelX, 190, 155, 23);
  place(ui->absoluteVoltageSlider, servoControlX, 190, servoControlWidth, 24);
  place(ui->absoluateVoltageLineedit, servoValueX, 190, 54, 24);
  place(ui->lowVoltageCuttoffBox, servoLabelX, 224, 155, 23);
  place(ui->lowVoltageThresholdSlider, servoControlX, 224, servoControlWidth, 24);
  place(ui->lowVoltageLineEdit, servoValueX, 224, 54, 24);
  place(ui->label_16, servoLabelX, 258, 155, 24);
  place(ui->temperatureSlider, servoControlX, 258, servoControlWidth, 24);
  place(ui->temperatureLineEdit, servoValueX, 258, 54, 24);
  place(ui->label_22, servoLabelX, 292, 155, 24);
  place(ui->currentSlider, servoControlX, 292, servoControlWidth, 24);
  place(ui->currentLineEdit, servoValueX, 292, 54, 24);
  place(ui->rcCarReverse, servoLabelX, 329, servoWidth - 32, 22);
  ui->writeEEPROM_2->hide();

  const int pidLabelX = pidX + 16;
  const int pidControlX = pidX + 175;
  const int pidControlWidth = qMax(75, inputFrameWidth - pidControlX - 25);
  place(ui->label_37, pidLabelX, 52, 145, 24);
  place(ui->currentLimitPedit, pidControlX, 52, pidControlWidth, 24);
  place(ui->label_38, pidLabelX, 83, 145, 24);
  place(ui->currentLimitIedit, pidControlX, 83, pidControlWidth, 24);
  place(ui->label_39, pidLabelX, 114, 145, 24);
  place(ui->currentLimitDedit, pidControlX, 114, pidControlWidth, 24);
}

void Widget::on_sendButton_clicked() { connectSerial(); }

void Widget::closeSerialPort() {
  if (m_serial->isOpen())
    m_serial->close();
  showStatusMessage(tr("Disconnected"));
}

void Widget::loadBinFile() {
  filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                          "c:", tr("All Files (*.*)"));
  // ui->textEdit->setPlainText(filename);
  ui->writeBinary->setHidden(false);
  // ui->VerifyFlash->setHidden(false);
}

void Widget::showStatusMessage(const QString &message) {
  ui->label_2->setText(message);
}

void Widget::serialInfoStuff() {

  if (m_serial->isOpen()) {
    return;
  }

  // qInfo("called serial info");
  const auto infos = QSerialPortInfo::availablePorts();
  //   qInfo("number of ports : %d ", infos.size());

  if (infos.size() == number_of_ports) {
    return;
  }
  number_of_ports = infos.size();

  ui->serialSelectorBox->clear();
  ui->serialSelectorBox->addItem("Select Port");

  QString s;

  for (const QSerialPortInfo &info :
       infos) { // here we should add to drop down menu

    ui->serialSelectorBox->addItem(info.portName());
    //  m_serial->setPortName(info.portName());
    s = s + QObject::tr("Port: ") + info.portName() + "\n" +
        QObject::tr("Location: ") + info.systemLocation() + "\n" +
        QObject::tr("Description: ") + info.description() + "\n" +
        QObject::tr("Manufacturer: ") + info.manufacturer() + "\n" +
        QObject::tr("Serial number: ") + info.serialNumber() + "\n" +
        QObject::tr("Vendor Identifier: ") +
        (info.hasVendorIdentifier()
             ? QString::number(info.vendorIdentifier(), 16)
             : QString()) +
        "\n" + QObject::tr("Product Identifier: ") +
        (info.hasProductIdentifier()
             ? QString::number(info.productIdentifier(), 16)
             : QString()) +
        "\n";
  }
  // ui->textEdit->setPlainText(s);
}

void Widget::connectSerial() {
  if (ui->checkBox_2->isChecked()) {
    four_way->direct = true;
    m_serial->setBaudRate(m_serial->Baud19200);
    if (ui->tabWidget->count() == 4) {
      ui->tabWidget->removeTab(2);
      showSingleMotor(true);
    }

  } else {
    ui->tabWidget->insertTab(2, ui->tab_3, "Motor Control");
    showSingleMotor(false);
    four_way->direct = false;
    m_serial->setBaudRate(m_serial->Baud115200);
  }

  m_serial->setPortName(ui->serialSelectorBox->currentText());

  m_serial->setDataBits(m_serial->Data8);
  m_serial->setParity(m_serial->NoParity);
  m_serial->setStopBits(m_serial->OneStop);
  m_serial->setFlowControl(m_serial->NoFlowControl);

  if (m_serial->open(QIODevice::ReadWrite)) {
    ui->ConnectedButton->setCheckable(true);
    ui->ConnectedButton->setChecked(true);
    ui->escStatusLabel->setText("Select Motor");
    showStatusMessage(tr("Connected to %1 : %2, %3, %4, %5, %6")
                          .arg(m_serial->portName())
                          .arg(m_serial->dataBits())
                          .arg(m_serial->baudRate())
                          .arg(m_serial->parity())
                          .arg(m_serial->stopBits())
                          .arg(m_serial->flowControl()));

    hide4wayButtons(false);
    QByteArray passthroughenable2; // payload  empty here
    four_way->passthrough_started = true;

    four_way->ack_required = false;
    if (four_way->direct == false) {
      parseMSPMessage = true;
      send_mspCommand(0x68, passthroughenable2);
      m_serial->waitForBytesWritten(100);
      while (m_serial->waitForReadyRead(100)) {
      }
      readData();
      send_mspCommand(0xf5, passthroughenable2);
      m_serial->waitForBytesWritten(100);
    }

  } else {
    QMessageBox::critical(this, tr("Error"), m_serial->errorString());

    showStatusMessage(tr("Open error"));
  }
}

void Widget::on_disconnectButton_clicked() {
  if (m_serial->isOpen()) {
    hide4wayButtons(true);
    hideESCSettings(true);
    hideEEPROMSettings(true);
    writeData(four_way->makeFourWayCommand(0x34, 0x00));
    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(500)) {
      QByteArray data = m_serial->readAll();
    }
    send_mspCommand(
        0x44,
        0x00); // reset the FC, otherwise it can hold the last throttle value;
    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(500)) {
      QByteArray data = m_serial->readAll();
    }
    parseMSPMessage = true;
    readData();

    four_way->passthrough_started = false;
    ui->ConnectedButton->setChecked(false);
    ui->ConnectedButton->setCheckable(false);
    allup();
  }
  closeSerialPort();
}

void Widget::readInitData() {
  QByteArray data = m_serial->readAll();
  if(data.size() != 0){
  qInfo("read data size next");
  if (data.size() > 21) {
    data.remove(0, 21);
  }

  if (data[8] == (char)0x30) {
    if (data[4] == (char)0x2b) {
      qInfo("G071ESC_2KB_PAGE");
      four_way->memory_divider_required_four = true;
      four_way->eeprom_address =
          0x7e00; // this equals an eeprom address of 0x1f800 126kb
    }
    if (data[4] == (char)0x1f) {
      qInfo("F0ESC_1KB_PAGE");
      four_way->memory_divider_required_four = false;
      four_way->eeprom_address = 0x7c00; //  eeprom address of 0x7c00 31kb
    }
    if (data[4] == (char)0x35) {
      qInfo("F3ESC_2KB_PAGE");
      four_way->memory_divider_required_four = false;
      four_way->eeprom_address = 0xF800; // eeprom address of 0xf800 62kb
    }
    ui->escStatusLabel->setText("Connected");
    four_way->ESC_connected = true;
    hideESCSettings(false);
  } else {
    hideESCSettings(true);
    four_way->ESC_connected = false;
  }
}
}
void Widget::readData() {
qInfo("reading");
  QByteArray data = m_serial->readAll();

if(data.size() != 0){
  if (four_way->passthrough_started) {
    qInfo("passthrough started");
  }

  if (!four_way->passthrough_started) {
    qInfo("no-passthrough started");
  }

  //   qInfo("size of data : %d ", data.size());
  //   QByteArray data_hex_string = data.toHex();
  if (four_way->passthrough_started) {

    if (four_way->ack_required == true) {

      if (data.size() < 3) {
        ui->StatusLabel->setText("No Response From ESC");
        return;
      }
      if (four_way->checkCRC(data, data.size())) {

        if (data[data.size() - 3] == (char)0x00) { // ACK OK!!
          four_way->ack_required = false;
          four_way->ack_type = ACK_OK;

           qInfo("line 271");


          ui->StatusLabel->setText("GOOD ACK FROM IF");
          if (data[1] == (char)0x3a) {
            //  if verifying flash

            input_buffer->clear();
            for (int i = 0; i < (uint8_t)data[4]; i++) {
              input_buffer->append(
                  data[i + 5]); // first 4 byte are package header
            }
            qInfo("GOOD ACK FROM ESC -- read");
            hideESCSettings(false);
            hideEEPROMSettings(false);
            //     QApplication::processEvents();
          }
          if (data[1] == (char)0x3b) {

            qInfo("GOOD ACK FROM ESC -- WRITE");
          }

          if (data[1] == (char)0x37) {
            hideESCSettings(false);
            // qInfo("ID 6: %d",data[6]);
            if (data[6] == (char)0x2b) {
              qInfo("G071ESC_2KB_PAGE");
              four_way->memory_divider_required_four = true;
              four_way->eeprom_address =
                  0x7e00; // this equals an eeprom address of 0x1f800 126kb
              four_way->firmware_start = 4096;
            }
            if (data[6] == (char)0x1f) {
              qInfo("F0531ESC_1KB_PAGE");
              four_way->memory_divider_required_four = false;
              four_way->eeprom_address =
                  0x7c00; //  eeprom address of 0x7c00 31kb
              four_way->firmware_start = 4096;
            }
            if (data[6] == (char)0x35) {
              qInfo("F3ESC_2KB_PAGE");
              four_way->memory_divider_required_four = false;
              four_way->eeprom_address =
                  0xF800; // eeprom address of 0x7c00 62kb
              four_way->firmware_start = 4096;
            }
            if (data[6] == (char)0x15) {
                qInfo("NXP ESC_8KB_PAGE");
                four_way->memory_divider_required_four = false;
                four_way->eeprom_address =
                    0xE000; // eeprom address of 64k-8k
                four_way->firmware_start = 16384;
            }
            ui->escStatusLabel->setText("Connected");
            four_way->ESC_connected = true;
          }
        } else { // bad ack
          qInfo("line 319");
          if (data[1] == (char)0x37) {
            hideESCSettings(true);
            four_way->ESC_connected = false;
          }
          if (data[1] == (char)0x3b) {

            qInfo("BAD ACK FROM ESC -- WRITE");
          }
         // hideEEPROMSettings(true);

          qInfo("BAD OR NO ACK FROM ESC");
          ui->StatusLabel->setText("BAD OR NO ACK FROM IF");
          four_way->ack_type = BAD_ACK;
          // four_way->ack_required = false;
        }
      } else {
        qInfo("4WAY CRC ERROR");
        ui->StatusLabel->setText("BAD OR NO ACK FROM ESC");
        four_way->ack_type = CRC_ERROR;
      }
    } else {
      qInfo("no ack required");
    }
  }
}

  if (parseMSPMessage) {
    parseMSPMessage = false;
    ////        if(data.size() == 59){
    ////        if(data[0] == 0x24 && data[2] == 0x3e){
    ////          rpm =  (uint8_t)data[6] | (uint8_t(data[7])<<8);
    ////          qInfo("RPM : %d ", rpm);
    ////        }
    ////        }
    //        if(data.size() == 22){
    //        if(data[0] == char(0x24) && data[2] == char(0x3e)){
    //          motor1throttle =  (uint8_t)data[5] | (uint8_t(data[6])<<8);
    //          motor2throttle =  (uint8_t)data[7] | (uint8_t(data[8])<<8);
    //          motor3throttle =  (uint8_t)data[9] | (uint8_t(data[10])<<8);
    //          motor4throttle =  (uint8_t)data[11] | (uint8_t(data[12])<<8);

    //          qInfo("Throttle 1 : %d ", motor1throttle);
    //          qInfo("Throttle 2 : %d ", motor2throttle);
    //          qInfo("Throttle 3: %d ", motor3throttle);
    //          qInfo("Throttle 4 : %d ", motor4throttle);

    //          ui->horizontalSlider->setValue(motor1throttle);
    //          ui->m1MSPSlider->setValue(motor1throttle);
    //          ui->m2MSPSlider->setValue(motor2throttle);
    //          ui->m3MSPSlider->setValue(motor3throttle);
    //          ui->m4MSPSlider->setValue(motor4throttle);
    //          QString s = QString::number(motor1throttle);
    //          ui->lineEdit->setText(s);
    //        }
    //        }
  }
}

void Widget::writeData(const QByteArray &data) { m_serial->write(data); }

void Widget::on_sendMessageButton_clicked() {
  // const QByteArray data = ui->plainTextEdit->toPlainText().toLocal8Bit();
  // writeData(data);
}

uint8_t Widget::mspSerialChecksumBuf(uint8_t checksum, const uint8_t *data,
                                     int len) {
  while (len-- > 0) {
    checksum ^= *data++;
  }
  return checksum;
}

void Widget::on_pushButton_clicked() {

  four_way->ack_required = true;
  writeData(four_way->makeFourWayCommand(0x3f, 0x04));
}

void Widget::on_pushButton_2_clicked() {
  four_way->ack_required = true;
  writeData(four_way->makeFourWayCommand(0x37, 0x00));
}

void Widget::on_passthoughButton_clicked() {
  hide4wayButtons(false);
  QByteArray passthroughenable2; // payload  empty here
  four_way->passthrough_started = true;
  parseMSPMessage = false;
  send_mspCommand(0xf5, passthroughenable2);
}

void Widget::on_horizontalSlider_sliderMoved(int position) {

  char highByteThrottle = (position >> 8) & 0xff;
  ;
  char lowByteThrottle = position & 0xff;

  QString s = QString::number(position);
  ui->lineEdit->setText(s);
  //    24 4d 3c 10 d6 d0 07 d0 07 d0 07 d0 07 00 00 00 00 00 00 00 00 c6
  QByteArray sliderThrottle;
  sliderThrottle.append((char)lowByteThrottle);  // motor 1
  sliderThrottle.append((char)highByteThrottle); //
  sliderThrottle.append((char)lowByteThrottle);  // motor 2
  sliderThrottle.append((char)highByteThrottle);
  sliderThrottle.append((char)lowByteThrottle);
  sliderThrottle.append((char)highByteThrottle);
  sliderThrottle.append((char)lowByteThrottle);
  sliderThrottle.append((char)highByteThrottle);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);

  if (ui->checkBox->isChecked()) {

    send_mspCommand(0xd6, sliderThrottle);
    m_serial->waitForBytesWritten(200);
  }
}

void Widget::on_serialSelectorBox_currentTextChanged(const QString &arg1) {}

void Widget::send_mspCommand(uint8_t cmd, QByteArray payload) {
  QByteArray mspMsgOut;
  mspMsgOut.append((char)0x24);
  mspMsgOut.append((char)0x4d);
  mspMsgOut.append((char)0x3c);
  mspMsgOut.append((char)payload.length());
  mspMsgOut.append((char)cmd);
  if (payload.length() > 0) {
    mspMsgOut.append(payload);
  }

  uint8_t checksum = 0;
  for (int i = 3; i < mspMsgOut.length(); i++) {
    checksum ^= mspMsgOut[i];
  }
  mspMsgOut.append((char)checksum);
  writeData(mspMsgOut);
}

QByteArray Widget::convertFromHex() {
  QFile inputHex(filename);
  uint16_t last_address = 0;
  uint16_t last_size = 0;

  QByteArray rawData;
  if (inputHex.open(QIODevice::ReadOnly)) {
    QTextStream in(&inputHex);
    while (!in.atEnd()) {
      QString line = in.readLine();
      QByteArray lineArray;
      uint16_t crc = 0;

      for (int i = 1; i < line.size(); i = i + 2) {

        QString word = line.at(i);
        word.append(line.at(i + 1));
        uint16_t num = word.toLongLong(nullptr, 16);
        crc = crc + num;
        //           qInfo("byte: %d", num);
        lineArray.append(num);
      }
      qInfo("crc line %d", crc);
      if (crc % 256) {
        qInfo("crc error");
        ui->StatusLabel->setText("CRC ERROR IN HEX FILE!");
        break;
      }

      // 0 size
      // 1 address high
      // 2 adress low
      // 3 data type
      uint16_t data_type = (char)lineArray.at(3);

      if (data_type == (char)0x00) { // data

        uint16_t address = (((uint8_t)lineArray.at(1) << 8) & 0xffff) |
                           ((uint8_t)lineArray.at(2) & 0xff);

        //  qInfo("address: %d", address);

        //   qInfo("last size: %d", last_size);

        if ((address - last_address > last_size) && (last_size > 0)) {
          for (int i = 0; i < address - last_address - last_size; i++) {
            rawData.append(char(0x00));
          }
        }
        last_address = address;
        last_size = (char)lineArray.at(0);

        lineArray.remove(0, 4);
        lineArray.chop(1);
        rawData.append(lineArray);
      }
    }
    inputHex.close();
  }
//QFileDialog::saveFileContent(rawData, "raw.bin");
  return rawData;

}

void Widget::resetESC() {
  if (four_way->direct) {
    QByteArray reset;
    reset.append(char(0x00));
    reset.append(char(0x00));
    reset.append(char(0x00));
    reset.append(char(0x00));
    writeData(reset);
  } else {
    writeData(four_way->makeFourWayCommand(0x35, four_way->connected_motor));
  }
}

void Widget::on_loadBinary_clicked() { loadBinFile(); }

void Widget::on_writeBinary_clicked() {
  uint16_t chunk_size = 128;

  if (four_way->ESC_connected == false) {
    ui->StatusLabel->setText("NOT CONNECTED");
    return;
  }

  four_way->ack_required = true;
  if(eeprom_buffer->size() != 0){
  QByteArray eeprom_out;
  for (int i = 0; i < 48; i++) {
    eeprom_out.append(eeprom_buffer->at(i));
  }
  eeprom_out[0] = 0x00;

  if (four_way->direct) {

    sendDirect(eeprom_out, 48, four_way->eeprom_address);
    chunk_size = 128;
  } else {
    writeData(four_way->makeFourWayWriteCommand(eeprom_out, 48,
                                                four_way->eeprom_address));
    chunk_size = 256;
    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(1000)) {
    }

    readData();
  }
  if (four_way->ack_required == false) { // good ack received from esc
    ui->escStatusLabel->setText("WRITE EEPROM SUCCESSFUL");
  } else {
    ui->escStatusLabel->setText("Unable to set safety bit");
    return;
  }
}
  QFileInfo fileInfo(filename);
  QByteArray line;
  QString ext = fileInfo.suffix(); // ext = "tar.gz"

  if (ext == "hex") {
    qInfo("hex");
    line = convertFromHex();

  } else if (ext == "bin") {
    QFile inputFile(filename);
    qInfo("bin");
    inputFile.open(QIODevice::ReadOnly);
    line = inputFile.readAll();
    //       qInfo("size of original: %d", line.size());
    inputFile.close();
  } else {

    ui->StatusLabel->setText("NOT A VALID FILE");
    ui->escStatusLabel->setText("Select a .bin or .hex file");
    return;
  }

  uint32_t sizeofBin = line.size();
  uint16_t index = 0;
  ui->progressBar->setValue(0);
  uint8_t pages = sizeofBin / 2048;
  //    uint8_t bytes_in_last_page = sizeofBin % 1024;
  uint8_t max_retries = 8;
  uint8_t retries = 0;

  for (int i = 0; i <= pages;
       i++) { // for each page ( including partial page at end)

    for (int j = 0; j < 2048 / chunk_size; j++) { // 8 or 16 buffers per page
      QByteArray onetwentyeight;
      // for debugging limit to 50
      for (int k = 0; k < chunk_size; k++) { // transfer 256 bytes each buffer
        onetwentyeight.append(line.at(k + (i * 2048) + (j * chunk_size)));
        index++;
        if (index >= sizeofBin) {
          break;
        }
      }
      four_way->ack_required = true;
      // four_way->ack_received = false;
      retries = 0;
      while (four_way->ack_required) {
        if (four_way->memory_divider_required_four) {
          if (four_way->direct) {
            sendDirect(onetwentyeight, onetwentyeight.size(),
                       (four_way->firmware_start + (i * 2048) + (j * chunk_size)) >> 2);
          } else {
            writeData(four_way->makeFourWayWriteCommand(
                onetwentyeight, onetwentyeight.size(),
                (four_way->firmware_start + (i * 2048) + (j * chunk_size)) >> 2));
          }
        } else {
          if (four_way->direct) {
            sendDirect(onetwentyeight, onetwentyeight.size(),
                       (four_way->firmware_start + (i * 2048) + (j * chunk_size)));
          } else {
            qInfo("adress: %d",
                  four_way->firmware_start + (i * 2048) + (j * chunk_size));
            writeData(four_way->makeFourWayWriteCommand(
                onetwentyeight, onetwentyeight.size(),
                four_way->firmware_start + (i * 2048) +
                    (j * chunk_size))); // increment address every i and j
          }
        }

        if (!four_way->direct) {
          while(m_serial->waitForBytesWritten(200)){

          }
       //   m_serial->waitForBytesWritten(200);
          while (m_serial->waitForReadyRead(200)) {
          }
          readData();
        }

        retries++;
        if (retries > max_retries) { // after 8 tries to get an ack

          break;
        }
      }
      if (four_way->ack_type == BAD_ACK) {

        ui->escStatusLabel_2->setText("FLASH FAILURE");
        return; //
      }
      if (four_way->ack_type == CRC_ERROR) {
        ui->escStatusLabel_2->setText("FLASH FAILURE");
        return;
        //            index = index -(256*j);
        //            i--;// go back to beggining of page to erase in case data
        //            has been written.

        break;
      }
      ui->progressBar->setValue((index * 100) / sizeofBin);
      QApplication::processEvents();
      if (index >= sizeofBin) {
        ui->escStatusLabel_2->setText("FLASH SUCCESS");
        ui->progressBar->setValue(0);
        four_way->ack_required = true;

        QByteArray another_eeprom_out;
        if(eeprom_buffer->size() != 0) {
        for (int i = 0; i < 48; i++) {
          another_eeprom_out.append(eeprom_buffer->at(i));
        }
        another_eeprom_out[00] = 0x01;
        if ((eeprom_buffer->at(1) < (char)0x03) || (eeprom_buffer->at(2) == char(0x00))) { // no eeprom ever sent, will be set to zero at
                            // beggining of flash.
            sendFirstEeprom(0);
          resetESC();
          return;
        } else {

          if (four_way->direct) {

            sendDirect(another_eeprom_out, 48, four_way->eeprom_address);

          } else {

            writeData(four_way->makeFourWayWriteCommand(
                another_eeprom_out, 48, four_way->eeprom_address));

            m_serial->waitForBytesWritten(1000);
            while (m_serial->waitForReadyRead(1000)) {
            }
          //  QByteArray data = m_serial->readAll();
          //  four_way->ack_required = false;
            readData();
          }
        }

        if (four_way->ack_required == false) { // good ack received from esc
          ui->escStatusLabel->setText("WRITE EEPROM SUCCESSFUL");
          writeMusic();
          return;
        } else {
          ui->escStatusLabel->setText("Unable to set safety bit");
          return;
        }
    //    resetESC();
        break;
        }
        }

    }
  }
  qInfo("what is going on? size :  %d ", sizeofBin);
}

bool Widget::getMusic() {
  four_way->ack_required = true;
  if (four_way->direct) {
    writeData(RL->setAddress(four_way->eeprom_address + 48));
    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(500)) {
    }
    QByteArray data = m_serial->readAll();
    if (data[data.size() - 1] == char(0x30)) {
      qInfo("good ack !!!!");
    } else {
      return false;
    }

    writeData(RL->readFlash(128));
    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(500)) {
    }
    QByteArray music = m_serial->readAll();
    music.remove(0, 4);
    //   qInfo("size of music : %d ", music.size());
    if (music[music.size() - 1] == char(0x30)) {
      qInfo("good ack read !!!!");
    } else {
      return false;
    }
    if ((uint8_t)music.at(0) == 0xFF) {
      musicBufferFull = false;
    } else {
      musicBufferFull = true;
    }
    music_buffer->clear();
    for (int i = 0; i < music.size() - 2; i++) {
      music_buffer->append(music[i]);
    }
  } else {

    while (four_way->ack_required) {

      writeData(
          four_way->makeFourWayReadCommand(128, four_way->eeprom_address + 48));
      m_serial->waitForBytesWritten(500);
      while (m_serial->waitForReadyRead(1000)) {
      }

      readData();
    }
    if ((uint8_t)input_buffer->at(0) == 0xFF) {
      musicBufferFull = false;
    } else {
      for (int i = 0; i < 128; i++) {
        music_buffer->append(input_buffer->at(i));
      }
      musicBufferFull = true;
    }
  }
  return true;
}

bool Widget::writeMusic() {
  if (musicBufferFull) {
    QByteArray musicBufferOut;
    for (int i = 0; i < 128; i++) {
      musicBufferOut.append(music_buffer->at(i));
    }
    if (four_way->direct) {
      sendDirect(musicBufferOut, 128, four_way->eeprom_address + 48);

    } else {

      writeData(four_way->makeFourWayWriteCommand(
          musicBufferOut, 128, four_way->eeprom_address + 48));

      m_serial->waitForBytesWritten(500);
      while (m_serial->waitForReadyRead(500)) {
      }

      readData();
    }
    if (four_way->ack_required == false) { // good ack received from esc
      ui->escStatusLabel->setText("WRITE EEPROM + SUCCESSFUL");

      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void Widget::on_VerifyFlash_clicked() {
  QFile inputFile(filename);
  inputFile.open(QIODevice::ReadOnly);

  //  QTextStream in(&inputFile);
  QByteArray line = inputFile.readAll();
  inputFile.close();

  uint16_t bin_size = line.size();
  uint16_t K128Chunks = bin_size / 128;

  uint32_t index = 0;

  for (int i = 0; i < K128Chunks + 1; i++) {
    retries = 0;
    four_way->ack_required = true;
    while (four_way->ack_required) {
      if (four_way->memory_divider_required_four) {
        writeData(
            four_way->makeFourWayReadCommand(128, (4096 + (i * 128)) >> 2));
      } else {
        writeData(four_way->makeFourWayReadCommand(128, 4096 + (i * 128)));
      }
      m_serial->waitForBytesWritten(500);
      while (m_serial->waitForReadyRead(500)) {
      }
      readData();
      retries++;
      if (retries > max_retries) { // after 8 tries to get an ack
        return;
      }
    }

    for (int j = 0; j < input_buffer->size(); j++) {
      if (input_buffer->at(j) == line.at(j + i * 128)) {
        qInfo("the same! index : %d", index);
        index++;
        if (index >= bin_size) {
          qInfo("all memory verified in flash memory");
          break;
        }
      } else {
        qInfo("data error in flash memory");
        return;
      }
    }

    ui->progressBar->setValue((index * 100) / bin_size);
    QApplication::processEvents();
  }
}

void Widget::endTimer(){
  timerdone = true;
    if (connectMotor(four_way->connected_motor)) {
      ui->escStatusLabel->setText("M1:Connected: Settings read OK ");
      return;
    } else {
      ui->escStatusLabel->setText("M1:Did not connect - retrying ");
      ui->StatusLabel->setText("Connecting");
    }
    if (connectMotor(four_way->connected_motor)) {
      ui->escStatusLabel->setText("M1:Connected: Settings read OK ");
    } else {
      ui->escStatusLabel->setText("M1:Did not connect");
    }
}


bool Widget::connectMotor(uint8_t motor) {
  uint16_t buffer_length = 48;

  ui->escStatusLabel->setText("Connecting to ESC...");
  ui->escStatusLabel_2->setText("Connecting to ESC...");
  QApplication::processEvents();


  four_way->ack_required = true;
  retries = 0;
  //   while(four_way->ack_required){
  if (four_way->direct) {
    uint8_t init[21] = {0, 0,    0,   0,   0,   0,   0,   0,   0,    0,   0,
                        0, 0x0D, 'B', 'L', 'H', 'e', 'l', 'i', 0xF4, 0x7D};
    QByteArray BootInit;
    for (int i = 0; i < 21; i++) {
      BootInit.append(init[i]);
    }
    writeData(BootInit);
    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(500)) {
    }
    readInitData();
    if (four_way->ESC_connected == false) {
      qInfo("not connected");
      ui->escStatusLabel->setText("Can Not Connect");
      ui->escStatusLabel_2->setText("Can Not Connect");
      return false;
    }
    writeData(RL->setAddress(four_way->eeprom_address - 32));
    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(500)) {
    }

    QByteArray data = m_serial->readAll();
    if (data[data.size() - 1] == char(0x30)) {
      qInfo("good ack !!!!");
    } else {
      return false;
    }

    writeData(RL->readFlash(48 + 32));
    m_serial->waitForBytesWritten(200);
    while (m_serial->waitForReadyRead(200)) {
    }

    QByteArray flash = m_serial->readAll();

    qInfo("size of flash 1 : %d ", flash.size());
    if(flash.size() == 87){
    flash.remove(0, 4);
    }
     qInfo("size of flash 2: %d ", flash.size());
    if (flash[flash.size() - 1] == char(0x30)) {
      qInfo("good ack read !!!!");
    } else {
      return false;
    }
    if (RL->checkCRC(flash, flash.size() - 1)) { // last byte ack 0x30
      qInfo("GOOD crc FROM ESC -- read");
      hideESCSettings(false);
      hideEEPROMSettings(false);
      ui->sendFirstEEPROM->setHidden(false);
      //ui->crawler_default_button->setHidden(false);
      input_buffer->clear();
      for (int i = 0; i < flash.size() - 2; i++) {
        input_buffer->append(flash[i]);
      }
    }

  } else {
    four_way->ack_required = true;
    writeData(four_way->makeFourWayCommand(0x37, motor));
    m_serial->waitForBytesWritten(200);
    while (m_serial->waitForReadyRead(200)) {
    }
    readData();

    if (four_way->ESC_connected == false) {
      qInfo("not connected");
      ui->escStatusLabel->setText("Can Not Connect");
      ui->escStatusLabel_2->setText("Can Not Connect");
      return false;
    }

    four_way->ack_required = true;
    while (four_way->ack_required) {

      writeData(four_way->makeFourWayReadCommand(buffer_length + 32,
                                                 four_way->eeprom_address - 32));
      m_serial->waitForBytesWritten(300);
      while (m_serial->waitForReadyRead(300)) {
      }
      qInfo("reads");
      readData();
      retries++;
      if (retries > max_retries / 4) {
        return false;
      }
    }
  }

  QString name; // 2 bytes
      name.append(QChar(input_buffer->at(0)));
      name.append(QChar(input_buffer->at(1)));
      name.append(QChar(input_buffer->at(2)));
      name.append(QChar(input_buffer->at(3)));
      name.append(QChar(input_buffer->at(4)));
      name.append(QChar(input_buffer->at(5)));
      name.append(QChar(input_buffer->at(6)));
      name.append(QChar(input_buffer->at(7)));
      name.append(QChar(input_buffer->at(8)));
      name.append(QChar(input_buffer->at(9)));
      name.append(QChar(input_buffer->at(10)));
      name.append(QChar(input_buffer->at(11)));
      name.append(QChar(input_buffer->at(12)));
      name.append(QChar(input_buffer->at(13)));
      name.append(QChar(input_buffer->at(14)));
  ui->FirmwareNameLabel->setText(name);
  if (auto *firmware = findChild<QLabel *>("webInfoValue0")) {
    firmware->setText(name);
  }

input_buffer->remove(0, 32);
qInfo("inputBUFFERAT0 : %d ", input_buffer->at(0));

//return false;
if ((input_buffer->at(0) == (char)0xFF)) {
    QMessageBox::warning( // no settings area found
        this, tr("Application Name"),
        tr("Boot bit set to 0xFF"));
    return 0;
}
if ((input_buffer->at(0) == (char)0x01)) {
    if ((input_buffer->at(1) == (char)0xFF) ||
        (input_buffer->at(2) ==
         (char)0x00)) { // if eeprom version is 0xff it means boot bit is set
                        // but no defaults have been sent
      QMessageBox::warning( // also if bootloader version is 00 it means it has
                            // been written
          this, tr("Application Name"),
          tr("No settings found use 'Send default EEPROM' under FLASH tab"));
      ui->eepromFrame->setHidden(true);
      ui->inputservoFrame->setHidden(true);
      ui->flashFourwayFrame->setHidden(false);
      ui->escStatusLabel_2->setText("Connected - No EEprom");
      ui->escStatusLabel->setText("Connected - No EEprom");
      return false;
    }
    if ((input_buffer->at(1) <
         (char)0x03)) { // if eeprom version is 0xff it means boot bit is set
                        // but no defaults have been sent
      QMessageBox::warning( // also if bootloader version is 00 it means it has
                            // been written
          this, tr("Application Name"),
          tr("Outdated firmware detected 'This config tool is for 2.19 or higher"));
   //   ui->sendFirstEEPROM->setHidden(true);
      ui->crawler_default_button->setHidden(true);
      ui->eepromFrame->setHidden(true);
      ui->inputservoFrame->setHidden(true);
      ui->flashFourwayFrame->setHidden(false);
      ui->escStatusLabel_2->setText("Connected - Firmware Update Required");
      ui->escStatusLabel->setText("Connected - Firmware Update Required");
      return false;
    }


    ui->maxRocSlider->setValue((uint8_t)(input_buffer->at(5)));
    ui->minDutySlider->setValue((uint8_t)(input_buffer->at(6)));
    if(input_buffer->at(7) == 0x01){
      ui->disableStickCalibCheckbox->setChecked(true);
    }else{
      ui->disableStickCalibCheckbox->setChecked(false);
    }
    ui->absoluteVoltageSlider->setValue((uint8_t)(input_buffer->at(8)));
    ui->currentLimitPedit->setText(QString::number((uint8_t)(input_buffer->at(9))*2));
    ui->currentLimitIedit->setText(QString::number((uint8_t)(input_buffer->at(10))));
    ui->currentLimitDedit->setText(QString::number((uint8_t)(input_buffer->at(11))*2));
    ui->activeBrakeSlider->setValue((uint8_t)(input_buffer->at(12)));



                if(input_buffer->at(17) == 0x01){
                ui->rvCheckBox->setChecked(true);
            }else{
               ui->rvCheckBox->setChecked(false);
            }


    if (input_buffer->at(18) == 0x01) {
      ui->biDirectionCheckbox->setChecked(true);
    } else {
      ui->biDirectionCheckbox->setChecked(false);
    }
    if (input_buffer->at(19) == 0x01) {
      ui->sinCheckBox->setChecked(true);
    } else {
      ui->sinCheckBox->setChecked(false);
    }
    if (input_buffer->at(20) == 0x01) {
      ui->comp_pwmCheckbox->setChecked(true);
    } else {
      ui->comp_pwmCheckbox->setChecked(false);
    }
    if (input_buffer->at(21) == 0x01) {
      ui->varPWMCheckBox->setChecked(true);
    } else {
      ui->varPWMCheckBox->setChecked(false);
    }
    if (input_buffer->at(21) == 0x02) {
      ui->autoPWM->setChecked(true);
    } else {
      ui->autoPWM->setChecked(false);
    }

    if (input_buffer->at(22) == 0x01) {
      ui->stuckProtectionBox->setChecked(true);
    } else {
      ui->stuckProtectionBox->setChecked(false);
    }
    ui->timingAdvanceLCD->display(((input_buffer->at(23))-10)*0.9375);
    ui->timingAdvanceSlider->setValue(input_buffer->at(23));
    ui->pwmFreqSlider->setValue((uint8_t)input_buffer->at(24));
    ui->startupPowerSlider->setValue((uint8_t)input_buffer->at(25));
    ui->motorKVSlider->setValue((uint8_t)input_buffer->at(26));
    ui->motorPolesSlider->setValue(input_buffer->at(27));

    if (input_buffer->at(28) == 0x01) {
      ui->brakecheckbox->setChecked(true);
    } else {
      ui->brakecheckbox->setChecked(false);
    }

    if (input_buffer->at(28) == 0x02) {
      ui->activeBrakeCheckbox->setChecked(true);
    } else {
      ui->activeBrakeCheckbox->setChecked(false);
    }

    if (input_buffer->at(29) == 0x01) {
      ui->antiStallBox->setChecked(true);
    } else {
      ui->antiStallBox->setChecked(false);
    }
    if (input_buffer->at(1) == 0) { // if ESC is curently on eeprom version 0
      ui->beepVolumeSlider->setValue(5);
      ui->servoLowSlider->setValue(128);
      ui->servoHighSlider->setValue(128);
      ui->servoNeutralSlider->setValue(128);
      ui->servoDeadBandSlider->setValue(50);
      ui->thirtymsTelemBox->setChecked(false);
    } else {

      ui->beepVolumeSlider->setValue(input_buffer->at(30));
      if (input_buffer->at(31) == 0x01) {
        ui->thirtymsTelemBox->setChecked(true);
      } else {
        ui->thirtymsTelemBox->setChecked(false);
      }
      ui->servoLowSlider->setValue((uint8_t)(input_buffer->at(32)));
      ui->servoHighSlider->setValue((uint8_t)(input_buffer->at(33)));
      ui->servoNeutralSlider->setValue((uint8_t)(input_buffer->at(34)));
      ui->servoDeadBandSlider->setValue((uint8_t)(input_buffer->at(35)));

      if (input_buffer->at(36) == 0x01) {
        ui->lowVoltageCuttoffBox->setChecked(true);
      } else {
        ui->lowVoltageCuttoffBox->setChecked(false);
      }
      ui->lowVoltageThresholdSlider->setValue((uint8_t)(input_buffer->at(37)));
      if (input_buffer->at(38) == 0x01) {
        ui->rcCarReverse->setChecked(true);
      } else {
        ui->rcCarReverse->setChecked(false);
      }
      if (input_buffer->at(39) == 0x01) {
        ui->hallSensorCheckbox->setChecked(true);
      } else {
        ui->hallSensorCheckbox->setChecked(false);
      }
      ui->sineStartupSlider->setValue((uint8_t)(input_buffer->at(40)));
      ui->dragBrakeSlider->setValue((uint8_t)(input_buffer->at(41)));

      ui->runningBrakeStrength->setValue((uint8_t)(input_buffer->at(42)));
      if ((uint8_t)(input_buffer->at(45)) > 10) {
        ui->sineModePowerSlider->setValue(5);
      } else {
        ui->sineModePowerSlider->setValue((uint8_t)(input_buffer->at(45)));
      }

      if ((uint8_t)(input_buffer->at(43)) < 70) {
        ui->temperatureSlider->setValue(142);
      } else {
        ui->temperatureSlider->setValue((uint8_t)(input_buffer->at(43)));
      }
      ui->currentSlider->setValue((uint8_t)(input_buffer->at(44)));
      ui->signalComboBox->setCurrentIndex((uint8_t)(input_buffer->at(46)));
    }
    if (input_buffer->at(47) == 0x01) {
      ui->AutoTimingButton->setChecked(true);
    } else {
      ui->AutoTimingButton->setChecked(false);
    }


    QString version = "FW Rev:";
    QString major = QString::number((uint8_t)input_buffer->at(3));
    QString minor = QString::number((uint8_t)input_buffer->at(4));
    version.append(major);
    version.append(".");
    version.append(minor);
    ui->FirmwareVerionsLabel->setText(version);
    if (auto *eeprom = findChild<QLabel *>("webInfoValue1")) {
      eeprom->setText(version);
    }

    QChar charas = input_buffer->at(18);
    int output = charas.toLatin1();


    qInfo(" output integer %i", output);
    eeprom_buffer->clear();
    // eeprom_buffer = input_buffer;
    for (int i = 0; i < buffer_length; i++) {
      eeprom_buffer->append(input_buffer->at(i));
    }
    ui->escStatusLabel_2->setText("Connected");
    if (!four_way->direct) {
      getMusic();
    }
    return true;

  } else {
    QMessageBox::warning( // no settings area found
        this, tr("Application Name"),
        tr("No Firmware found 'Please flash latest AM32 firmware"));

    ui->eepromFrame->setHidden(true);
    ui->inputservoFrame->setHidden(true);
    //ui->sendFirstEEPROM->setHidden(true);
    ui->crawler_default_button->setHidden(true);
    ui->flashFourwayFrame->setHidden(false);
    ui->biDirectionCheckbox->setChecked(false);
    ui->rvCheckBox->setChecked(false);
    ui->sinCheckBox->setChecked(false);
    ui->escStatusLabel_2->setText("Connected - No EEprom");
    ui->escStatusLabel->setText("Connected - No EEprom");
    for (int i = 0; i < 48; i++) {
      eeprom_buffer->append(char(0));
    }
    return false;
  }
}

void Widget::allup() {
  ui->initMotor1->setDown(false);
  ui->initMotor1_2->setDown(false);
  ui->initMotor1_3->setDown(false);
  ui->initMotor1->setFlat(false);
  ui->initMotor2->setFlat(false);
  ui->initMotor3->setFlat(false);
  ui->initMotor4->setFlat(false);
  ui->initMotor1_2->setFlat(false);
  ui->initMotor2_2->setFlat(false);
  ui->initMotor3_2->setFlat(false);
  ui->initMotor4_2->setFlat(false);
  ui->initMotor1_3->setFlat(false);
  ui->initMotor2_3->setFlat(false);
  ui->initMotor3_3->setFlat(false);
  ui->initMotor4_3->setFlat(false);
}

void Widget::showSingleMotor(bool tf) {

  ui->initMotor2->setHidden(tf);
  ui->initMotor3->setHidden(tf);
  ui->initMotor4->setHidden(tf);

  ui->initMotor2_2->setHidden(tf);
  ui->initMotor3_2->setHidden(tf);
  ui->initMotor4_2->setHidden(tf);

  ui->initMotor2_3->setHidden(tf);
  ui->initMotor3_3->setHidden(tf);
  ui->initMotor4_3->setHidden(tf);
}

void Widget::on_initMotor1_clicked() {
  allup();
  if (four_way->direct) {
    ui->initMotor1->setDown(true);
    ui->initMotor1_2->setDown(true);
    ui->initMotor1_3->setDown(true);
  } else {
    ui->initMotor1->setFlat(true);
    ui->initMotor1_2->setFlat(true);
    ui->initMotor1_3->setFlat(true);
  }
  ui->MotorLabel->setText("M1:");
 // timerdone = false;
 // QTimer::singleShot(1200, this, &Widget::endTimer);
 // four_way->connected_motor = 0x00;


    if (connectMotor(0x00)) {
    ui->escStatusLabel->setText("M1:Connected: Settings read OK ");
    four_way->connected_motor = 0x00;
    return;
  } else {
    ui->escStatusLabel->setText("M1:Did not connect - retrying ");
    ui->StatusLabel->setText("Connecting");
  }
  if (connectMotor(0x00)) {
    ui->escStatusLabel->setText("M1:Connected: Settings read OK ");
    four_way->connected_motor = 0x00;
  } else {
    ui->escStatusLabel->setText("M1:Did not connect");
  }
//  four_way->ack_required = true;
//  writeData(four_way->makeFourWayCommand(0x37, 0x00));
//  m_serial->waitForBytesWritten(200);
//  while (m_serial->waitForReadyRead(100)) {
//  }
}

void Widget::on_initMotor2_clicked() {
  allup();
  ui->initMotor2->setFlat(true);
  ui->initMotor2_2->setFlat(true);
  ui->initMotor2_3->setFlat(true);
  ui->MotorLabel->setText("M2:");
  if (connectMotor(0x01)) {
    ui->escStatusLabel->setText("M2:Connected: Settings read OK ");
    four_way->connected_motor = 0x01;
    return;
  } else {
    ui->escStatusLabel->setText("M2:Did not connect - retrying ");
    ui->StatusLabel->setText("Connecting");
  }
  if (connectMotor(0x01)) {
    ui->escStatusLabel->setText("M2:Connected: Settings read OK ");
    four_way->connected_motor = 0x01;
  } else {
    ui->escStatusLabel->setText("M2:Did not connect");
  }
//  timerdone = false;
//  QTimer::singleShot(1200, this, &Widget::endTimer);
//  four_way->connected_motor = 0x01;
//  four_way->ack_required = true;
//  writeData(four_way->makeFourWayCommand(0x37, 0x01));
//  m_serial->waitForBytesWritten(200);
//  while (m_serial->waitForReadyRead(100)) {
//  }
}

void Widget::on_initMotor3_clicked() {
  allup();
  ui->initMotor3->setFlat(true);
  ui->initMotor3_2->setFlat(true);
  ui->initMotor3_3->setFlat(true);
  ui->MotorLabel->setText("M3:");
  if (connectMotor(0x02)) {
    ui->escStatusLabel->setText("M3:Connected: Settings read OK ");
    four_way->connected_motor = 0x02;
    return;
  } else {
    ui->escStatusLabel->setText("M3:Did not connect - retrying ");

    ui->StatusLabel->setText("Connecting");
  }
  if (connectMotor(0x02)) {
    ui->escStatusLabel->setText("M3:Connected: Settings read OK ");
    four_way->connected_motor = 0x02;
  } else {
    ui->escStatusLabel->setText("M3:Did not connect");
  }
//  timerdone = false;
//  QTimer::singleShot(1200, this, &Widget::endTimer);
//  four_way->connected_motor = 0x02;
//  four_way->ack_required = true;
//  writeData(four_way->makeFourWayCommand(0x37, 0x02));
//  m_serial->waitForBytesWritten(200);
//  while (m_serial->waitForReadyRead(100)) {
//  }
}

void Widget::on_initMotor4_clicked() {
  allup();
  ui->initMotor4->setFlat(true);
  ui->initMotor4_2->setFlat(true);
  ui->initMotor4_3->setFlat(true);
  ui->MotorLabel->setText("M4:");
  if (connectMotor(0x03)) {
    ui->escStatusLabel->setText("M4:Connected: Settings read OK ");
    four_way->connected_motor = 0x03;
    return;
  } else {
    ui->escStatusLabel->setText("M4:Did not connect - retrying ");
    ui->StatusLabel->setText("Connecting");
  }

  if (connectMotor(0x03)) {
    ui->escStatusLabel->setText("M4:Connected: Settings read OK ");
    four_way->connected_motor = 0x03;
  } else {
    ui->escStatusLabel->setText("M4:Did not connect");
  }
//  timerdone = false;
//  QTimer::singleShot(1200, this, &Widget::endTimer);
//  four_way->connected_motor = 0x03;
//  four_way->ack_required = true;
//  writeData(four_way->makeFourWayCommand(0x37, 0x03));
//  m_serial->waitForBytesWritten(200);
//  while (m_serial->waitForReadyRead(100)) {
//  }
}

void Widget::sendDirect(const QByteArray sendbuffer, uint16_t buffer_size,
                        uint16_t address) {
  writeData(RL->setAddress(address)); // set address
  m_serial->waitForBytesWritten(10);
  while (m_serial->waitForReadyRead(20)) {
  }
  QByteArray data = m_serial->readAll();
  if (data[data.size() - 1] == char(0x30)) {
    qInfo("good ADDRESS ack !!!!");
  } else {
    four_way->ack_type = BAD_ACK;
    return;
  }
  writeData(RL->setBufferSize(buffer_size)); // set buffer size
 m_serial->waitForBytesWritten(10);
  while (m_serial->waitForReadyRead(20)) {
  }
  writeData(RL->sendBuffer(sendbuffer)); // send buffer
  m_serial->waitForBytesWritten(20);
  while (m_serial->waitForReadyRead(75)) {
  }
  QByteArray data2 = m_serial->readAll();
  if (data2[data2.size() - 1] == char(0x30)) {
    qInfo("good ack receive !!!!");
  } else {
    four_way->ack_type = BAD_ACK;
    return;
  }
  writeData(RL->writeFlash()); // send write command
  m_serial->waitForBytesWritten(10);
  while (m_serial->waitForReadyRead(30)) {
  }
  QByteArray data3 = m_serial->readAll();
  if (data3[data3.size() - 1] == char(0x30)) {
    qInfo("good ack flash !!!!");
    four_way->ack_required = false;
    four_way->ack_type = ACK_OK;
  } else {
    four_way->ack_type = BAD_ACK;
    return;
  }
}

void Widget::on_writeEEPROM_2_clicked() { on_writeEEPROM_clicked(); }

void Widget::on_writeEEPROM_clicked() {
  four_way->ack_required = true;
  QByteArray eeprom_out;
  for (int i = 0; i < 48; i++) {
    eeprom_out.append(eeprom_buffer->at(i));
  }

  eeprom_out[5] = ui->maxRocSlider->value();
  eeprom_out[6] = ui->minDutySlider->value();
  eeprom_out[7] = ui->disableStickCalibCheckbox->isChecked();
  eeprom_out[8] = ui->absoluteVoltageSlider->value();
  eeprom_out[9] = (ui->currentLimitPedit->text().toInt())/2;
  eeprom_out[10] = ui->currentLimitIedit->text().toInt();
  eeprom_out[11] = (ui->currentLimitDedit->text().toInt())/2;
  eeprom_out[12] = ui->activeBrakeSlider->value();

  eeprom_out[17] = (char)ui->rvCheckBox->isChecked();
  eeprom_out[18] = (char)ui->biDirectionCheckbox->isChecked();
  eeprom_out[19] = (char)ui->sinCheckBox->isChecked();
  eeprom_out[20] = (char)ui->comp_pwmCheckbox->isChecked();

  if((char)ui->varPWMCheckBox->isChecked()){
    eeprom_out[21] = 0x01;
  } else if((char)ui->autoPWM->isChecked()){
    eeprom_out[21] = 0x02;
  } else {
    eeprom_out[21] = 0x00;
  }

  eeprom_out[22] = (char)ui->stuckProtectionBox->isChecked();
  eeprom_out[23] = (char)ui->timingAdvanceSlider->value();
  eeprom_out[24] = (uint8_t)ui->pwmFreqSlider->value();
  eeprom_out[25] = (char)ui->startupPowerSlider->value();
  eeprom_out[26] = (char)ui->motorKVSlider->value();
  eeprom_out[27] = (char)ui->motorPolesSlider->value();

  if((char)ui->brakecheckbox->isChecked()){
    eeprom_out[28] = 0x01;
  } else if((char)ui->activeBrakeCheckbox->isChecked()){
    eeprom_out[28] = 0x02;
  }else{
    eeprom_out[28] = 0x00;
  }

  eeprom_out[29] = (char)ui->antiStallBox->isChecked();
  eeprom_out[30] = (char)ui->beepVolumeSlider->value();
  eeprom_out[31] = (char)ui->thirtymsTelemBox->isChecked();
  eeprom_out[32] = (char)ui->servoLowSlider->value();
  eeprom_out[33] = (char)ui->servoHighSlider->value();
  eeprom_out[34] = (char)ui->servoNeutralSlider->value();
  eeprom_out[35] = (char)ui->servoDeadBandSlider->value();
  eeprom_out[36] = (char)ui->lowVoltageCuttoffBox->isChecked();
  eeprom_out[37] = (char)ui->lowVoltageThresholdSlider->value();
  eeprom_out[38] = (char)ui->rcCarReverse->isChecked();
  eeprom_out[39] = (char)ui->hallSensorCheckbox->isChecked();
  eeprom_out[40] = (char)ui->sineStartupSlider->value();
  eeprom_out[41] = (char)ui->dragBrakeSlider->value();
  eeprom_out[42] = (char)ui->runningBrakeStrength->value();
  eeprom_out[43] = (char)ui->temperatureSlider->value();
  eeprom_out[44] = (char)ui->currentSlider->value();
  eeprom_out[45] = (char)ui->sineModePowerSlider->value();
  eeprom_out[46] = (char)ui->signalComboBox->currentIndex();
  eeprom_out[47] = (char)ui->AutoTimingButton->isChecked();

  if (four_way->direct) {
    sendDirect(eeprom_out, 48, four_way->eeprom_address);
    ui->escStatusLabel->setText("WRITE EEPROM SUCCESSFUL");

  } else {

    writeData(four_way->makeFourWayWriteCommand(eeprom_out, 48,
                                                four_way->eeprom_address));

    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(500)) {
    }

    readData();
    if (four_way->ack_required == false) { // good ack received from esc
      ui->escStatusLabel->setText("WRITE EEPROM SUCCESSFUL");
      writeMusic();
    }
  }
}

void Widget::hide4wayButtons(bool b) {
  ui->fourWayFrame->setHidden(b);
  ui->connectFrameInputPage->setHidden(b);
  ui->flashMotorsFrame->setHidden(b);
 // ui->tunesFrame->setHidden(b);
  //   ui->flashFourwayFrame->setHidden(b);
}

void Widget::hideESCSettings(bool b) {
  // ui->flashFourwayFrame->setHidden(b);
  //  qInfo(" slot working");
}

void Widget::hideEEPROMSettings(bool b) {
  ui->eepromFrame->setHidden(b);
  ui->inputservoFrame->setHidden(b);
  ui->flashFourwayFrame->setHidden(b);
  ui->writeEEPROM->setDisabled(b);
  ui->saveConfigButton->setDisabled(b);
  ui->loadConfigButton->setDisabled(b);
  if (auto *reset = findChild<QPushButton *>("webResetButton")) {
    reset->setDisabled(b);
  }
  if (auto *restore = findChild<QPushButton *>("webRestoreButton")) {
    restore->setDisabled(b);
  }
  ui->MusicTextEdit->setHidden(b);
  ui->uploadMusic->setHidden(b);
  ui->tunesFrame->setHidden(b);

  //  qInfo(" slot working");
}

void Widget::sendFirstEeprom(uint8_t eeprom_type) {

  QByteArray eeprom_out;
  if (eeprom_type == 0) {
    for (int i = 0; i < 48; i++) {
      eeprom_out.append((char)air_starteeprom[i]);
    }
  }
  if (eeprom_type == 1) {
    for (int i = 0; i < 48; i++) {
      eeprom_out.append((char)crawler_starteeprom[i]);
    }
  }
  four_way->ack_required = true;

  if (four_way->direct) {
    sendDirect(eeprom_out, 48, four_way->eeprom_address);
  } else {
    writeData(four_way->makeFourWayWriteCommand(eeprom_out, 48,
                                                four_way->eeprom_address));
    m_serial->waitForBytesWritten(500);
    while (m_serial->waitForReadyRead(500)) {
    }

    readData();
  }
  if (four_way->ack_required == false) { // good ack received from esc
    ui->escStatusLabel->setText("WRITE DEFAULT SUCCESS");
  }
  ui->eepromFrame->setHidden(true);
  ui->inputservoFrame->setHidden(true);
}
void Widget::on_sendFirstEEPROM_clicked() {
  sendFirstEeprom(0);
 // resetESC();////////////////////////////////////////////////////////////////////////////////////////////////////////debug!!
}

void Widget::on_devSettings_stateChanged(int arg1) {
  if (arg1 == 0) {
    // ui->devFrame->setHidden(true);
  } else {
    //  ui->devFrame->setHidden(false);
  }
}

void Widget::on_endPassthrough_clicked() {
  hide4wayButtons(true);
  hideESCSettings(true);
  hideEEPROMSettings(true);
  writeData(four_way->makeFourWayCommand(0x34, 0x00));
  parseMSPMessage = true;
  four_way->passthrough_started = false;
}

void Widget::on_checkBox_stateChanged(int arg1) {

  if (ui->checkBox->isChecked()) {
    writeData(four_way->makeFourWayCommand(0x34, 0x00)); // get msp throttle
    m_serial->waitForBytesWritten(200);
    while (m_serial->waitForReadyRead(200)) {
    }
    readData();
    //    parseMSPMessage = true;
    four_way->passthrough_started = false;
    hideEEPROMSettings(true);
    four_way->ESC_connected = false;
    ui->escStatusLabel->setText("Disconnected - Disable MSP motor control");
    ui->escStatusLabel_2->setText("Disconnected");
    //    parseMSPMessage = true;
    four_way->ack_required = false;

    QByteArray passthroughenable2;
    if (four_way->direct == false) {

      //            send_mspCommand(0x68,passthroughenable2);
      //            m_serial->waitForBytesWritten(200);
      //            while (m_serial->waitForReadyRead(200)){

      //            }
      //            readData();
    }

    //        if(four_way->direct == false){
    //            m_serial->clear();
    //            send_mspCommand(0x63,0x00);      // get motor values from
    //            flight controller m_serial->waitForBytesWritten(100); while
    //            (m_serial->waitForReadyRead(100)){

    //            }
    //            QByteArray data = m_serial->readAll();
    //            qInfo("size of data : %d ", data.size());
    //            QByteArray data_hex_string = data.toHex();
    //            if(data.size() == 31){
    //            if(data[9] == char(0x24) && data[11] == char(0x3e)){
    //              motor1throttle =  (uint8_t)data[16] |
    //              (uint8_t(data[15])<<8); qInfo("Throttle : %d ",
    //              motor1throttle);
    //              ui->horizontalSlider->setValue(motor1throttle);
    //              ui->m1MSPSlider->setValue(motor1throttle);
    //              ui->m2MSPSlider->setValue(motor1throttle);
    //              ui->m3MSPSlider->setValue(motor1throttle);
    //              ui->m4MSPSlider->setValue(motor1throttle);
    //            }
    //            }
    //            ui->horizontalSlider->setValue(motor1throttle);
    //            ui->m1MSPSlider->setValue(motor1throttle);
    //            ui->m2MSPSlider->setValue(motor1throttle);
    //            ui->m3MSPSlider->setValue(motor1throttle);
    //            ui->m4MSPSlider->setValue(motor1throttle);
    //           }
  } else {
    QByteArray passthroughenable2; // payload  empty here
    ui->horizontalSlider->setValue(0);
    ui->m1MSPSlider->setValue(0);
    ui->m2MSPSlider->setValue(0);
    ui->m3MSPSlider->setValue(0);
    ui->m4MSPSlider->setValue(0);
    sendMSPThrottle();
    m_serial->waitForBytesWritten(200);

    parseMSPMessage = false;
    send_mspCommand(0xf5, passthroughenable2);
    ui->escStatusLabel->setText("Disconnected - Select Motor");
    ui->escStatusLabel_2->setText("Disconnected");
    four_way->passthrough_started = true;
  }
}

void Widget::on_initMotor1_2_clicked() { on_initMotor1_clicked(); }

void Widget::on_initMotor2_2_clicked() { on_initMotor2_clicked(); }

void Widget::on_initMotor3_2_clicked() { on_initMotor3_clicked(); }

void Widget::on_initMotor4_2_clicked() { on_initMotor4_clicked(); }

void Widget::on_startupPowerSlider_valueChanged(int value) {
  ui->startupPowerLCD->display(value);
}

void Widget::on_timingAdvanceSlider_valueChanged(int value) {
  ui->timingAdvanceLCD->display((value - 10) * .9375);
}

void Widget::on_pwmFreqSlider_valueChanged(int value) {
  ui->pwmFreqLCD->display(value);
}

void Widget::on_motorKVSlider_valueChanged(int value) {
  ui->motorKVLCD->display((value * 40) + 20);
}

void Widget::on_motorPolesSlider_valueChanged(int value) {
  ui->motorPolesLCD->display(value);
}

void Widget::on_beepVolumeSlider_valueChanged(int value) {
  ui->beepVolumeLCD->display(value);
}

void Widget::on_dragBrakeSlider_valueChanged(int value) {
  ui->dragBrakeLCD->display(value);
}

void Widget::on_sineStartupSlider_valueChanged(int value) {
  ui->sineRangeLCD->display(value);
}

void Widget::on_sineModePowerSlider_valueChanged(int value) {
  ui->sineLcd->display(value);
}

void Widget::on_runningBrakeStrength_valueChanged(int value) {
  ui->runningBrakeLcd->display(value);
}

void Widget::on_temperatureSlider_valueChanged(int value) {
  ui->temperatureLineEdit->setText(QString::number(value));
  if (value > 140) {
    ui->temperatureLineEdit->setText("Disable");
  }
}

void Widget::on_currentSlider_valueChanged(int value) {
  ui->currentLineEdit->setText(QString::number(value * 2));
  if (value > 100) {
    ui->currentLineEdit->setText("Disable");
  }
}

void Widget::on_varPWMCheckBox_stateChanged(int arg1) {
  if (ui->varPWMCheckBox->isChecked()) {
    ui->pwmFreqSlider->setEnabled(false);
  } else {
    ui->pwmFreqSlider->setEnabled(true);
  }
}

void Widget::sendMSPThrottle() {
  uint16_t m1throttle = ui->m1throttle->text().toInt();
  uint16_t m2throttle = ui->m2throttle->text().toInt();
  uint16_t m3throttle = ui->m3throttle->text().toInt();
  uint16_t m4throttle = ui->m4throttle->text().toInt();
  //  char highByteThrottle = (m1throttle >> 8) & 0xff;
  //  char lowByteThrottle = m1throttle & 0xff;
  QByteArray sliderThrottle;

  sliderThrottle.append((char)m1throttle & 0xff);        // motor 1
  sliderThrottle.append((char)(m1throttle >> 8) & 0xff); //
  sliderThrottle.append((char)m2throttle & 0xff);        // motor 2
  sliderThrottle.append((char)(m2throttle >> 8) & 0xff);
  sliderThrottle.append((char)m3throttle & 0xff);
  sliderThrottle.append((char)(m3throttle >> 8) & 0xff);
  sliderThrottle.append((char)m4throttle & 0xff);
  sliderThrottle.append((char)(m4throttle >> 8) & 0xff);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);
  sliderThrottle.append((char)0x00);

  if (ui->checkBox->isChecked()) {
    send_mspCommand(0xd6, sliderThrottle);
  }
}
void Widget::on_m1MSPSlider_valueChanged(int value) {
  QString s = QString::number(value);
  ui->m1throttle->setText(s);
  sendMSPThrottle();
}

void Widget::on_m2MSPSlider_valueChanged(int value) {

  QString s = QString::number(value);
  ui->m2throttle->setText(s);
  sendMSPThrottle();
}

void Widget::on_m3MSPSlider_valueChanged(int value) {

  QString s = QString::number(value);
  ui->m3throttle->setText(s);
  sendMSPThrottle();
}

void Widget::on_m4MSPSlider_valueChanged(int value) {
  QString s = QString::number(value);
  ui->m4throttle->setText(s);
  sendMSPThrottle();
}

void Widget::on_ConnectedButton_clicked() {
  if (four_way->passthrough_started == true) {
    ui->ConnectedButton->setChecked(true);
  }
}

void Widget::on_lowThresholdLineEdit_editingFinished() {
  ui->servoLowSlider->setValue(
      (((ui->lowThresholdLineEdit->text()).toInt()) - 750) / 2);
}

void Widget::on_servoLowSlider_valueChanged(int value) {
  ui->lowThresholdLineEdit->setText(QString::number(value * 2 + 750));
}

void Widget::on_highThresholdLineEdit_editingFinished() {
  ui->servoHighSlider->setValue(
      (((ui->highThresholdLineEdit->text()).toInt()) - 1750) / 2);
}
void Widget::on_servoHighSlider_valueChanged(int value) {
  ui->highThresholdLineEdit->setText(QString::number((value * 2) + 1750));
}

void Widget::on_servoNeuralLineEdit_editingFinished() {
  ui->servoNeutralSlider->setValue(((ui->servoNeuralLineEdit->text()).toInt()) -
                                   1374);
}
void Widget::on_servoNeutralSlider_valueChanged(int value) {
  ui->servoNeuralLineEdit->setText(QString::number(value + 1374));
}

void Widget::on_servoDeadbandLineEdit_editingFinished() {
  ui->servoDeadBandSlider->setValue(
      (ui->servoDeadbandLineEdit->text()).toInt());
}
void Widget::on_servoDeadBandSlider_valueChanged(int value) {
  ui->servoDeadbandLineEdit->setText(QString::number(value));
}

void Widget::on_lowVoltageLineEdit_editingFinished() {
  ui->lowVoltageThresholdSlider->setValue(
      (ui->lowVoltageLineEdit->text().toInt()*100));
}
void Widget::on_lowVoltageThresholdSlider_valueChanged(int value) {
  ui->lowVoltageLineEdit->setText(QString::number((float)(value + 250)/100));
}

void Widget::on_initMotor1_3_clicked() { on_initMotor1_clicked(); }

void Widget::on_initMotor2_3_clicked() { on_initMotor2_clicked(); }

void Widget::on_initMotor3_3_clicked() { on_initMotor3_clicked(); }

void Widget::on_initMotor4_3_clicked() { on_initMotor4_clicked(); }

int Widget::getshift(int some_number)

{
  int pos = 1;
  // counting the position of first set bit
  for (int i = 0; i < 8; i++) {
    if (!(some_number & (1 << i)))
      pos++;
    else
      break;
  }
  return pos;
}


void Widget::on_uploadMusic_clicked()
{
   QString music = ui->MusicTextEdit->toPlainText();
   uint8_t bytes[128];
   uint16_t gen_length = ui->genLengthSpinbox->value();
   uint16_t newbpm =  (12*240) / gen_length;

   blheli32_to_bluejay_array(music,newbpm,bytes);

  // for( int i = 0; i < 128; i ++){
  //  qInfo(" output integer %i", bytes[i]+128);
  //  }

   QByteArray eeprom_music_out;
   uint16_t buffersize2 = 128;
 //  qInfo(" buffersize: %i", buffersize2);

           for (int i = 0; i < 48; i++) {
               eeprom_music_out.append((char)eeprom_buffer->at(i));
           }
            for (int i = 0; i < buffersize2; i++) {
                eeprom_music_out.append((char)bytes[i]);

            }
            eeprom_music_out[50]= (255 - (ui->genIntervalSpinbox->value()));
            while (eeprom_music_out.size() % 4 != 0)
                eeprom_music_out.append('\xFF');

            uint16_t totalbuffersize = eeprom_music_out.size();
//qInfo(" totalbuffersize: %i", totalbuffersize);
       four_way->ack_required = true;

       if (four_way->direct) {
           sendDirect(eeprom_music_out, totalbuffersize, four_way->eeprom_address);
       } else {
           writeData(four_way->makeFourWayWriteCommand(eeprom_music_out, totalbuffersize,
                                                       four_way->eeprom_address));
           m_serial->waitForBytesWritten(1500);
           while (m_serial->waitForReadyRead(1500)) {
           }

           readData();
       }
       if (four_way->ack_required == false) { // good ack received from esc
           ui->escStatusLabel->setText("WRITE DEFAULT SUCCESS");
       }
       ui->eepromFrame->setHidden(true);
       ui->inputservoFrame->setHidden(true);

}

void Widget::on_crawler_default_button_clicked() {
  sendFirstEeprom(1);
  resetESC();
}

void Widget::on_saveConfigButton_clicked()
{
  QByteArray eeprom_out;
  for (int i = 0; i < 48; i++) {
    eeprom_out.append(eeprom_buffer->at(i));
  }
  eeprom_out[5] = ui->maxRocSlider->value();
  eeprom_out[6] = ui->minDutySlider->value();
  eeprom_out[7] = ui->disableStickCalibCheckbox->isChecked();
  eeprom_out[8] = ui->absoluteVoltageSlider->value();
  eeprom_out[9] = (ui->currentLimitPedit->text().toInt())/2;
  eeprom_out[10] = ui->currentLimitIedit->text().toInt();
  eeprom_out[11] = (ui->currentLimitDedit->text().toInt())/2;
  eeprom_out[12] = ui->activeBrakeSlider->value();

  eeprom_out[17] = (char)ui->rvCheckBox->isChecked();
  eeprom_out[18] = (char)ui->biDirectionCheckbox->isChecked();
  eeprom_out[19] = (char)ui->sinCheckBox->isChecked();
  eeprom_out[20] = (char)ui->comp_pwmCheckbox->isChecked();

  if((char)ui->varPWMCheckBox->isChecked()){
    eeprom_out[21] = 0x01;
  } else if((char)ui->autoPWM->isChecked()){
    eeprom_out[21] = 0x02;
  }else{
    eeprom_out[21] = 0x00;
  }

  eeprom_out[22] = (char)ui->stuckProtectionBox->isChecked();
  eeprom_out[23] = (char)ui->timingAdvanceSlider->value();
  eeprom_out[24] = (uint8_t)ui->pwmFreqSlider->value();
  eeprom_out[25] = (char)ui->startupPowerSlider->value();
  eeprom_out[26] = (char)ui->motorKVSlider->value();
  eeprom_out[27] = (char)ui->motorPolesSlider->value();

  if((char)ui->brakecheckbox->isChecked()){
    eeprom_out[28] = 0x01;
  } else if ((char)ui->activeBrakeCheckbox->isChecked()){
    eeprom_out[28] = 0x02;
  }else{
    eeprom_out[28] = 0x00;
  }



  eeprom_out[29] = (char)ui->antiStallBox->isChecked();
  eeprom_out[30] = (char)ui->beepVolumeSlider->value();
  eeprom_out[31] = (char)ui->thirtymsTelemBox->isChecked();
  eeprom_out[32] = (char)ui->servoLowSlider->value();
  eeprom_out[33] = (char)ui->servoHighSlider->value();
  eeprom_out[34] = (char)ui->servoNeutralSlider->value();
  eeprom_out[35] = (char)ui->servoDeadBandSlider->value();
  eeprom_out[36] = (char)ui->lowVoltageCuttoffBox->isChecked();
  eeprom_out[37] = (char)ui->lowVoltageThresholdSlider->value();
  eeprom_out[38] = (char)ui->rcCarReverse->isChecked();
  eeprom_out[39] = (char)ui->hallSensorCheckbox->isChecked();
  eeprom_out[40] = (char)ui->sineStartupSlider->value();
  eeprom_out[41] = (char)ui->dragBrakeSlider->value();
  eeprom_out[42] = (char)ui->runningBrakeStrength->value();
  eeprom_out[43] = (char)ui->temperatureSlider->value();
  eeprom_out[44] = (char)ui->currentSlider->value();
  eeprom_out[45] = (char)ui->sineModePowerSlider->value();
  eeprom_out[46] = (char)ui->signalComboBox->currentIndex();
  eeprom_out[47] = (char)ui->AutoTimingButton->isChecked();



  QFileDialog::saveFileContent(eeprom_out, "am32_v3_config.bin");
  qInfo("file written");
   ui->configFileInfo->setText("Config File Saved");
}


void Widget::loadConfig(){
    QByteArray fileBuffer;
    uint16_t buffer_length = 48;
    if(firstOffline == true){
        for (int i = 0; i < 48; i++) {
            fileBuffer.append(air_starteeprom[i]);

        }
        firstOffline = false;
        //   override = 0;
    }else{


        filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                "c:", tr("All Files (*.bin)"));
        QFile inputFile(filename);
        inputFile.open(QIODevice::ReadOnly);
        if(inputFile.isOpen()){

            fileBuffer = inputFile.readAll();
        }
    }
    if ((fileBuffer.at(0) == (char)0x01)) {
        if ((fileBuffer.at(1) == (char)0xFF) ||
            (fileBuffer.at(2) ==
             (char)0x00)) { // if eeprom version is 0xff it means boot bit is set
            // but no defaults have been sent
            QMessageBox::warning( // also if bootloader version is 00 it means it has
                // been written
                this, tr("Application Name"),
                tr("No settings found use 'Send default EEPROM' under FLASH tab"));
            ui->eepromFrame->setHidden(true);
            ui->inputservoFrame->setHidden(true);
            ui->flashFourwayFrame->setHidden(false);
            ui->escStatusLabel_2->setText("Connected - No EEprom");
            ui->escStatusLabel->setText("Connected - No EEprom");
            //    return false;
        }
        if ((fileBuffer.at(1) <
             (char)0x01)) { // if eeprom version is 0xff it means boot bit is set
            // but no defaults have been sent
            QMessageBox::warning( // also if bootloader version is 00 it means it has
                // been written
                this, tr("Application Name"),
                tr("Outdated firmware detected 'Please update to lastest AM32 "
                   "release"));
            //   ui->sendFirstEEPROM->setHidden(true);
            ui->crawler_default_button->setHidden(true);
            ui->eepromFrame->setHidden(true);
            ui->inputservoFrame->setHidden(true);
            ui->flashFourwayFrame->setHidden(false);
            ui->escStatusLabel_2->setText("Connected - Firmware Update Required");
            ui->escStatusLabel->setText("Connected - Firmware Update Required");
            //    return false;
        }


        ui->maxRocSlider->setValue((uint8_t)(fileBuffer.at(5)));
        ui->minDutySlider->setValue((uint8_t)(fileBuffer.at(6)));
        if(fileBuffer.at(7) == 0x01){
            ui->disableStickCalibCheckbox->setChecked(true);
        }else{
            ui->disableStickCalibCheckbox->setChecked(false);
        }
        ui->absoluteVoltageSlider->setValue((uint8_t)(fileBuffer.at(8)));
        ui->currentLimitPedit->setText(QString::number((uint8_t)(fileBuffer.at(9))*2));
        ui->currentLimitIedit->setText(QString::number((uint8_t)(fileBuffer.at(10))));
        ui->currentLimitDedit->setText(QString::number((uint8_t)(fileBuffer.at(11))*2));
        ui->activeBrakeSlider->setValue((uint8_t)(fileBuffer.at(12)));

        if(fileBuffer.at(17) == 0x01){
            ui->rvCheckBox->setChecked(true);
        }else{
            ui->rvCheckBox->setChecked(false);
        }
        if (fileBuffer.at(18) == 0x01) {
            ui->biDirectionCheckbox->setChecked(true);
        } else {
            ui->biDirectionCheckbox->setChecked(false);
        }
        if (fileBuffer.at(19) == 0x01) {
            ui->sinCheckBox->setChecked(true);
        } else {
            ui->sinCheckBox->setChecked(false);
        }
        if (fileBuffer.at(20) == 0x01) {
            ui->comp_pwmCheckbox->setChecked(true);
        } else {
            ui->comp_pwmCheckbox->setChecked(false);
        }
        if (fileBuffer.at(21) == 0x01) {
            ui->varPWMCheckBox->setChecked(true);
        } else {
            ui->varPWMCheckBox->setChecked(false);
        }
        if (fileBuffer.at(21) == 0x02) {
            ui->autoPWM->setChecked(true);
        } else {
            ui->autoPWM->setChecked(false);
        }

        if (fileBuffer.at(22) == 0x01) {
            ui->stuckProtectionBox->setChecked(true);
        } else {
            ui->stuckProtectionBox->setChecked(false);
        }
        ui->timingAdvanceLCD->display(((fileBuffer.at(23))-10)*0.9375);
        ui->timingAdvanceSlider->setValue(fileBuffer.at(23));
        ui->pwmFreqSlider->setValue(fileBuffer.at(24));
        ui->startupPowerSlider->setValue((uint8_t)fileBuffer.at(25));
        ui->motorKVSlider->setValue((uint8_t)fileBuffer.at(26));
        ui->motorPolesSlider->setValue(fileBuffer.at(27));
        if (fileBuffer.at(28) == 0x01) {
            ui->brakecheckbox->setChecked(true);
        } else {
            ui->brakecheckbox->setChecked(false);
        }

        if (fileBuffer.at(28) == 0x02) {
            ui->activeBrakeCheckbox->setChecked(true);
        } else {
            ui->activeBrakeCheckbox->setChecked(false);
        }

        if (fileBuffer.at(29) == 0x01) {
            ui->antiStallBox->setChecked(true);
        } else {
            ui->antiStallBox->setChecked(false);
        }
        if (fileBuffer.at(1) == 0) { // if ESC is curently on eeprom version 0
            ui->beepVolumeSlider->setValue(5);
            ui->servoLowSlider->setValue(128);
            ui->servoHighSlider->setValue(128);
            ui->servoNeutralSlider->setValue(128);
            ui->servoDeadBandSlider->setValue(50);
            ui->thirtymsTelemBox->setChecked(false);
        } else {

            ui->beepVolumeSlider->setValue(fileBuffer.at(30));
            if (fileBuffer.at(31) == 0x01) {
                ui->thirtymsTelemBox->setChecked(true);
            } else {
                ui->thirtymsTelemBox->setChecked(false);
            }
            ui->servoLowSlider->setValue((uint8_t)(fileBuffer.at(32)));
            ui->servoHighSlider->setValue((uint8_t)(fileBuffer.at(33)));
            ui->servoNeutralSlider->setValue((uint8_t)(fileBuffer.at(34)));
            ui->servoDeadBandSlider->setValue((uint8_t)(fileBuffer.at(35)));

            if (fileBuffer.at(36) == 0x01) {
                ui->lowVoltageCuttoffBox->setChecked(true);
            } else {
                ui->lowVoltageCuttoffBox->setChecked(false);
            }
            ui->lowVoltageThresholdSlider->setValue((uint8_t)(fileBuffer.at(37)));
            if (fileBuffer.at(38) == 0x01) {
                ui->rcCarReverse->setChecked(true);
            } else {
                ui->rcCarReverse->setChecked(false);
            }
            if (fileBuffer.at(39) == 0x01) {
                ui->hallSensorCheckbox->setChecked(true);
            } else {
                ui->hallSensorCheckbox->setChecked(false);
            }
            ui->sineStartupSlider->setValue((uint8_t)(fileBuffer.at(40)));
            ui->dragBrakeSlider->setValue((uint8_t)(fileBuffer.at(41)));

            ui->runningBrakeStrength->setValue((uint8_t)(fileBuffer.at(42)));
            if ((uint8_t)(fileBuffer.at(45)) > 10) {
                ui->sineModePowerSlider->setValue(5);
            } else {
                ui->sineModePowerSlider->setValue((uint8_t)(fileBuffer.at(45)));
            }

            if ((uint8_t)(fileBuffer.at(43)) < 70) {
                ui->temperatureSlider->setValue(142);
            } else {
                ui->temperatureSlider->setValue((uint8_t)(fileBuffer.at(43)));
            }
            ui->currentSlider->setValue((uint8_t)(fileBuffer.at(44)));
            ui->signalComboBox->setCurrentIndex((uint8_t)(fileBuffer.at(46)));
        }

        QString name; // 2 bytes
        //    name.append(QChar(fileBuffer.at(5)));
        //    name.append(QChar(fileBuffer.at(6)));
        //    name.append(QChar(fileBuffer.at(7)));
        //    name.append(QChar(fileBuffer.at(8)));
        //    name.append(QChar(fileBuffer.at(9)));
        //    name.append(QChar(fileBuffer.at(10)));
        //    name.append(QChar(fileBuffer.at(11)));
        //    name.append(QChar(fileBuffer.at(12)));
        //    name.append(QChar(fileBuffer.at(13)));
        //    name.append(QChar(fileBuffer.at(14)));
        //    name.append(QChar(fileBuffer.at(15)));
        //    name.append(QChar(fileBuffer.at(16)));
        ui->FirmwareNameLabel->setText(name);
        if (auto *firmware = findChild<QLabel *>("webInfoValue0")) {
          firmware->setText(name);
        }

        QString version = "FW Rev:";
        QString major = QString::number((uint8_t)fileBuffer.at(3));
        QString minor = QString::number((uint8_t)fileBuffer.at(4));
        version.append(major);
        version.append(".");
        version.append(minor);
        ui->FirmwareVerionsLabel->setText(version);
        if (auto *eeprom = findChild<QLabel *>("webInfoValue1")) {
          eeprom->setText(version);
        }

        QChar charas = fileBuffer.at(18);
        int output = charas.toLatin1();
        qInfo(" output integer %i", output);
        eeprom_buffer->clear();
        //     eeprom_buffer = fileBuffer;
        for (int i = 0; i < buffer_length; i++) {
            eeprom_buffer->append(fileBuffer.at(i));
        }

        ui->configFileInfo->setText("Config File:" + filename);

    }

}



void Widget::on_loadConfigButton_clicked()
{
loadConfig();
}

void Widget::on_OfflineCheckBox_stateChanged(int arg1)
{
   if(ui->OfflineCheckBox->isChecked()){
     hide4wayButtons(false);
     hideESCSettings(false);
     hideEEPROMSettings(false);
     if (ui->tabWidget->count() == 5) {
       ui->tabWidget->removeTab(4);
       ui->tabWidget->removeTab(2);
       ui->tabWidget->removeTab(1);
       showSingleMotor(true);
     firstOffline = true;
       loadConfig();
       ui->writeEEPROM->setHidden(true);
       ui->writeEEPROM_2->setHidden(true);
   }

  }else{
    hide4wayButtons(true);
    hideESCSettings(true);
    hideEEPROMSettings(true);
     ui->tabWidget->insertTab(2, ui->tab_2, "Flash");
     ui->tabWidget->insertTab(2, ui->tab_3, "Motor Control");
     showSingleMotor(false);
     ui->writeEEPROM->setHidden(false);
     ui->writeEEPROM_2->setHidden(false);
  }
}


void Widget::on_maxRocSlider_valueChanged(int value)
{
  float dv = float(value);
  QString text = QString::number(dv/10, 'f', 1);
  ui->rocLineEdit->setText(text);
}


void Widget::on_minDutySlider_valueChanged(int value)
{
  float dv = float(value);
  QString text = QString::number(dv/2, 'f', 1);
  ui->minDutyLineEdit->setText(text);
}

void Widget::on_activeBrakeSlider_valueChanged(int value)
{
  QString text = QString::number(value);
  ui->activeBrakeLineEdit->setText(text);
}


void Widget::on_rocLineEdit_textEdited(const QString &arg1)
{

}


void Widget::on_minDutyLineEdit_editingFinished()
{
  QString arg1 = ui->minDutyLineEdit->text();
  if(arg1.toFloat() <= 25 && arg1.toFloat() >= 0){
  ui->minDutySlider->setValue(arg1.toFloat()*2);
  }else{
  qInfo("invalid input");
  ui->minDutySlider->setValue(0);
  }
}


void Widget::on_rocLineEdit_editingFinished()
{
  QString arg1 = ui->rocLineEdit->text();
  if(arg1.toFloat() <= 20 && arg1.toFloat() > 0){
  ui->maxRocSlider->setValue(arg1.toFloat() * 10);
  }else{
  qInfo("invalid input");
  ui->maxRocSlider->setValue(1);
  }
}





void Widget::on_activeBrakeLineEdit_editingFinished()
{
  QString arg1 = ui->activeBrakeLineEdit->text();
  if(arg1.toInt() <= 10 && arg1.toInt() > 0){
  ui->activeBrakeSlider->setValue(arg1.toInt());
  }else{
  qInfo("invalid input");
  ui->activeBrakeSlider->setValue(1);
  }
}


void Widget::on_lowVoltageCuttoffBox_stateChanged(int arg1)
{
  if(ui->lowVoltageCuttoffBox->isChecked()){
  ui->absoluteVotlageCheckbox->setEnabled(false);
  }else{
  ui->absoluteVotlageCheckbox->setEnabled(true);
  }
}


void Widget::on_absoluteVotlageCheckbox_stateChanged(int arg1)
{
  if(ui->absoluteVotlageCheckbox->isChecked()){
  ui->lowVoltageCuttoffBox->setEnabled(false);
  }else{
  ui->lowVoltageCuttoffBox->setEnabled(true);
  }
}


void Widget::on_brakecheckbox_stateChanged(int arg1)
{
  if(ui->brakecheckbox->isChecked()){
  ui->activeBrakeCheckbox->setEnabled(false);
  ui->dragBrakeSlider->setEnabled(true);
  }else{
  ui->activeBrakeCheckbox->setEnabled(true);
  ui->dragBrakeSlider->setEnabled(false);
  }
}


void Widget::on_activeBrakeCheckbox_stateChanged(int arg1)
{
  if(ui->activeBrakeCheckbox->isChecked()){
  ui->brakecheckbox->setEnabled(false);
  ui->activeBrakeSlider->setEnabled(true);
  }else{
  ui->brakecheckbox->setEnabled(true);
  ui->activeBrakeSlider->setEnabled(false);
  }
}


void Widget::on_absoluteVoltageSlider_valueChanged(int value)
{
  QString text = QString::number(value);
  ui->absoluateVoltageLineedit->setText(text);
}


void Widget::on_currentLimitPedit_editingFinished()
{
  QString arg1 = ui->currentLimitPedit->text();
  if(arg1.toInt() <= 500 && arg1.toInt() >= 0){
  }else{
  qInfo("invalid input");
  ui->currentLimitPedit->setText("100");
  }
}


void Widget::on_currentLimitDedit_editingFinished()
{
      QString arg1 = ui->currentLimitDedit->text();
  if(arg1.toInt() <= 500 && arg1.toInt() >=0){
  }else{
  qInfo("invalid input");
  ui->currentLimitDedit->setText("100");
  }
}


void Widget::on_currentLimitIedit_editingFinished()
{
      QString arg1 = ui->currentLimitIedit->text();
  if(arg1.toInt() <= 255 && arg1.toInt() >=0){
  }else{
  qInfo("invalid input");
  ui->currentLimitIedit->setText("0");
  }
}


// void Widget::on_uploadMusic_clicked()
// {


//   //  QString data = ui->musicLineEdit->text();


//     QString str = ui->MusicTextEdit->toPlainText();
//     QStringList parts = str.split(",", Qt::SkipEmptyParts);
//     uint8_t buffersize = parts.size();
//     uint nHex;
// bool Status2 = false;
//     QByteArray eeprom_music_out;
//         for (int i = 0; i < 48; i++) {
//             eeprom_music_out.append((char)eeprom_buffer->at(i));
//         }
//          for (int i = 0; i < buffersize; i++) {
//              qInfo(parts[i].toLatin1());
//              Status2 = false;
//              nHex = parts[i].toInt();
//     //         nHex = parts[i].toUInt(&Status2,16);
//              eeprom_music_out.append(nHex);
//             // qInfo(" output integer %i", nHex);
//          }
// qInfo(" buffersize: %i", buffersize);

// //qInfo(" eepromout: %i", eeprom_music_out[48]);
// //qInfo(" eepromout: %i", eeprom_music_out[49]);
// //qInfo(" eepromout: %i", eeprom_music_out[50]);
//     //    uint nHex = parts[1].toUInt(&Status2,16);

// //        qInfo(parts[0].toLatin1());
//   //      qInfo(parts[1].toLatin1());
//   //      qInfo(parts[2].toLatin1());
//   //      qInfo(parts[3].toLatin1());
//  //       qInfo(" output integer %i", nHex);
//   //      qInfo(" output integer %i", parts[2].toInt());
//   //      qInfo(" output integer %i", parts[3].toInt());
//  //       qInfo(" output integer %i", parts[4].toInt());
//   //      qInfo(" output integer %i", parts[5].toInt());

//     four_way->ack_required = true;

//     if (four_way->direct) {
//         sendDirect(eeprom_music_out, 48+buffersize, four_way->eeprom_address);
//     } else {
//         writeData(four_way->makeFourWayWriteCommand(eeprom_music_out, 48+buffersize,
//                                                     four_way->eeprom_address));
//         m_serial->waitForBytesWritten(1000);
//         while (m_serial->waitForReadyRead(1000)) {
//         }

//         readData();
//     }
//     if (four_way->ack_required == false) { // good ack received from esc
//         ui->escStatusLabel->setText("WRITE DEFAULT SUCCESS");
//     }
//     ui->eepromFrame->setHidden(true);
//     ui->inputservoFrame->setHidden(true);
// }


void Widget::on_initMotor1_4_clicked(){ on_initMotor1_clicked(); }


void Widget::on_initMotor2_4_clicked(){ on_initMotor2_clicked(); }


void Widget::on_initMotor3_4_clicked(){ on_initMotor3_clicked(); }


void Widget::on_initMotor4_4_clicked(){ on_initMotor4_clicked(); }
