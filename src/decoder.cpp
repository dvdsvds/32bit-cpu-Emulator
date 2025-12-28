#include "decoder/decoder.hpp"
#include <iostream>
#include <cstdint>

namespace decoder {
    Mode extract_mode(inst_t inst) {
        uint8_t mode = extract_bits(inst, MODE_LSB, MODE_SIZE);
        return static_cast<Mode>(mode);
    }
    Opcode extract_opcode(inst_t inst) {
        uint8_t opcode = extract_bits(inst, OPCODE_LSB, OPCODE_SIZE);
        if (opcode <= 32) {
            return static_cast<Opcode>(opcode);
        }
        return Opcode::INVALID;
    }
    Format classify_format(Opcode op, Mode mode) {
        switch(op) {
            case Opcode::ADD:
            case Opcode::SUB:
            case Opcode::AND:
            case Opcode::OR:
            case Opcode::XOR:
            case Opcode::SHL:
            case Opcode::SHR:
            case Opcode::SAR:
            case Opcode::MUL:
            case Opcode::DIV:
            case Opcode::CMP:
            case Opcode::MOV:
                return (mode == Mode::REGISTER) ? Format::R : Format::I;
            
            case Opcode::LUI:
            case Opcode::AUI:
            case Opcode::LOADI:
                return Format::I;
            
            case Opcode::LOADW:
            case Opcode::LOADB:
            case Opcode::STOREW:
            case Opcode::STOREB:
            case Opcode::PUSH:
            case Opcode::POP:
                return Format::M;
            
            case Opcode::JMP:
            case Opcode::BJMP:
            case Opcode::CALL:
            case Opcode::RET:
            case Opcode::HLT:
            case Opcode::NOP:
            case Opcode::SYSCALL:
            case Opcode::TRAP:
            case Opcode::IRET:
                return Format::S;
            
            case Opcode::CSRR:
            case Opcode::CSRW:
            case Opcode::CSRWI:
                return Format::C;
            
            default:
                return Format::INVALID;
        }
    }
    Cond extract_cond(inst_t inst) {
        return static_cast<Cond>(extract_bits(inst, COND_LSB, COND_SIZE));
    }
    reg_index extract_rd(inst_t inst) {
        return extract_bits(inst, RD_LSB, RD_SIZE);
    }
    reg_index extract_rs1(inst_t inst) {
        return extract_bits(inst, RS1_LSB, RS1_SIZE);
    }
    reg_index extract_rs2(inst_t inst) {
        return extract_bits(inst, RS2_LSB, RS2_SIZE);
    }
    reg_index extract_csr(inst_t inst) {
        return extract_bits(inst, CSR_ADDR_LSB, CSR_ADDR_SIZE);
    }
    int extract_imm19(inst_t inst) {
        uint32_t imm = extract_bits(inst, IMM19_LSB, IMM19_SIZE);
        return sign_extend(imm, IMM19_SIZE);
    }
    int extract_imm21(inst_t inst) {
        uint32_t imm = extract_bits(inst, IMM21_LSB, IMM21_SIZE);
        return sign_extend(imm, IMM21_SIZE);
    }
    int extract_imm16(inst_t inst) {
        uint32_t imm = extract_bits(inst, IMM16_LSB, IMM16_SIZE);
        return sign_extend(imm, IMM16_SIZE);
    }
    int extract_imm26(inst_t inst) {
        uint32_t imm = extract_bits(inst, IMM26_LSB, IMM26_SIZE);
        return sign_extend(imm, IMM26_SIZE);
    }
    int extract_imm11(inst_t inst) {
        uint32_t imm = extract_bits(inst, IMM11_LSB, IMM11_SIZE);
        return sign_extend(imm, IMM11_SIZE);
    }
    int extract_addr23(inst_t inst) {
        uint32_t addr = extract_bits(inst, ADDR23_LSB, ADDR23_SIZE);
        return sign_extend(addr, ADDR23_SIZE);
    }
    int extract_addr26(inst_t inst) {
        uint32_t addr = extract_bits(inst, ADDR26_LSB, ADDR26_SIZE);
        return sign_extend(addr, ADDR26_SIZE);
    }
    decode decode_inst(inst_t inst) {
        decode d{};
        d.mode = extract_mode(inst);
        d.opcode = extract_opcode(inst);
        d.format = classify_format(d.opcode, d.mode);

        switch(d.format) {
            case Format::R:
                d.rd = extract_rd(inst);
                d.rs1 = extract_rs1(inst);
                if (d.opcode != Opcode::MOV) {
                    d.rs2 = extract_rs2(inst);
                }
                if (d.opcode == Opcode::CMP) {
                    d.rd = 0;
                }
                break;
            case Format::I:
                d.rd = extract_rd(inst);
                if (d.opcode == Opcode::SHL || d.opcode == Opcode::SHR || d.opcode == Opcode::SAR ||
                    d.opcode == Opcode::MUL || d.opcode == Opcode::DIV ||
                    d.opcode == Opcode::SUB || d.opcode == Opcode::ADD) {
                    d.rs1 = extract_rs1(inst); 
                }
                if (d.opcode == Opcode::LUI || d.opcode == Opcode::AUI || d.opcode == Opcode::LOADI) {
                    d.imm21 = extract_imm21(inst);
                } else {
                    d.imm19 = extract_imm19(inst);
                }
                if (d.opcode == Opcode::CMP) {
                    d.rd = 0;
                    d.rs1 = extract_rd(inst);
                }
                break;

            case Format::M:
                d.rd = extract_rd(inst);
                d.rs1 = extract_rs1(inst);

                if(d.opcode == Opcode::PUSH) {
                    d.rs1 = d.rd;
                    d.rd = 0;
                } else if(d.opcode == Opcode::POP) {
                    d.rs1 = 0;
                } else {
                    d.imm16 = extract_imm16(inst);
                }
                break;

            case Format::S:
                if (d.opcode == Opcode::JMP || d.opcode == Opcode::CALL) {
                    d.addr26 = extract_addr26(inst);
                } else if (d.opcode == Opcode::BJMP) {
                    d.cond = extract_cond(inst);
                    d.addr23 = extract_addr23(inst);
                } else if (d.opcode == Opcode::TRAP) {
                    d.imm26 = extract_imm26(inst);
                }
                break;

            case Format::C:
                if (d.opcode == Opcode::CSRR) {
                    d.rd = extract_rd(inst);
                    d.csr = extract_csr(inst);
                } else if (d.opcode == Opcode::CSRW) {
                    d.rs1 = extract_rd(inst);
                    d.csr = extract_csr(inst);
                } else if (d.opcode == Opcode::CSRWI) {
                    d.csr = extract_bits(inst, CSRWI_ADDR_LSB, CSR_ADDR_SIZE);
                    d.imm11 = extract_imm11(inst);
                }
                break;

            default:
                break;
        }
        
        return d;
    }
}