#pragma once
#include <cstdint>

static constexpr size_t MODE_LSB = 0;
static constexpr size_t OPCODE_LSB = 26;
static constexpr size_t RD_LSB = 21;
static constexpr size_t RS1_LSB = 16;
static constexpr size_t RS2_LSB = 11;

static constexpr size_t IMM19_LSB = 2;
static constexpr size_t IMM21_LSB = 0;
static constexpr size_t IMM16_LSB = 0;
static constexpr size_t IMM26_LSB = 2;
static constexpr size_t IMM11_LSB = 8;

static constexpr size_t ADDR23_LSB = 0;
static constexpr size_t ADDR26_LSB = 0;

static constexpr size_t COND_LSB = 23;
static constexpr size_t CSR_ADDR_LSB = 16;
static constexpr size_t CSRWI_ADDR_LSB = 19;

static constexpr size_t MODE_SIZE = 2;
static constexpr size_t OPCODE_SIZE = 6;
static constexpr size_t RD_SIZE = 5;
static constexpr size_t RS1_SIZE = 5;
static constexpr size_t RS2_SIZE = 5;
static constexpr size_t IMM19_SIZE = 19;
static constexpr size_t IMM21_SIZE = 21;
static constexpr size_t IMM16_SIZE = 16;
static constexpr size_t IMM26_SIZE = 26;
static constexpr size_t IMM11_SIZE = 11;
static constexpr size_t ADDR23_SIZE = 23;
static constexpr size_t ADDR26_SIZE = 26;
static constexpr size_t COND_SIZE = 3;
static constexpr size_t CSR_ADDR_SIZE = 5;

inline uint32_t extract_bits(uint32_t instruction, size_t lsb, size_t size) {
    return (instruction >> lsb) & ((1u << size) - 1);
}

inline int32_t sign_extend(uint32_t value, size_t bits) {
    uint32_t sign_bit = 1u << (bits - 1);
    if (value & sign_bit) {
        return static_cast<int32_t>(value | (~((1u << bits) - 1)));
    }
    return static_cast<int32_t>(value);
}