#pragma once
#include <cstdint>

enum class Mode : uint8_t {
    REGISTER = 0,
    IMMEDIATE = 1,
    FLAG = 2,
    RESERVED = 3,
    INVALID = 0xFF
};