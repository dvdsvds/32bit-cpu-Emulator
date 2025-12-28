#pragma once
#include <cstdint>

// define basic types
using u8 = uint8_t;
using u16 = uint16_t; 
using u32 = uint32_t;
using reg_index = uint8_t;
using addr_t = uint32_t;
using flags_t = uint8_t;
using inst_t = uint32_t;

// define pulic constants
static constexpr uint8_t REGISTER_COUNT = 32;
static constexpr uint32_t MEMORY_SIZE = 16 * 1024 * 1024;
static constexpr uint8_t DATA_WIDTH = 32;
static constexpr uint8_t SHORT_INST_SIZE = 2;
static constexpr uint8_t LONG_INST_SIZE = 4;
static constexpr uint32_t STACK_START = MEMORY_SIZE - 4;

// define Flags
static constexpr int ZF_BIT = 0;
static constexpr int ZF_MASK = (1 << ZF_BIT);

static constexpr int CF_BIT = 1;
static constexpr int CF_MASK = (1 << CF_BIT);

static constexpr int SF_BIT = 2;
static constexpr int SF_MASK = (1 << SF_BIT);

static constexpr int OF_BIT = 3;
static constexpr int OF_MASK = (1 << OF_BIT);

struct Flags {
    bool zf = false;   
    bool cf = false;   
    bool sf = false;   
    bool of = false;   
};