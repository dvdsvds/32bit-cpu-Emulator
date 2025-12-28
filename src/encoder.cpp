#include "encoder/encoder.hpp"

using o = Opcode;
namespace encoder {
    inst_t ADD(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::ADD) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t SUB(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::SUB) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t AND(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::AND) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t OR(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::OR) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t XOR(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::XOR) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t SHL(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::SHL) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t SHR(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::SHR) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t SAR(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::SAR) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t MUL(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::MUL) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t DIV(reg_index rd, reg_index rs1_or_unused, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::DIV) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs1_or_unused & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t CMP(reg_index rs1, int rs2_or_imm, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::CMP) << OPCODE_LSB);
        
        if (mode == Mode::REGISTER) {
            inst |= ((rs1 & 0x1F) << RS1_LSB);
            inst |= ((rs2_or_imm & 0x1F) << RS2_LSB);
        } else {
            inst |= ((rs1 & 0x1F) << RD_LSB);
            inst |= ((rs2_or_imm & 0x7FFFF) << IMM19_LSB);
        }
        return inst;
    }
    inst_t MOV(reg_index rd, reg_index rs1) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::MOV) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((rs1 & 0x1F) << RS1_LSB);
        return inst;
    }
    inst_t LUI(reg_index rd, int imm21) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::LUI) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((imm21 & 0x1FFFFF) << IMM21_LSB);
        return inst;
    }
    inst_t AUI(reg_index rd, int imm21) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::AUI) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((imm21 & 0x1FFFFF) << IMM21_LSB);
        return inst;
    }
    inst_t LOADI(reg_index rd, int imm21) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::LOADI) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((imm21 & 0x1FFFFF) << 0);
        return inst;
    }
    inst_t LOADW(reg_index rd, reg_index rs1, int imm16) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::LOADW) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((rs1 & 0x1F) << RS1_LSB);
        inst |= ((imm16 & 0xFFFF) << IMM16_LSB);
        return inst;
    }
    inst_t LOADB(reg_index rd, reg_index rs1, int imm16) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::LOADB) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((rs1 & 0x1F) << RS1_LSB);
        inst |= ((imm16 & 0xFFFF) << IMM16_LSB);
        return inst;
    }
    inst_t STOREW(reg_index rd, reg_index rs1, int imm16) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::STOREW) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((rs1 & 0x1F) << RS1_LSB);
        inst |= ((imm16 & 0xFFFF) << IMM16_LSB);
        return inst;
    }
    inst_t STOREB(reg_index rd, reg_index rs1, int imm16) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::STOREB) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((rs1 & 0x1F) << RS1_LSB);
        inst |= ((imm16 & 0xFFFF) << IMM16_LSB);
        return inst;
    }
    inst_t JMP(int addr26) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::JMP) << OPCODE_LSB);
        inst |= ((addr26 & 0x3FFFFFF) << ADDR26_LSB);
        return inst;
    }
    inst_t BJMP(Cond cond, int addr23) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::BJMP) << OPCODE_LSB);
        inst |= ((static_cast<uint8_t>(cond) & 0x07) << COND_LSB);
        inst |= ((addr23 & 0x7FFFFF) << ADDR23_LSB);
        return inst;
    }
    inst_t CALL(int addr26) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::CALL) << OPCODE_LSB);
        inst |= ((addr26 & 0x3FFFFFF) << ADDR26_LSB);
        return inst;
    }
    inst_t RET() {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::RET) << OPCODE_LSB);
        return inst;
    }
    inst_t HLT() {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::HLT) << OPCODE_LSB);
        return inst;
    }
    inst_t NOP() {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::NOP) << OPCODE_LSB);
        return inst;
    }
    inst_t SYSCALL() {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::SYSCALL) << OPCODE_LSB);
        return inst;
    }
    inst_t TRAP(int imm26) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::TRAP) << OPCODE_LSB);
        inst |= ((imm26 & 0x3FFFFFF) << IMM26_LSB);
        return inst;
    }
    inst_t IRET() {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::IRET) << OPCODE_LSB);
        return inst;
    }
    inst_t CSRR(reg_index rd, reg_index csr, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::CSRR) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        inst |= ((csr & 0x1F) << CSR_ADDR_LSB);
        return inst;
    }
    inst_t CSRW(reg_index rs1, reg_index csr, Mode mode) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(mode) << MODE_LSB);
        inst |= (static_cast<uint32_t>(o::CSRW) << OPCODE_LSB);
        inst |= ((rs1 & 0x1F) << RD_LSB);
        inst |= ((csr & 0x1F) << CSR_ADDR_LSB);
        return inst;
    }
    inst_t CSRWI(reg_index csr, int imm11) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::CSRWI) << OPCODE_LSB);
        inst |= ((csr & 0x1F) << CSRWI_ADDR_LSB);
        inst |= ((imm11 & 0x7FF) << IMM11_LSB);
        return inst;
    }
    inst_t PUSH(reg_index rs) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::PUSH) << OPCODE_LSB);
        inst |= ((rs & 0x1F) << RD_LSB);
        return inst;
    }
    inst_t POP(reg_index rd) {
        inst_t inst = 0;
        inst |= (static_cast<uint32_t>(o::POP) << OPCODE_LSB);
        inst |= ((rd & 0x1F) << RD_LSB);
        return inst;
    }
}