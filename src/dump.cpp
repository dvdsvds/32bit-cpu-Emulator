#include "debug/dump.hpp"
#include <iostream>
#include <iomanip>

namespace dump {
    void dump_memory(const Memory& mem, addr_t start, size_t size) {
        std::cout << "----------------------------------------------------------" << std::endl;
        for(addr_t curr_addr = start; curr_addr < start + size; curr_addr += 16) {
            std::cout << "| " <<std::hex << std::setw(4) << std::setfill('0') << curr_addr << " | ";
            for(int i = 0; i < 16 && curr_addr + i < start + size; i++) {
                u8 b = mem.read_u8(curr_addr + i);
                std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)b << " ";
            }
            std::cout << "|";
            std::cout << std::endl;
            std::cout << "----------------------------------------------------------";
            std::cout << std::endl;
        }
    }

    void dump_registers(const CPU& cpu) {
        for(int i = 0; i < REGISTER_COUNT; i++) {
            u32 val = cpu.get_register(i);
            if(val == 0) continue;

            if(i < 10) {
                std::cout << "R0" << std::dec << i << " = 0x" << std::hex << std::setw(8) << std::setfill('0') << val << " > " << std::dec << val << std::endl;
            } else {
                std::cout << "R" << std::dec << i << " = 0x" << std::hex << std::setw(8) << std::setfill('0') << val << " > " << std::dec << val << std::endl;
            }
        }
    }

    void dump_stack(const CPU& cpu, addr_t sp, size_t size) {
        const Memory& mem = cpu.get_memory();
        for(int i = 0; i < size; i++) {
            addr_t addr = sp + (i * 2); 
            u32 data = mem.read_u32(addr);
            std::cout << "[0x" << std::hex << std::setw(8) << std::setfill('0') << addr << "] = 0x" << std::hex << std::setw(8) << std::setfill('0') << data;
            if(addr == sp) {
                std::cout << "  <- SP";
            }
            std::cout << std::endl;
        }
    }
};