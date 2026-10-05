#ifndef BASIC_ESC_MEMORYADDRESS_H
#define BASIC_ESC_MEMORYADDRESS_H

#include <QtGlobal>

namespace EscMemory {
// G071 uses word-addressed SET_ADDRESS (shift 2); other supported loaders use
// byte addresses. Read lengths always count bytes in both cases.
constexpr quint16 identityAddress(quint16 eepromWireAddress, quint8 addressShift)
{
    return quint16(eepromWireAddress - (32u >> addressShift));
}
}

#endif
