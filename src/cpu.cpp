#ifdef _WIN32
    #include <conio.h>
#endif
#include "cpu/cpu.hpp"
#include <iomanip>
#include <cstring>

CPU::CPU() { 
    reset(); 
}
bool CPU::need_forwarding(reg_index num) {
    EX_WB& exwb = pipe.getEXWB();
    return ((exwb.is_valid == true) && 
            (exwb.reg_write == true || exwb.mem_read == true) &&
            (num != 0) && 
            (exwb.n_rd == num));
}
void CPU::reset() {
    std::memset(registers, 0, sizeof(registers));
    std::memset(csr, 0, sizeof(csr));
    f = Flags{};
    pc = 0;
    mem.reset();
    halted = false;
    csr[static_cast<u8>(Csr::SP)] = STACK_START;
    interrupt_jump_pending = false;
    interrupt_target = 0;
    for(int i = 0; i < 4; i++) {
        mem_history[i].valid = false;
        mem_history[i].addr = 0;
        mem_history[i].data = 0;
    }
}
void CPU::wb_stage() { 
    EX_WB& exwb = pipe.getEXWB();

    if(exwb.is_valid) {
        if(exwb.reg_write) {
            write_reg(exwb.n_rd, exwb.alu_result);
        }

        if(exwb.mem_read) {
            u32 read_data;
            bool forwarded = false;

            for(int i = 0; i < 8; i++) {
                if(mem_history[i].valid && mem_history[i].addr == exwb.mem_addr) {
                    read_data = mem_history[i].data;
                    forwarded = true;
                    break;
                }
            }

            if(!forwarded) {
                if(exwb.mem_u8) {
                    read_data = mem.read_u8(exwb.mem_addr);
                } else {
                    read_data = mem.read_u32(exwb.mem_addr);
                }
            }
            exwb.mem_read_data = read_data;
            write_reg(exwb.n_rd, read_data);
        }

        if(exwb.mem_write) {
            // std::cerr << "[MEM_WRITE] addr=" << std::hex << exwb.mem_addr << " val=" << exwb.w_data << std::endl;
            if(exwb.mem_u8) {
                mem.write_u8(exwb.mem_addr, exwb.w_data);
            } else {
                mem.write_u32(exwb.mem_addr, exwb.w_data);
            }

            bool found = false;
            for(int i = 0; i < 8; i++) {
                if(mem_history[i].valid && mem_history[i].addr == exwb.mem_addr) {
                    mem_history[i].data = exwb.w_data;
                    found = true;
                    break;
                }
            }
            
            if(!found) {
                for(int i = 7; i > 0; i--) {
                    mem_history[i] = mem_history[i-1];
                }
                mem_history[0].valid = true;
                mem_history[0].addr = exwb.mem_addr;
                mem_history[0].data = exwb.w_data;
            }
        }
        if(exwb.opcode == Opcode::HLT) {
            u32 status = csr[static_cast<u8>(Csr::STATUS)];
            u32 mode = (status >> CSR_MODE_LSB) & 1;
            
            if(mode == 0) {  
                halted = true;
            } 
        }

        if(exwb.flags_write) {
            f = exwb.f;
        }
    }
}
void CPU::ex_stage() {
    ID_EX& idex = pipe.getIDEX();
    EX_WB& next = pipe.get_next_EXWB();
    EX_WB& exwb = pipe.getEXWB();

    if(!idex.is_valid) {
        next.is_valid = false;
        return;
    } 

    next.branch_taken = false;
    next.flags_write = false;
    u32 result;
    next.opcode = idex.opcode;
    next.reg_write = idex.reg_write;
    next.mem_read = idex.mem_read;
    next.mem_write = idex.mem_write;
    next.is_branch = idex.is_branch;

    auto get_forwarded_value = [&](reg_index reg, u32 original_val) -> u32 {
        if(need_forwarding(reg)) {
            return exwb.mem_read ? exwb.mem_read_data : exwb.alu_result;
        }
        return original_val;
    };

    switch(idex.opcode) {
        case Opcode::ADD: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = rs1_val + rs2_val;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_add(result, rs1_val, rs2_val);
            } else {
                u32 rd_val = get_forwarded_value(idex.n_rd, idex.curr_v_rd);
                result = rd_val + idex.imm19;

                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_add(result, rd_val, idex.imm19);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::SUB: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = rs1_val - rs2_val;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_sub(result, rs1_val, rs2_val);
            } else {
                u32 rd_val = get_forwarded_value(idex.n_rd, idex.curr_v_rd);
                result = rd_val - idex.imm19;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_sub(result, rd_val, idex.imm19);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::AND: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = rs1_val & rs2_val;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            } else {
                u32 rd_val = get_forwarded_value(idex.n_rd, idex.curr_v_rd);
                result = rd_val & idex.imm19;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::OR: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = rs1_val | rs2_val;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            } else {
                u32 rd_val = get_forwarded_value(idex.n_rd, idex.curr_v_rd);
                result = rd_val | idex.imm19;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::XOR: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = rs1_val ^ rs2_val;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            } else {
                u32 rd_val = get_forwarded_value(idex.n_rd, idex.curr_v_rd);
                result = rd_val ^ idex.imm19;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::SHL: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = rs1_val << (rs2_val & 0x1F);
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_shift(result);
            } else {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                result = rs1_val << (idex.imm19 & 0x1F);
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_shift(result);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::SHR: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = rs1_val >> (rs2_val & 0x1F);
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_shift(result);
            } else {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                result = rs1_val >> (idex.imm19 & 0x1F);
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_shift(result);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::SAR: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = (int32_t)rs1_val >> (rs2_val & 0x1F);
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_shift(result);
            } else {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                result = (int32_t)rs1_val >> (idex.imm19 & 0x1F);
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_shift(result);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::MUL: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = (int32_t)rs1_val * (int32_t)rs2_val;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            } else {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                result = (int32_t)rs1_val * idex.imm19;
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::DIV: {
            if(idex.format == Format::R) {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                if(rs2_val != 0) {
                    result = (int32_t)rs1_val / (int32_t)rs2_val;
                } else {
                    result = 0;
                }
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            } else {
                u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
                if(idex.imm19 != 0) {
                    result = (int32_t)rs1_val / idex.imm19;
                } else {
                    result = 0;
                }
                next.alu_result = result;
                next.n_rd = idex.n_rd;
                next.f = update_flags_logic(result);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::CMP: {
            u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            if(idex.format == Format::R) {
                u32 rs2_val = get_forwarded_value(idex.n_rs2, idex.v_rs2);
                result = rs1_val - rs2_val;
                next.f = update_flags_sub(result, rs1_val, rs2_val);
            } else {
                result = rs1_val - idex.imm19;
                next.f = update_flags_sub(result, rs1_val, idex.imm19);
            }
            next.is_valid = true;
            next.flags_write = true;
            break;
        }
        case Opcode::MOV: {
            u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            next.alu_result = rs1_val;
            next.n_rd = idex.n_rd;
            next.is_valid = true;
            break;
        }
        case Opcode::LUI: {
            next.alu_result = (idex.imm21 << 11);
            next.n_rd = idex.n_rd;
            next.is_valid = true;
            break;
        }
        case Opcode::AUI: {
            u32 rd_val = get_forwarded_value(idex.n_rd, idex.curr_v_rd);
            next.alu_result = rd_val + (idex.imm21 << 11);
            next.n_rd = idex.n_rd;
            next.is_valid = true;
            break;
        }
        case Opcode::LOADI: {
            next.alu_result = idex.imm21;
            next.n_rd = idex.n_rd;
            next.is_valid = true;
            break;
        }
        case Opcode::LOADW: {
            u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            next.mem_addr = rs1_val + idex.imm16;
            next.n_rd = idex.n_rd;
            next.mem_u8 = false;
            next.mem_read = true;
            next.is_valid = true;
            break;
        }
        case Opcode::LOADB: {
            u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            next.mem_addr = rs1_val + idex.imm16;
            next.n_rd = idex.n_rd;
            next.mem_u8 = true;
            next.mem_read = true;
            next.is_valid = true;
            break;
        }
        case Opcode::STOREW: {
            u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            u32 rd_val = get_forwarded_value(idex.n_rd, idex.curr_v_rd);
            next.mem_addr = rs1_val + idex.imm16;
            next.w_data = rd_val;
            next.mem_u8 = false;
            next.mem_write = true;
            next.is_valid = true;
            break;
        }
        case Opcode::STOREB: {
            u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            u32 rd_val = get_forwarded_value(idex.n_rd, idex.curr_v_rd);
            next.mem_addr = rs1_val + idex.imm16;
            next.w_data = rd_val & 0xFF;
            next.mem_u8 = true;
            next.mem_write = true;
            next.is_valid = true;
            break;
        }
        case Opcode::JMP: {
            int32_t offset = sign_extend_26bit(idex.addr26);
            addr_t target = idex.curr_pc + offset;

            pc = target;
            need_flush_ifid = true;
            need_flush_idex = true;
            next.is_valid = false;
            break;
        }
        case Opcode::BJMP: {
            Flags current_flags = f;
            if(exwb.is_valid && exwb.flags_write) {
                current_flags = exwb.f;
            }
            
            bool take = false;
            switch(idex.cond) {
                case Cond::EQ: take = current_flags.zf; break;
                case Cond::NE: take = !current_flags.zf; break;
                case Cond::LT: take = (current_flags.sf != current_flags.of); break;
                case Cond::LE: take = current_flags.zf || (current_flags.sf != current_flags.of); break;
                case Cond::GT: take = !current_flags.zf && (current_flags.sf == current_flags.of); break;
                case Cond::GE: take = (current_flags.sf == current_flags.of); break;
                case Cond::CS: take = current_flags.cf; break;
                case Cond::CC: take = !current_flags.cf; break;
                default: take = false; break;
            }
            
            next.branch_taken = take;
            addr_t fallthrough = idex.curr_pc + 4;
            
            int32_t offset = sign_extend_23bit(idex.addr23);
            addr_t target = idex.curr_pc + offset;
            
            if(take) {
                if(idex.predicted_pc != target) {
                    pc = target;
                    need_flush_ifid = true;  
                    need_flush_idex = true;  
                }
            } else {
                if(idex.predicted_pc != fallthrough) {
                    pc = fallthrough;
                    need_flush_ifid = true;  
                    need_flush_idex = true;  
                }
            }
            next.is_valid = false;
            break;
        }
        case Opcode::CALL: {
            addr_t return_addr = idex.curr_pc + 4;

            u32 old_sp = csr[static_cast<u8>(Csr::SP)];
            u32 new_sp = old_sp - 4;

            mem.write_u32(new_sp, return_addr);
            csr[static_cast<u8>(Csr::SP)] = new_sp;

            int32_t offset = sign_extend_26bit(idex.addr26);
            addr_t target = idex.curr_pc + offset;
            pc = target;

            need_flush_ifid = true;
            need_flush_idex = true;
            next.is_valid = false;
            break;
        }
        case Opcode::CALLR: {
            addr_t return_addr = idex.curr_pc + 4;
            u32 old_sp = csr[static_cast<u8>(Csr::SP)];
            u32 new_sp = old_sp - 4;
            mem.write_u32(new_sp, return_addr);
            csr[static_cast<u8>(Csr::SP)] = new_sp;
            addr_t target = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            pc = target;

            need_flush_ifid = true;
            need_flush_idex = true;
            next.is_valid = false;
            break;
        }
        case Opcode::RET: {
            u32 old_sp = csr[static_cast<u8>(Csr::SP)];
            addr_t return_addr = mem.read_u32(old_sp);
            u32 new_sp = old_sp + 4;
            csr[static_cast<u8>(Csr::SP)] = new_sp;

            pc = return_addr;
            need_flush_ifid = true;
            need_flush_idex = true;
            next.is_valid = true;
            break;
        }
        case Opcode::HLT: {
            next.is_valid = true;
            break;
        }
        case Opcode::NOP: {
            next.is_valid = true;
            break;
        }
        case Opcode::SYSCALL: {
            csr[static_cast<u8>(Csr::EPC)] = idex.curr_pc + 4;

            u8 code = 5;
            csr[static_cast<u8>(Csr::CAUSE)] = ((1u << INTERRUPT_LSB) | code);

            u32 flags_value = (f.zf << 0) | (f.sf << 1) | (f.cf << 2) | (f.of << 3);
            csr[static_cast<u8>(Csr::EFLAGS)] = flags_value;

            u32 status = csr[static_cast<u8>(Csr::STATUS)];
            u32 ie = (status >> IE_LSB) & 1;
            status &= ~(1u << IE_LSB);
            status |= (1u << CSR_MODE_LSB);
            status &= ~(1u << PIE_LSB);
            status |= (ie << PIE_LSB);
            csr[static_cast<u8>(Csr::STATUS)] = status;

            u32 ivtbr = csr[static_cast<u8>(Csr::IVTBR)];
            u32 offset = code * 4;
            u32 handler_addr = mem.read_u32(ivtbr + offset);

            addr_t sp = csr[static_cast<u8>(Csr::SP)];
            for(int i = 31; i >= 1; i--) {
                sp -= 4;
                mem.write_u32_direct(sp, registers[i]);
            }
            csr[static_cast<u8>(Csr::SCRATCH)] = sp;
            csr[static_cast<u8>(Csr::SP)] = sp;

            pc = handler_addr;
            
            next.is_valid = false;
            need_flush_ifid = true;
            need_flush_idex = true;
            break;
        }
        case Opcode::TRAP: {
            csr[static_cast<u8>(Csr::EPC)] = idex.curr_pc + 4;
            
            u8 code = idex.imm26 & 0xFF;
            csr[static_cast<u8>(Csr::CAUSE)] = ((1u << INTERRUPT_LSB) | code);
            
            u32 flags_value = (f.zf << 0) | (f.sf << 1) | (f.cf << 2) | (f.of << 3);
            csr[static_cast<u8>(Csr::EFLAGS)] = flags_value;
            
            u32 status = csr[static_cast<u8>(Csr::STATUS)];
            u32 ie = (status >> IE_LSB) & 1;
            status &= ~(1u << IE_LSB);
            status |= (1u << CSR_MODE_LSB);
            status &= ~(1u << PIE_LSB);
            status |= (ie << PIE_LSB);
            csr[static_cast<u8>(Csr::STATUS)] = status;
            
            u32 ivtbr = csr[static_cast<u8>(Csr::IVTBR)];
            u32 offset = code * 4;
            u32 handler_addr = mem.read_u32(ivtbr + offset);
            
            pc = handler_addr;
            
            next.is_valid = false;
            need_flush_ifid = true;
            need_flush_idex = true;
            break;
        }
        case Opcode::IRET: {
            addr_t sp = csr[static_cast<u8>(Csr::SCRATCH)];
            for(int i = 1; i <= 31; i++) {
                registers[i] = mem.read_u32_direct(sp);
                sp += 4;
            }
            csr[static_cast<u8>(Csr::SP)] = sp;

            u32 epc = csr[static_cast<u8>(Csr::EPC)];
            u32 eflags = csr[static_cast<u8>(Csr::EFLAGS)];
            
            f.zf = (eflags >> 0) & 1;
            f.sf = (eflags >> 1) & 1;
            f.cf = (eflags >> 2) & 1;
            f.of = (eflags >> 3) & 1;
            
            u32 status = csr[static_cast<u8>(Csr::STATUS)];
            u32 pie = (status >> PIE_LSB) & 1;
            
            status |= (pie << IE_LSB);
            status &= ~(1u << CSR_MODE_LSB);
            csr[static_cast<u8>(Csr::STATUS)] = status;
            
            pc = epc;
            need_flush_ifid = true;
            need_flush_idex = true;
            next.is_valid = false;
            break;
        }
        case Opcode::CSRR: {
            next.alu_result = csr[idex.n_csr];
            next.n_rd = idex.n_rd;
            next.is_valid = true;
            break;
        }
        case Opcode::CSRW: {
            u32 rs1_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            csr[idex.n_csr] = rs1_val;
            next.is_valid = true;
            break;
        }
        case Opcode::CSRWI: {
            csr[idex.n_csr] = idex.imm11;
            next.is_valid = true;
            break;
        }
        case Opcode::PUSH: {
            u32 rs_val = get_forwarded_value(idex.n_rs1, idex.v_rs1);
            u32 old_sp = csr[static_cast<u8>(Csr::SP)];
            u32 new_sp = old_sp - 4;

            mem.write_u32(new_sp, rs_val);
            csr[static_cast<u8>(Csr::SP)] = new_sp;

            next.is_valid = true;
            break;
        }
        case Opcode::POP: {
            u32 old_sp = csr[static_cast<u8>(Csr::SP)];
            u32 value = mem.read_u32(old_sp);
            u32 new_sp = old_sp + 4;
            
            next.alu_result = value;
            next.n_rd = idex.n_rd;
            next.reg_write = true;
            
            csr[static_cast<u8>(Csr::SP)] = new_sp;
            next.is_valid = true;
            break;
        }
        default:
            next.is_valid = false;
            break;
    }
}
void CPU::id_stage() {
    IF_ID& ifid = pipe.getIFID();
    ID_EX& next = pipe.get_next_IDEX();
    
    if(!ifid.is_valid) {
        next.is_valid = false;
        return;
    }
    
    decoder::decode d = decoder::decode_inst(ifid.inst);
    
    next.opcode = d.opcode;
    next.format = d.format;
    next.mode = d.mode;
    next.curr_pc = ifid.curr_pc;
    next.predicted_pc = ifid.predicted_pc;
    
    next.n_rd = d.rd;
    next.n_rs1 = d.rs1;
    next.n_rs2 = d.rs2;
    next.n_csr = d.csr;
    
    next.curr_v_rd = read_reg(d.rd);
    next.v_rs1 = read_reg(d.rs1);
    next.v_rs2 = read_reg(d.rs2);
    
    next.imm19 = d.imm19;
    next.imm21 = d.imm21;
    next.imm16 = d.imm16;
    next.imm26 = d.imm26;
    next.imm11 = d.imm11;
    next.addr23 = d.addr23;
    next.addr26 = d.addr26;
    next.cond = d.cond;
    
    next.reg_write = false;
    next.mem_read = false;
    next.mem_write = false;
    next.is_branch = false;
    
    switch(d.opcode) {
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
        case Opcode::MOV:
        case Opcode::LUI:
        case Opcode::AUI:
        case Opcode::LOADI:
            next.reg_write = true;
            break;
            
        case Opcode::LOADW:
        case Opcode::LOADB:
            next.mem_read = true;
            break;
            
        case Opcode::POP:
            next.reg_write = true;
            break;

        case Opcode::STOREW:
        case Opcode::STOREB:
        case Opcode::PUSH:
            next.mem_write = true;
            break;
            
        case Opcode::BJMP:
            next.is_branch = true;
            break;
            
        case Opcode::CSRR:
            next.reg_write = true;
            break;
            
        default:
            break;
    }
    
    next.is_valid = true;
}
void CPU::if_stage() {
    if(need_flush_ifid) {
        return;
    }

    IF_ID& next = pipe.get_next_IFID();
    
    next.curr_pc = pc;
    inst_t inst = mem.read_u32_direct(pc);
    next.inst = inst;
    
    // std::cerr << "[IF] pc=0x" << std::hex << pc << " inst=0x" << inst << std::endl;

    Mode mode = decoder::extract_mode(inst);
    Opcode opcode = decoder::extract_opcode(inst);

    if(opcode == Opcode::BJMP) {
        Cond cond = decoder::extract_cond(inst);
        int32_t offset = sign_extend_23bit(decoder::extract_addr23(inst));
        addr_t target = pc + offset;
        
        bool prediction = branch_pred.predict(pc);
        
        if(prediction) {
            next.predicted_pc = target;
            pc = target;
        } else {
            next.predicted_pc = pc + 4;
            pc += 4;
        }
    } else {
        next.predicted_pc = pc + 4;
        pc += 4;
    }
    
    next.is_valid = true;
}
void CPU::step() {
#ifdef _WIN32
    if(_kbhit()) {
        u8 c = static_cast<u8>(_getch());
        push_key(c);
    }
#endif
    update_timer();
    wb_stage();
    handle_interrupt();
    ex_stage();
    id_stage();
    if_stage();

    if(need_flush_ifid) {
        pipe.flush_IFID();
        need_flush_ifid = false;
    }
    if(need_flush_idex) {
        pipe.flush_IDEX();
        need_flush_idex = false;
    }
    
    pipe.clock();

    if(has_breakpoint(pc)) {
        paused = true;
    }
}
void CPU::run() {
    paused = false;
    while(!is_halted() && !is_paused()) {
        step();
    }
}
u32 CPU::get_register(reg_index idx) const {
    if(idx == 0) return 0;
    if(idx >= REGISTER_COUNT) {
        throw std::out_of_range("Register index out of range");
    }
    return registers[idx];
}
void CPU::set_register(reg_index idx, u32 value) {
    if(idx == 0) return;
    if(idx >= REGISTER_COUNT) {
        throw std::out_of_range("Register index out of range");
    }
    registers[idx] = value;
}
Flags CPU::get_flags() {
    return f;
}
void CPU::set_flags(Flags new_flags) {
    f = new_flags;
}
Flags CPU::update_flags_add(u32 result, u32 op1, u32 op2) {
    Flags f{};
    f.zf = (result == 0);
    f.sf = (result & 0x80000000) != 0;
    f.cf = (result < op1);
    bool op1_sign = (op1 & 0x80000000) != 0;
    bool op2_sign = (op2 & 0x80000000) != 0;
    bool result_sign = (result & 0x80000000) != 0;
    f.of = (op1_sign == op2_sign) && (op1_sign != result_sign);
    return f;
}
Flags CPU::update_flags_sub(u32 result, u32 op1, u32 op2) {
    Flags f{};
    f.zf = (result == 0);
    f.sf = (result & 0x80000000) != 0;
    f.cf = (op1 < op2);
    bool op1_sign = (op1 & 0x80000000) != 0;
    bool op2_sign = (op2 & 0x80000000) != 0;
    bool result_sign = (result & 0x80000000) != 0;
    f.of = (op1_sign != op2_sign) && (op1_sign != result_sign);
    return f;
}
Flags CPU::update_flags_logic(u32 result) {
    Flags f{};
    f.zf = (result == 0);
    f.sf = (result & 0x80000000) != 0;
    f.cf = false;
    f.of = false;
    return f;
}
Flags CPU::update_flags_shift(u32 result) {
    Flags f{};
    f.zf = (result == 0);
    f.sf = (result & 0x80000000) != 0;
    return f;
}
void CPU::load_program(const inst_t* program, size_t count, addr_t start_addr) {
    addr_t addr = start_addr;
    for(size_t i = 0; i < count; i++) {
        mem.write_u32_direct(addr, program[i]);
        addr += 4;
    }
}
int32_t CPU::sign_extend_11bit(uint16_t value) {
    if(value & 0x400) {
        return value | 0xFFFFF800;
    }
    return value;
}
int32_t CPU::sign_extend_16bit(uint16_t value) {
    if (value & 0x8000) {  
        return value | 0xFFFF0000;
    }
    return value;
}
int32_t CPU::sign_extend_19bit(uint32_t value) {
    if(value & 0x40000) {
        return value | 0xFFF80000;
    }
    return value;
}
int32_t CPU::sign_extend_21bit(uint32_t value) {
    if (value & 0x100000) {  
        return value | 0xFFE00000;
    }
    return value;
}
int32_t CPU::sign_extend_23bit(uint32_t value) {
    if(value & 0x400000) {
        return value | 0xFF800000;
    }
    return value;
}
int32_t CPU::sign_extend_26bit(uint32_t value) {
    if(value & 0x2000000) {  
        return value | 0xFC000000;
    }
    return value;
}
void CPU::add_breakpoint(addr_t addr) {
    breakpoint.insert(addr);
}
void CPU::remove_breakpoint(addr_t addr) {
    breakpoint.erase(addr);
}
void CPU::clear_breakpoints() {
    breakpoint.clear();
}
bool CPU::has_breakpoint(addr_t addr) const {
    return breakpoint.find(addr) != breakpoint.end();
}
void CPU::continue_execution() {
    paused = false;
}
void CPU::handle_interrupt() {
    u32 status = csr[static_cast<u8>(Csr::STATUS)];
    u32 ie = (status >> IE_LSB) & 1;

    if(ie == 0) {
        return;
    }

    u32 ipending = csr[static_cast<u8>(Csr::IPENDING)];
    u32 imask = csr[static_cast<u8>(Csr::IMASK)];
    u32 pending = ipending & imask;

    if(pending == 0) return;

    u8 code = 0;
    for(; code < 16; code++) {
        if(pending & (1u << code)) {
            break;
        }
    }

    // addr_t return_pc = pc;
    addr_t return_pc = pipe.getIDEX().is_valid ? pipe.getIDEX().curr_pc : pipe.getIFID().is_valid ? pipe.getIFID().curr_pc : pc;
    csr[static_cast<u8>(Csr::EPC)] = return_pc;
    csr[static_cast<u8>(Csr::CAUSE)] = ((1u << INTERRUPT_LSB) | code);

    u32 flags_value = (f.zf << 0) | (f.sf << 1) | (f.cf << 2) | (f.of << 3);
    csr[static_cast<u8>(Csr::EFLAGS)] = flags_value;

    status &= ~(1u << IE_LSB);
    status |= (1u << CSR_MODE_LSB); 
    status &= ~(1u << PIE_LSB);
    status |= (ie << PIE_LSB);
    csr[static_cast<u8>(Csr::STATUS)] = status;

    u32 ivtbr = csr[static_cast<u8>(Csr::IVTBR)];
    u32 offset = code * 4;
    u32 handler_addr = mem.read_u32(ivtbr + offset);

    addr_t sp = csr[static_cast<u8>(Csr::SP)];
    for(int i = 31; i >= 1; i--) {
        sp -= 4;
        mem.write_u32_direct(sp, registers[i]);
    }
    csr[static_cast<u8>(Csr::SCRATCH)] = sp;
    csr[static_cast<u8>(Csr::SP)] = sp;

    pc = handler_addr;
    // need_flush_idex = true;
    pipe.getIDEX().is_valid = false;
    pipe.getIFID().is_valid = false;

    ipending &= ~(1u << code);
    csr[static_cast<u8>(Csr::IPENDING)] = ipending;
}
void CPU::trigger_interrupt(u8 code) {
    u32 ipending = csr[static_cast<u8>(Csr::IPENDING)];
    ipending |= (1u << code);
    csr[static_cast<u8>(Csr::IPENDING)] = ipending;
}
void CPU::update_timer() {
    u32 tcontrol = csr[static_cast<u8>(Csr::TCONTROL)];
    u32 tcounter = csr[static_cast<u8>(Csr::TCOUNTER)];
    u32 tperiod = csr[static_cast<u8>(Csr::TPERIOD)];

    u32 enable = (tcontrol >> ENABLE_LSB) & ((1u << ENABLE_SIZE) - 1);
    if(enable == 0) return;

    tcounter++;
    if(tcounter >= tperiod) {
        tcounter = 0;
        trigger_interrupt(static_cast<u8>(Interrupt::TIMER));

        u32 tmode = (tcontrol >> TMODE_LSB) & ((1u << TMODE_SIZE) - 1);
        
        if(tmode == 0) { 
            tcontrol &= ~(1u << ENABLE_LSB);
        }
    }

    csr[static_cast<u8>(Csr::TCOUNTER)] = tcounter;
    csr[static_cast<u8>(Csr::TCONTROL)] = tcontrol;
}
void CPU::set_irq(int irq_num, bool value) {
    if(irq_num >= 0 && irq_num < 16) {
        u32 ipending = csr[static_cast<u8>(Csr::IPENDING)];
        if(value) {
            ipending |= (1u << irq_num);
        } else {
            ipending &= ~(1u << irq_num);
        }
        csr[static_cast<u8>(Csr::IPENDING)] = ipending;
    }
}
bool CPU::get_irq(int irq_num) {
    u32 ipending = csr[static_cast<u8>(Csr::IPENDING)];
    return (ipending >> irq_num) & 1;
}
void CPU::push_key(u8 c) {
    mem.push_key(c); 
    trigger_interrupt(static_cast<u8>(Interrupt::EXT0));
}