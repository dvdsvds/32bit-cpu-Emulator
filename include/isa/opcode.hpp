#pragma once
#include <cstdint>

enum class Opcode : uint8_t {
    ADD = 0,
    SUB = 1,
    AND = 2,
    OR = 3,
    XOR = 4,
    SHL = 5,
    SHR = 6,
    SAR = 7,
    MUL = 8,
    DIV = 9,
    CMP = 10,
    MOV = 11,
    LUI = 12,
    AUI = 13,
    LOADI = 14,
    LOADW = 15,
    LOADB = 16,
    STOREW = 17,
    STOREB = 18,
    JMP = 19,
    BJMP = 20,
    CALL = 21,
    RET = 22,
    HLT = 23,
    NOP = 24,
    SYSCALL = 25,
    TRAP = 26,
    IRET = 27,
    CSRR = 28,
    CSRW = 29,
    CSRWI = 30,
    PUSH = 31,
    POP = 32,
    CALLR = 33,
    INVALID = 0xFF
};

enum class InstrGroup : uint8_t {
    ARITH,
    COMPARE,
    MOVE,
    MEMORY,
    BRANCH,
    JUMP,
    SYSTEM,
    CSR,
    INVALID
};