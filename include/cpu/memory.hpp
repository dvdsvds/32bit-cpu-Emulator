#pragma once

#include "types.hpp"
#include <memory>

class Memory {
    private:
        std::unique_ptr<u8[]> data;
    public:
        Memory();
        void reset(); 

        u8 read_u8(addr_t addr) const;
        void write_u8(addr_t addr, u8 value);

        u16 read_u16(addr_t addr) const;
        void write_u16(addr_t addr, u16 value);

        u32 read_u32(addr_t addr) const;
        void write_u32(addr_t addr, u32 value);

        void write_u32_direct(addr_t addr, u32 value);
};