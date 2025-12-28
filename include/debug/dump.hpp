#pragma once

#include "types.hpp"
#include "cpu/cpu.hpp"
#include "cpu/memory.hpp"

class CPU;
class Memory;

namespace dump {
    void dump_memory(const Memory& mem, addr_t start, size_t size);
    void dump_registers(const CPU& cpu);
    void dump_stack(const CPU& cpu, addr_t sp, size_t size);
}