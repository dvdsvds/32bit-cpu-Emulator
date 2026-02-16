#include "cpu/memory.hpp"
#include <cstring>
#include <iostream>
#include <iomanip>

Memory::Memory() : data(new u8[MEMORY_SIZE]) { reset(); } 

void Memory::reset() {
    std::memset(data.get(), 0, MEMORY_SIZE);
} 

u8 Memory::read_u8(addr_t addr) const {
    if(addr < MEMORY_SIZE) {
        return data[addr];
    } else {
        return 0;
    }
}

void Memory::write_u8(addr_t addr, u8 value) {
    if(addr < MEMORY_SIZE) {
        data[addr] = value;
    }
}

u16 Memory::read_u16(addr_t addr) const {
    if(addr <= MEMORY_SIZE - 2) {
        return data[addr] | (data[addr + 1] << 8);
    } else {
        return 0;
    }
}

void Memory::write_u16(addr_t addr, u16 value) {
    if(addr <= MEMORY_SIZE - 2) {
        data[addr] = (value & 0xFF);
        data[addr + 1] = (value >> 8) & 0xFF;
    }
}

u32 Memory::read_u32(addr_t addr) const {
    if(addr <= MEMORY_SIZE - 4) {
        return data[addr] | (data[addr + 1] << 8) | (data[addr + 2] << 16) | (data[addr + 3] << 24);
    } else {
        return 0;
    }
}

void Memory::write_u32_direct(addr_t addr, u32 value) {
    if(addr <= MEMORY_SIZE - 4) {
        data[addr] = (value & 0xFF);
        data[addr + 1] = (value >> 8) & 0xFF;
        data[addr + 2] = (value >> 16) & 0xFF;
        data[addr + 3] = (value >> 24) & 0xFF;
    }
}

void Memory::write_u32(addr_t addr, u32 value) {
    if(addr == 0x100) {
        std::cout << "[PRINT:INT] " << static_cast<int32_t>(value) << std::endl;
        return;
    }
    if(addr == 0x104) {
        std::cout << "[PRINT:BOOL] " << (value ? "true" : "false") << std::endl;
        return;
    }
    if(addr == 0x108) {
        std::string str;
        addr_t str_addr = value;
        char c;
        while((c = data[str_addr++]) != '\0') {
            str += c;
        }
        std::cout << "[PRINT:STR] " << str << std::endl;
        return;
    }

    write_u32_direct(addr, value);
}
