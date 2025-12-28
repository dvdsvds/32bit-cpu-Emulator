#pragma once
#include "cpu/memory.hpp"
#include "decoder/decoder.hpp"
#include "cpu/pipeline.hpp"
#include "cpu/predictor.hpp"
#include "cpu/csr.hpp"
#include <set>
#include <iostream>

class CPU {
    private:
        Memory mem;
        u32 registers[REGISTER_COUNT];
        u32 csr[CSR_COUNT];
        Flags f{};
        addr_t pc;
        
        Flags update_flags_add(u32 result, u32 op1, u32 op2);
        Flags update_flags_sub(u32 result, u32 op1, u32 op2);
        Flags update_flags_logic(u32 result);
        Flags update_flags_shift(u32 result);
        
        int32_t sign_extend_11bit(uint16_t value);
        int32_t sign_extend_16bit(uint16_t value);
        int32_t sign_extend_19bit(uint32_t value);
        int32_t sign_extend_21bit(uint32_t value);
        int32_t sign_extend_23bit(uint32_t value);
        int32_t sign_extend_26bit(uint32_t value);
        
        std::set<addr_t> breakpoint;
        bool halted;
        bool paused;
        
        inline u32 read_reg(reg_index idx) const { return (idx == 0) ? 0 : registers[idx]; }
        inline void write_reg(reg_index idx, u32 value) { if (idx != 0) registers[idx] = value; }
        
        pipeline pipe;
        bool need_forwarding(reg_index num);
        
        void if_stage();
        void id_stage();
        void ex_stage();
        void wb_stage();
        
        predictor branch_pred;
        
        bool interrupt_jump_pending;
        addr_t interrupt_target;

        bool need_flush_ifid = false;
        bool need_flush_idex = false;
        
        struct MemWrite {
            bool valid;
            addr_t addr;
            u32 data;
        };

        MemWrite mem_history[8];
    public:
        CPU();
        void reset();
        void step();
        void run();
        void set_register(reg_index idx, u32 value);
        void set_flags(Flags f);
        void load_program(const inst_t* program, size_t count, addr_t start_addr = 0);
        void add_breakpoint(addr_t addr);
        void remove_breakpoint(addr_t addr);
        void clear_breakpoints();
        bool has_breakpoint(addr_t addr) const;
        void continue_execution();
        void handle_interrupt();
        void trigger_interrupt(u8 code);
        void update_timer();
        void set_irq(int irq_num, bool value);
        bool get_irq(int irq_num);
        
        u32 get_register(reg_index idx) const;
        Flags get_flags();

        template<size_t N>
        void load_program(const inst_t (&program)[N], addr_t start_addr = 0) {
            load_program(program, N, start_addr);
        }

        addr_t get_pc() const { return pc; }
        addr_t get_sp() const { return csr[static_cast<u8>(Csr::SP)]; }
        Memory& get_memory() { return mem; }
        const Memory& get_memory() const { return mem; } 
        u32 get_csr(size_t index) const { return csr[index]; }
        int get_register_count() const { return REGISTER_COUNT; }
        bool is_halted() const { return halted; }
        bool is_paused() const { return paused; }
        
};