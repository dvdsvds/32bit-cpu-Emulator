#pragma once

#include "types.hpp"

enum class Csr : uint8_t {
    EPC = 0,
    CAUSE,
    STATUS,
    IVTBR,
    IMASK,
    IPENDING,
    SEPC,
    SSTATUS,
    SCRATCH,
    CYCLE,
    TPERIOD = 10,
    TCONTROL = 11,
    TCOUNTER = 12,
    EFLAGS = 13,
    SP = 14,
};

static constexpr size_t CSR_COUNT = 15;

// MODE
static constexpr size_t CSR_MODE_LSB = 2;
static constexpr size_t CSR_MODE_SIZE = 1;

// STATUS
static constexpr size_t IE_LSB = 0;
static constexpr size_t IE_SIZE = 1;
static constexpr size_t PIE_LSB = 1;
static constexpr size_t PIE_SIZE = 1;

// CAUSE
static constexpr size_t CODE_LSB = 0;
static constexpr size_t CODE_SIZE = 5;
static constexpr size_t INTERRUPT_LSB = 31;
static constexpr size_t INTERRUPT_SIZE = 1;

// IMASK
static constexpr size_t TIM_LSB = 0;
static constexpr size_t TIM_SIZE = 1;
static constexpr size_t EXT0_LSB = 1;
static constexpr size_t EXT0_SIZE = 1;
static constexpr size_t EXT1_LSB = 2;
static constexpr size_t EXT1_SIZE = 1;
static constexpr size_t EXT2_LSB = 3;
static constexpr size_t EXT2_SIZE = 1;
static constexpr size_t EXT3_LSB = 4;
static constexpr size_t EXT3_SIZE = 1;
static constexpr size_t SWI_LSB = 5;
static constexpr size_t SWI_SIZE = 1;

// IPENDING
static constexpr size_t TIP_LSB = 0;
static constexpr size_t TIP_SIZE = 1;
static constexpr size_t EXT0P_LSB = 1;
static constexpr size_t EXT0P_SIZE = 1;
static constexpr size_t EXT1P_LSB = 2;
static constexpr size_t EXT1P_SIZE = 1;
static constexpr size_t EXT2P_LSB = 3;
static constexpr size_t EXT2P_SIZE = 1;
static constexpr size_t EXT3P_LSB = 4;
static constexpr size_t EXT3P_SIZE = 1;
static constexpr size_t SWIP_LSB = 5;
static constexpr size_t SWIP_SIZE = 1;

// TCONTROL
static constexpr size_t ENABLE_LSB = 0;
static constexpr size_t ENABLE_SIZE = 1;
static constexpr size_t TMODE_LSB = 1;
static constexpr size_t TMODE_SIZE = 1;

enum class Interrupt : uint8_t {
    TIMER = 0,
    EXT0,
    EXT1,
    EXT2,
    EXT3,
    SWI
};

enum class Exception : uint8_t {
    INVALID_OPCODE = 0,
    DIVISION_BY_ZERO,
    ALIGNMENT_ERROR,
    ACCESS_VIOLATION,
    BREAKPOINT 
};
