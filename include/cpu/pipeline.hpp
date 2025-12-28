#pragma once
#include "types.hpp"
#include "isa/opcode.hpp"
#include "isa/format.hpp"
#include "isa/mode.hpp"
#include "isa/cond.hpp"

struct IF_ID {
    inst_t inst;
    addr_t curr_pc;
    addr_t predicted_pc;
    bool is_valid;
};
struct ID_EX {
    Opcode opcode;
    Format format;
    Mode mode;
    Cond cond;
    
    u32 v_rs1;
    u32 v_rs2;
    u32 curr_v_rd;
    
    reg_index n_rs1;
    reg_index n_rs2;
    reg_index n_rd;
    reg_index n_csr;
    
    int imm19;
    int imm21;
    int imm16;
    int imm26;
    int imm11;
    int addr23;
    int addr26;
    
    bool reg_write;
    bool mem_read;
    bool mem_write;
    bool is_branch;
    
    addr_t curr_pc;
    addr_t predicted_pc;
    bool is_valid;
};
struct EX_WB {
    Opcode opcode;
    u32 alu_result;
    u32 mem_read_data;
    addr_t mem_addr;
    u32 w_data;
    reg_index n_rd;
    
    bool reg_write;
    bool mem_read;
    bool mem_write;
    bool mem_u8;
    bool is_branch;
    bool branch_taken;
    
    Flags f;
    bool is_valid;
};
struct PipelineRegs {
    IF_ID ifid;
    ID_EX idex;
    EX_WB exwb;
};
class pipeline {
    private:
        PipelineRegs regs[2];
        int idx = 0;
        
    public:
        pipeline();
        void reset();

        IF_ID& ifid() { return regs[idx].ifid; }
        ID_EX& idex() { return regs[idx].idex; }
        EX_WB& exwb() { return regs[idx].exwb; }

        IF_ID& next_ifid() { return regs[1 - idx].ifid; }
        ID_EX& next_idex() { return regs[1 - idx].idex; }
        EX_WB& next_exwb() { return regs[1 - idx].exwb; }
        
        void clock() { idx = 1 - idx; }
        
        void flush_ifid() { next_ifid().is_valid = false; }
        void flush_idex() { next_idex().is_valid = false; }
        void flush_exwb() { next_exwb().is_valid = false; }
        
        IF_ID& getIFID() { return ifid(); }
        ID_EX& getIDEX() { return idex(); }
        EX_WB& getEXWB() { return exwb(); }
        
        IF_ID& get_next_IFID() { return next_ifid(); }
        ID_EX& get_next_IDEX() { return next_idex(); }
        EX_WB& get_next_EXWB() { return next_exwb(); }

        void update_IFID() {}
        void update_IDEX() {}
        void update_EXWB() {}

        void flush_IFID() { flush_ifid(); }
        void flush_IDEX() { flush_idex(); }
        void flush_EXWB() { flush_exwb(); }
};