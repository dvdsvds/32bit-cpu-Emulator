#pragma once
#include "isa/opcode.hpp"
#include "isa/cond.hpp"
#include "isa/mode.hpp"
#include "isa/format.hpp"
#include "cpu/csr.hpp"
#include "decoder/encoding.hpp"

namespace decoder {
    struct decode {
        Opcode opcode;
        Mode mode;
        Format format;
        
        reg_index rd;
        reg_index rs1;
        reg_index rs2;
        reg_index csr;
        
        int imm19;
        int imm21;
        int imm16;
        int imm26;
        int imm11;
        
        int addr23;
        int addr26;
        
        Cond cond;
    };
    
    decode decode_inst(inst_t inst);
    Format classify_format(Opcode op, Mode mode);
    Mode extract_mode(inst_t inst);
    Opcode extract_opcode(inst_t inst);
    
    Cond extract_cond(inst_t inst);
    reg_index extract_rd(inst_t inst);
    reg_index extract_rs1(inst_t inst);
    reg_index extract_rs2(inst_t inst);
    reg_index extract_csr(inst_t inst);
    
    int extract_imm19(inst_t inst);
    int extract_imm21(inst_t inst);
    int extract_imm16(inst_t inst);
    int extract_imm26(inst_t inst);
    int extract_imm11(inst_t inst);
    int extract_addr23(inst_t inst);
    int extract_addr26(inst_t inst);
}