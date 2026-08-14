/****************************************************************************
** Meta object code from reading C++ file 'widget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../widget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_Widget_t {
    QByteArrayData data[99];
    char stringdata0[2598];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Widget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Widget_t qt_meta_stringdata_Widget = {
    {
QT_MOC_LITERAL(0, 0, 6), // "Widget"
QT_MOC_LITERAL(1, 7, 20), // "mspSerialChecksumBuf"
QT_MOC_LITERAL(2, 28, 7), // "uint8_t"
QT_MOC_LITERAL(3, 36, 0), // ""
QT_MOC_LITERAL(4, 37, 8), // "checksum"
QT_MOC_LITERAL(5, 46, 14), // "const uint8_t*"
QT_MOC_LITERAL(6, 61, 4), // "data"
QT_MOC_LITERAL(7, 66, 3), // "len"
QT_MOC_LITERAL(8, 70, 15), // "send_mspCommand"
QT_MOC_LITERAL(9, 86, 3), // "cmd"
QT_MOC_LITERAL(10, 90, 7), // "payload"
QT_MOC_LITERAL(11, 98, 21), // "on_sendButton_clicked"
QT_MOC_LITERAL(12, 120, 27), // "on_disconnectButton_clicked"
QT_MOC_LITERAL(13, 148, 21), // "on_pushButton_clicked"
QT_MOC_LITERAL(14, 170, 28), // "on_sendMessageButton_clicked"
QT_MOC_LITERAL(15, 199, 27), // "on_passthoughButton_clicked"
QT_MOC_LITERAL(16, 227, 31), // "on_horizontalSlider_sliderMoved"
QT_MOC_LITERAL(17, 259, 8), // "position"
QT_MOC_LITERAL(18, 268, 39), // "on_serialSelectorBox_currentT..."
QT_MOC_LITERAL(19, 308, 4), // "arg1"
QT_MOC_LITERAL(20, 313, 23), // "on_pushButton_2_clicked"
QT_MOC_LITERAL(21, 337, 21), // "on_loadBinary_clicked"
QT_MOC_LITERAL(22, 359, 22), // "on_writeBinary_clicked"
QT_MOC_LITERAL(23, 382, 22), // "on_VerifyFlash_clicked"
QT_MOC_LITERAL(24, 405, 21), // "on_initMotor2_clicked"
QT_MOC_LITERAL(25, 427, 21), // "on_initMotor1_clicked"
QT_MOC_LITERAL(26, 449, 21), // "on_initMotor3_clicked"
QT_MOC_LITERAL(27, 471, 21), // "on_initMotor4_clicked"
QT_MOC_LITERAL(28, 493, 22), // "on_writeEEPROM_clicked"
QT_MOC_LITERAL(29, 516, 26), // "on_sendFirstEEPROM_clicked"
QT_MOC_LITERAL(30, 543, 27), // "on_devSettings_stateChanged"
QT_MOC_LITERAL(31, 571, 25), // "on_endPassthrough_clicked"
QT_MOC_LITERAL(32, 597, 14), // "convertFromHex"
QT_MOC_LITERAL(33, 612, 24), // "on_checkBox_stateChanged"
QT_MOC_LITERAL(34, 637, 23), // "on_initMotor1_2_clicked"
QT_MOC_LITERAL(35, 661, 23), // "on_initMotor2_2_clicked"
QT_MOC_LITERAL(36, 685, 23), // "on_initMotor3_2_clicked"
QT_MOC_LITERAL(37, 709, 23), // "on_initMotor4_2_clicked"
QT_MOC_LITERAL(38, 733, 34), // "on_startupPowerSlider_valueCh..."
QT_MOC_LITERAL(39, 768, 5), // "value"
QT_MOC_LITERAL(40, 774, 35), // "on_timingAdvanceSlider_valueC..."
QT_MOC_LITERAL(41, 810, 29), // "on_pwmFreqSlider_valueChanged"
QT_MOC_LITERAL(42, 840, 30), // "on_varPWMCheckBox_stateChanged"
QT_MOC_LITERAL(43, 871, 27), // "on_m1MSPSlider_valueChanged"
QT_MOC_LITERAL(44, 899, 27), // "on_m2MSPSlider_valueChanged"
QT_MOC_LITERAL(45, 927, 27), // "on_m3MSPSlider_valueChanged"
QT_MOC_LITERAL(46, 955, 27), // "on_m4MSPSlider_valueChanged"
QT_MOC_LITERAL(47, 983, 26), // "on_ConnectedButton_clicked"
QT_MOC_LITERAL(48, 1010, 29), // "on_motorKVSlider_valueChanged"
QT_MOC_LITERAL(49, 1040, 32), // "on_motorPolesSlider_valueChanged"
QT_MOC_LITERAL(50, 1073, 32), // "on_beepVolumeSlider_valueChanged"
QT_MOC_LITERAL(51, 1106, 39), // "on_lowThresholdLineEdit_editi..."
QT_MOC_LITERAL(52, 1146, 30), // "on_servoLowSlider_valueChanged"
QT_MOC_LITERAL(53, 1177, 31), // "on_servoHighSlider_valueChanged"
QT_MOC_LITERAL(54, 1209, 34), // "on_servoNeutralSlider_valueCh..."
QT_MOC_LITERAL(55, 1244, 41), // "on_lowVoltageThresholdSlider_..."
QT_MOC_LITERAL(56, 1286, 40), // "on_highThresholdLineEdit_edit..."
QT_MOC_LITERAL(57, 1327, 38), // "on_servoNeuralLineEdit_editin..."
QT_MOC_LITERAL(58, 1366, 35), // "on_servoDeadBandSlider_valueC..."
QT_MOC_LITERAL(59, 1402, 40), // "on_servoDeadbandLineEdit_edit..."
QT_MOC_LITERAL(60, 1443, 37), // "on_lowVoltageLineEdit_editing..."
QT_MOC_LITERAL(61, 1481, 24), // "on_writeEEPROM_2_clicked"
QT_MOC_LITERAL(62, 1506, 23), // "on_initMotor1_3_clicked"
QT_MOC_LITERAL(63, 1530, 23), // "on_initMotor2_3_clicked"
QT_MOC_LITERAL(64, 1554, 23), // "on_initMotor3_3_clicked"
QT_MOC_LITERAL(65, 1578, 23), // "on_initMotor4_3_clicked"
QT_MOC_LITERAL(66, 1602, 15), // "serialInfoStuff"
QT_MOC_LITERAL(67, 1618, 8), // "resetESC"
QT_MOC_LITERAL(68, 1627, 10), // "loadConfig"
QT_MOC_LITERAL(69, 1638, 31), // "on_dragBrakeSlider_valueChanged"
QT_MOC_LITERAL(70, 1670, 33), // "on_sineStartupSlider_valueCha..."
QT_MOC_LITERAL(71, 1704, 35), // "on_sineModePowerSlider_valueC..."
QT_MOC_LITERAL(72, 1740, 36), // "on_runningBrakeStrength_value..."
QT_MOC_LITERAL(73, 1777, 29), // "on_currentSlider_valueChanged"
QT_MOC_LITERAL(74, 1807, 33), // "on_temperatureSlider_valueCha..."
QT_MOC_LITERAL(75, 1841, 33), // "on_crawler_default_button_cli..."
QT_MOC_LITERAL(76, 1875, 27), // "on_saveConfigButton_clicked"
QT_MOC_LITERAL(77, 1903, 27), // "on_loadConfigButton_clicked"
QT_MOC_LITERAL(78, 1931, 31), // "on_OfflineCheckBox_stateChanged"
QT_MOC_LITERAL(79, 1963, 28), // "on_maxRocSlider_valueChanged"
QT_MOC_LITERAL(80, 1992, 29), // "on_minDutySlider_valueChanged"
QT_MOC_LITERAL(81, 2022, 25), // "on_rocLineEdit_textEdited"
QT_MOC_LITERAL(82, 2048, 34), // "on_minDutyLineEdit_editingFin..."
QT_MOC_LITERAL(83, 2083, 30), // "on_rocLineEdit_editingFinished"
QT_MOC_LITERAL(84, 2114, 33), // "on_activeBrakeSlider_valueCha..."
QT_MOC_LITERAL(85, 2148, 38), // "on_activeBrakeLineEdit_editin..."
QT_MOC_LITERAL(86, 2187, 36), // "on_lowVoltageCuttoffBox_state..."
QT_MOC_LITERAL(87, 2224, 39), // "on_absoluteVotlageCheckbox_st..."
QT_MOC_LITERAL(88, 2264, 29), // "on_brakecheckbox_stateChanged"
QT_MOC_LITERAL(89, 2294, 35), // "on_activeBrakeCheckbox_stateC..."
QT_MOC_LITERAL(90, 2330, 37), // "on_absoluteVoltageSlider_valu..."
QT_MOC_LITERAL(91, 2368, 36), // "on_currentLimitPedit_editingF..."
QT_MOC_LITERAL(92, 2405, 36), // "on_currentLimitDedit_editingF..."
QT_MOC_LITERAL(93, 2442, 36), // "on_currentLimitIedit_editingF..."
QT_MOC_LITERAL(94, 2479, 22), // "on_uploadMusic_clicked"
QT_MOC_LITERAL(95, 2502, 23), // "on_initMotor1_4_clicked"
QT_MOC_LITERAL(96, 2526, 23), // "on_initMotor2_4_clicked"
QT_MOC_LITERAL(97, 2550, 23), // "on_initMotor3_4_clicked"
QT_MOC_LITERAL(98, 2574, 23) // "on_initMotor4_4_clicked"

    },
    "Widget\0mspSerialChecksumBuf\0uint8_t\0"
    "\0checksum\0const uint8_t*\0data\0len\0"
    "send_mspCommand\0cmd\0payload\0"
    "on_sendButton_clicked\0on_disconnectButton_clicked\0"
    "on_pushButton_clicked\0"
    "on_sendMessageButton_clicked\0"
    "on_passthoughButton_clicked\0"
    "on_horizontalSlider_sliderMoved\0"
    "position\0on_serialSelectorBox_currentTextChanged\0"
    "arg1\0on_pushButton_2_clicked\0"
    "on_loadBinary_clicked\0on_writeBinary_clicked\0"
    "on_VerifyFlash_clicked\0on_initMotor2_clicked\0"
    "on_initMotor1_clicked\0on_initMotor3_clicked\0"
    "on_initMotor4_clicked\0on_writeEEPROM_clicked\0"
    "on_sendFirstEEPROM_clicked\0"
    "on_devSettings_stateChanged\0"
    "on_endPassthrough_clicked\0convertFromHex\0"
    "on_checkBox_stateChanged\0"
    "on_initMotor1_2_clicked\0on_initMotor2_2_clicked\0"
    "on_initMotor3_2_clicked\0on_initMotor4_2_clicked\0"
    "on_startupPowerSlider_valueChanged\0"
    "value\0on_timingAdvanceSlider_valueChanged\0"
    "on_pwmFreqSlider_valueChanged\0"
    "on_varPWMCheckBox_stateChanged\0"
    "on_m1MSPSlider_valueChanged\0"
    "on_m2MSPSlider_valueChanged\0"
    "on_m3MSPSlider_valueChanged\0"
    "on_m4MSPSlider_valueChanged\0"
    "on_ConnectedButton_clicked\0"
    "on_motorKVSlider_valueChanged\0"
    "on_motorPolesSlider_valueChanged\0"
    "on_beepVolumeSlider_valueChanged\0"
    "on_lowThresholdLineEdit_editingFinished\0"
    "on_servoLowSlider_valueChanged\0"
    "on_servoHighSlider_valueChanged\0"
    "on_servoNeutralSlider_valueChanged\0"
    "on_lowVoltageThresholdSlider_valueChanged\0"
    "on_highThresholdLineEdit_editingFinished\0"
    "on_servoNeuralLineEdit_editingFinished\0"
    "on_servoDeadBandSlider_valueChanged\0"
    "on_servoDeadbandLineEdit_editingFinished\0"
    "on_lowVoltageLineEdit_editingFinished\0"
    "on_writeEEPROM_2_clicked\0"
    "on_initMotor1_3_clicked\0on_initMotor2_3_clicked\0"
    "on_initMotor3_3_clicked\0on_initMotor4_3_clicked\0"
    "serialInfoStuff\0resetESC\0loadConfig\0"
    "on_dragBrakeSlider_valueChanged\0"
    "on_sineStartupSlider_valueChanged\0"
    "on_sineModePowerSlider_valueChanged\0"
    "on_runningBrakeStrength_valueChanged\0"
    "on_currentSlider_valueChanged\0"
    "on_temperatureSlider_valueChanged\0"
    "on_crawler_default_button_clicked\0"
    "on_saveConfigButton_clicked\0"
    "on_loadConfigButton_clicked\0"
    "on_OfflineCheckBox_stateChanged\0"
    "on_maxRocSlider_valueChanged\0"
    "on_minDutySlider_valueChanged\0"
    "on_rocLineEdit_textEdited\0"
    "on_minDutyLineEdit_editingFinished\0"
    "on_rocLineEdit_editingFinished\0"
    "on_activeBrakeSlider_valueChanged\0"
    "on_activeBrakeLineEdit_editingFinished\0"
    "on_lowVoltageCuttoffBox_stateChanged\0"
    "on_absoluteVotlageCheckbox_stateChanged\0"
    "on_brakecheckbox_stateChanged\0"
    "on_activeBrakeCheckbox_stateChanged\0"
    "on_absoluteVoltageSlider_valueChanged\0"
    "on_currentLimitPedit_editingFinished\0"
    "on_currentLimitDedit_editingFinished\0"
    "on_currentLimitIedit_editingFinished\0"
    "on_uploadMusic_clicked\0on_initMotor1_4_clicked\0"
    "on_initMotor2_4_clicked\0on_initMotor3_4_clicked\0"
    "on_initMotor4_4_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Widget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      87,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    3,  449,    3, 0x08 /* Private */,
       8,    2,  456,    3, 0x08 /* Private */,
      11,    0,  461,    3, 0x08 /* Private */,
      12,    0,  462,    3, 0x08 /* Private */,
      13,    0,  463,    3, 0x08 /* Private */,
      14,    0,  464,    3, 0x08 /* Private */,
      15,    0,  465,    3, 0x08 /* Private */,
      16,    1,  466,    3, 0x08 /* Private */,
      18,    1,  469,    3, 0x08 /* Private */,
      20,    0,  472,    3, 0x08 /* Private */,
      21,    0,  473,    3, 0x08 /* Private */,
      22,    0,  474,    3, 0x08 /* Private */,
      23,    0,  475,    3, 0x08 /* Private */,
      24,    0,  476,    3, 0x08 /* Private */,
      25,    0,  477,    3, 0x08 /* Private */,
      26,    0,  478,    3, 0x08 /* Private */,
      27,    0,  479,    3, 0x08 /* Private */,
      28,    0,  480,    3, 0x08 /* Private */,
      29,    0,  481,    3, 0x08 /* Private */,
      30,    1,  482,    3, 0x08 /* Private */,
      31,    0,  485,    3, 0x08 /* Private */,
      32,    0,  486,    3, 0x08 /* Private */,
      33,    1,  487,    3, 0x08 /* Private */,
      34,    0,  490,    3, 0x08 /* Private */,
      35,    0,  491,    3, 0x08 /* Private */,
      36,    0,  492,    3, 0x08 /* Private */,
      37,    0,  493,    3, 0x08 /* Private */,
      38,    1,  494,    3, 0x08 /* Private */,
      40,    1,  497,    3, 0x08 /* Private */,
      41,    1,  500,    3, 0x08 /* Private */,
      42,    1,  503,    3, 0x08 /* Private */,
      43,    1,  506,    3, 0x08 /* Private */,
      44,    1,  509,    3, 0x08 /* Private */,
      45,    1,  512,    3, 0x08 /* Private */,
      46,    1,  515,    3, 0x08 /* Private */,
      47,    0,  518,    3, 0x08 /* Private */,
      48,    1,  519,    3, 0x08 /* Private */,
      49,    1,  522,    3, 0x08 /* Private */,
      50,    1,  525,    3, 0x08 /* Private */,
      51,    0,  528,    3, 0x08 /* Private */,
      52,    1,  529,    3, 0x08 /* Private */,
      53,    1,  532,    3, 0x08 /* Private */,
      54,    1,  535,    3, 0x08 /* Private */,
      55,    1,  538,    3, 0x08 /* Private */,
      56,    0,  541,    3, 0x08 /* Private */,
      57,    0,  542,    3, 0x08 /* Private */,
      58,    1,  543,    3, 0x08 /* Private */,
      59,    0,  546,    3, 0x08 /* Private */,
      60,    0,  547,    3, 0x08 /* Private */,
      61,    0,  548,    3, 0x08 /* Private */,
      62,    0,  549,    3, 0x08 /* Private */,
      63,    0,  550,    3, 0x08 /* Private */,
      64,    0,  551,    3, 0x08 /* Private */,
      65,    0,  552,    3, 0x08 /* Private */,
      66,    0,  553,    3, 0x08 /* Private */,
      67,    0,  554,    3, 0x08 /* Private */,
      68,    0,  555,    3, 0x08 /* Private */,
      69,    1,  556,    3, 0x08 /* Private */,
      70,    1,  559,    3, 0x08 /* Private */,
      71,    1,  562,    3, 0x08 /* Private */,
      72,    1,  565,    3, 0x08 /* Private */,
      73,    1,  568,    3, 0x08 /* Private */,
      74,    1,  571,    3, 0x08 /* Private */,
      75,    0,  574,    3, 0x08 /* Private */,
      76,    0,  575,    3, 0x08 /* Private */,
      77,    0,  576,    3, 0x08 /* Private */,
      78,    1,  577,    3, 0x08 /* Private */,
      79,    1,  580,    3, 0x08 /* Private */,
      80,    1,  583,    3, 0x08 /* Private */,
      81,    1,  586,    3, 0x08 /* Private */,
      82,    0,  589,    3, 0x08 /* Private */,
      83,    0,  590,    3, 0x08 /* Private */,
      84,    1,  591,    3, 0x08 /* Private */,
      85,    0,  594,    3, 0x08 /* Private */,
      86,    1,  595,    3, 0x08 /* Private */,
      87,    1,  598,    3, 0x08 /* Private */,
      88,    1,  601,    3, 0x08 /* Private */,
      89,    1,  604,    3, 0x08 /* Private */,
      90,    1,  607,    3, 0x08 /* Private */,
      91,    0,  610,    3, 0x08 /* Private */,
      92,    0,  611,    3, 0x08 /* Private */,
      93,    0,  612,    3, 0x08 /* Private */,
      94,    0,  613,    3, 0x08 /* Private */,
      95,    0,  614,    3, 0x08 /* Private */,
      96,    0,  615,    3, 0x08 /* Private */,
      97,    0,  616,    3, 0x08 /* Private */,
      98,    0,  617,    3, 0x08 /* Private */,

 // slots: parameters
    0x80000000 | 2, 0x80000000 | 2, 0x80000000 | 5, QMetaType::Int,    4,    6,    7,
    QMetaType::Void, 0x80000000 | 2, QMetaType::QByteArray,    9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::QString,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void,
    QMetaType::QByteArray,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void, QMetaType::QString,   19,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, QMetaType::Int,   19,
    QMetaType::Void, QMetaType::Int,   39,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Widget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Widget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: { uint8_t _r = _t->mspSerialChecksumBuf((*reinterpret_cast< uint8_t(*)>(_a[1])),(*reinterpret_cast< const uint8_t*(*)>(_a[2])),(*reinterpret_cast< int(*)>(_a[3])));
            if (_a[0]) *reinterpret_cast< uint8_t*>(_a[0]) = std::move(_r); }  break;
        case 1: _t->send_mspCommand((*reinterpret_cast< uint8_t(*)>(_a[1])),(*reinterpret_cast< QByteArray(*)>(_a[2]))); break;
        case 2: _t->on_sendButton_clicked(); break;
        case 3: _t->on_disconnectButton_clicked(); break;
        case 4: _t->on_pushButton_clicked(); break;
        case 5: _t->on_sendMessageButton_clicked(); break;
        case 6: _t->on_passthoughButton_clicked(); break;
        case 7: _t->on_horizontalSlider_sliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->on_serialSelectorBox_currentTextChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->on_pushButton_2_clicked(); break;
        case 10: _t->on_loadBinary_clicked(); break;
        case 11: _t->on_writeBinary_clicked(); break;
        case 12: _t->on_VerifyFlash_clicked(); break;
        case 13: _t->on_initMotor2_clicked(); break;
        case 14: _t->on_initMotor1_clicked(); break;
        case 15: _t->on_initMotor3_clicked(); break;
        case 16: _t->on_initMotor4_clicked(); break;
        case 17: _t->on_writeEEPROM_clicked(); break;
        case 18: _t->on_sendFirstEEPROM_clicked(); break;
        case 19: _t->on_devSettings_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 20: _t->on_endPassthrough_clicked(); break;
        case 21: { QByteArray _r = _t->convertFromHex();
            if (_a[0]) *reinterpret_cast< QByteArray*>(_a[0]) = std::move(_r); }  break;
        case 22: _t->on_checkBox_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 23: _t->on_initMotor1_2_clicked(); break;
        case 24: _t->on_initMotor2_2_clicked(); break;
        case 25: _t->on_initMotor3_2_clicked(); break;
        case 26: _t->on_initMotor4_2_clicked(); break;
        case 27: _t->on_startupPowerSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 28: _t->on_timingAdvanceSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 29: _t->on_pwmFreqSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 30: _t->on_varPWMCheckBox_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 31: _t->on_m1MSPSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 32: _t->on_m2MSPSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 33: _t->on_m3MSPSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 34: _t->on_m4MSPSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 35: _t->on_ConnectedButton_clicked(); break;
        case 36: _t->on_motorKVSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 37: _t->on_motorPolesSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 38: _t->on_beepVolumeSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 39: _t->on_lowThresholdLineEdit_editingFinished(); break;
        case 40: _t->on_servoLowSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 41: _t->on_servoHighSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 42: _t->on_servoNeutralSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 43: _t->on_lowVoltageThresholdSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 44: _t->on_highThresholdLineEdit_editingFinished(); break;
        case 45: _t->on_servoNeuralLineEdit_editingFinished(); break;
        case 46: _t->on_servoDeadBandSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 47: _t->on_servoDeadbandLineEdit_editingFinished(); break;
        case 48: _t->on_lowVoltageLineEdit_editingFinished(); break;
        case 49: _t->on_writeEEPROM_2_clicked(); break;
        case 50: _t->on_initMotor1_3_clicked(); break;
        case 51: _t->on_initMotor2_3_clicked(); break;
        case 52: _t->on_initMotor3_3_clicked(); break;
        case 53: _t->on_initMotor4_3_clicked(); break;
        case 54: _t->serialInfoStuff(); break;
        case 55: _t->resetESC(); break;
        case 56: _t->loadConfig(); break;
        case 57: _t->on_dragBrakeSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 58: _t->on_sineStartupSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 59: _t->on_sineModePowerSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 60: _t->on_runningBrakeStrength_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 61: _t->on_currentSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 62: _t->on_temperatureSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 63: _t->on_crawler_default_button_clicked(); break;
        case 64: _t->on_saveConfigButton_clicked(); break;
        case 65: _t->on_loadConfigButton_clicked(); break;
        case 66: _t->on_OfflineCheckBox_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 67: _t->on_maxRocSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 68: _t->on_minDutySlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 69: _t->on_rocLineEdit_textEdited((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 70: _t->on_minDutyLineEdit_editingFinished(); break;
        case 71: _t->on_rocLineEdit_editingFinished(); break;
        case 72: _t->on_activeBrakeSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 73: _t->on_activeBrakeLineEdit_editingFinished(); break;
        case 74: _t->on_lowVoltageCuttoffBox_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 75: _t->on_absoluteVotlageCheckbox_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 76: _t->on_brakecheckbox_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 77: _t->on_activeBrakeCheckbox_stateChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 78: _t->on_absoluteVoltageSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 79: _t->on_currentLimitPedit_editingFinished(); break;
        case 80: _t->on_currentLimitDedit_editingFinished(); break;
        case 81: _t->on_currentLimitIedit_editingFinished(); break;
        case 82: _t->on_uploadMusic_clicked(); break;
        case 83: _t->on_initMotor1_4_clicked(); break;
        case 84: _t->on_initMotor2_4_clicked(); break;
        case 85: _t->on_initMotor3_4_clicked(); break;
        case 86: _t->on_initMotor4_4_clicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject Widget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_Widget.data,
    qt_meta_data_Widget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *Widget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Widget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_Widget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int Widget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 87)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 87;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 87)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 87;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
