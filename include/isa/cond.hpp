#pragma once
#include <cstdint>

enum class Cond : uint8_t {
    EQ = 0,  // Equal (ZF=1)
    NE = 1,  // Not Equal (ZF=0)
    LT = 2,  // Less Than (SF≠OF)
    LE = 3,  // Less or Equal (ZF=1 OR SF≠OF)
    GT = 4,  // Greater Than (ZF=0 AND SF=OF)
    GE = 5,  // Greater or Equal (SF=OF)
    CS = 6,  // Carry Set (CF=1)
    CC = 7,  // Carry Clear (CF=0)
    INVALID = 0xFF
};