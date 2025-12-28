#pragma once
#include "types.hpp"
#include "isa/opcode.hpp"
#include "isa/cond.hpp"
#include "isa/mode.hpp"
#include "decoder/encoding.hpp"

namespace encoder {
    inst_t ADD(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t SUB(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t AND(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t OR(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t XOR(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t SHL(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t SHR(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t SAR(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t MUL(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    inst_t DIV(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode);
    
    inst_t CMP(reg_index rs1, int rs2_or_imm, Mode mode);
    
    inst_t MOV(reg_index rd, reg_index rs1);
    inst_t LUI(reg_index rd, int imm21);
    inst_t AUI(reg_index rd, int imm21);
    inst_t LOADI(reg_index rd, int imm21);
    
    inst_t LOADW(reg_index rd, reg_index rs1, int imm16);
    inst_t LOADB(reg_index rd, reg_index rs1, int imm16);
    inst_t STOREW(reg_index rd, reg_index rs1, int imm16);
    inst_t STOREB(reg_index rd, reg_index rs1, int imm16);
    
    inst_t JMP(int addr26);
    inst_t BJMP(Cond cond, int addr23);
    inst_t CALL(int addr26);
    inst_t RET();
    
    inst_t HLT();
    inst_t NOP();
    inst_t SYSCALL();
    inst_t TRAP(int imm26);
    inst_t IRET();
    
    inst_t CSRR(reg_index rd, reg_index csr, Mode mode);
    inst_t CSRW(reg_index rs1, reg_index csr, Mode mode);
    inst_t CSRWI(reg_index csr, int imm11);

    inst_t PUSH(reg_index rs);
    inst_t POP(reg_index rd);
}