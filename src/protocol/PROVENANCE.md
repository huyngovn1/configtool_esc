# Protocol helper provenance

These four files adapt the packet construction and CRC behavior of the
user-supplied `BF_ROOTLOADER.cpp`, `BF_ROOTLOADER.h`, `fourwayif.cpp`, and
`fourwayif.h` from the user's `Offline-Configurator-main` folder. No original UI,
generated UI code, application project, or assets are included here.

The supplied files contain no copyright or license notice. No LICENSE or COPYING
file was found in that supplied project tree. This document does not grant or
assert a third-party license; retain applicable upstream notices if established.

Changes in this adaptation:

- Qt Core (`QByteArray`) only; removed the unused `QMessageBox` dependency.
- Complete header guards and deterministic initial state.
- Implemented previously missing ACK methods and the missing ACK return value.
- Explicit unsigned CRC byte handling; no packed union or host-endian assumption.
- Bounds checks before indexing buffers; invalid construction returns an empty
  byte array and invalid CRC input returns false.
- Write lengths must match the actual payload. A length of 0 or 256 denotes a
  256-byte transfer; other permitted transfer lengths are 1 through 255.
- Explicit compatibility fix: direct `SET_BUFFER` for 256 bytes now uses
  `FE 00 01 00` before CRC. The supplied helper emitted `FE 00 00 00`, which
  requests zero bytes from the user's AT32F421 bootloader. Its `Src/main.c`,
  lines 344-356, selects 256 only when byte 2 is 1. API size 0 is retained as
  an alias for 256. This does not affect the app's 48-byte settings writes.
- Original commands, address byte order, CRC algorithm and CRC byte order remain:
  direct bootloader uses CRC-16/ARC low byte first; 4-way uses CRC-16/XMODEM high
  byte first. Address conversion for MCU families belongs to the session layer.

The `protocol_tests` executable checks fixed packet and CRC vectors without a
serial port. It is a software compatibility check, not hardware validation.
