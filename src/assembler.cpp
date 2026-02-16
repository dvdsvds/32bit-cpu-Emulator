#include "encoder/encoder.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

using namespace encoder;

struct Label {
    std::string name;
    uint32_t address;
};

class Assembler {
private:
    std::map<std::string, uint32_t> labels;
    std::vector<inst_t> instructions;
    uint32_t current_address;

    std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while(std::getline(ss, token, delimiter)) {
            // trim
            size_t start = token.find_first_not_of(" \t");
            size_t end = token.find_last_not_of(" \t");
            if(start != std::string::npos && end != std::string::npos) {
                tokens.push_back(token.substr(start, end - start + 1));
            }
        }
        return tokens;
    }

    int parseRegister(const std::string& reg) {
        if(reg[0] == 'r') {
            if(reg == "r_temp") return 31;
            return std::stoi(reg.substr(1));
        }
        std::cerr << "Invalid register: " << reg << std::endl;
        return -1;
    }

    int parseImmediate(const std::string& imm) {
        if(labels.find(imm) != labels.end()) {
            return labels[imm];
        }
        if(imm.find("0x") == 0) {
            return std::stoi(imm, nullptr, 16);
        }
        return std::stoi(imm);
    }

    void firstPass(const std::vector<std::string>& lines) {
        current_address = 0;
        
        for(const auto& line : lines) {
            std::string trimmed = line;
            size_t comment = trimmed.find("//");
            if(comment != std::string::npos) {
                trimmed = trimmed.substr(0, comment);
            }
            
            size_t start = trimmed.find_first_not_of(" \t");
            size_t end = trimmed.find_last_not_of(" \t");
            if(start == std::string::npos) continue;
            trimmed = trimmed.substr(start, end - start + 1);
            
            if(trimmed.empty()) continue;
            
            if(trimmed.back() == ':') {
                std::string label = trimmed.substr(0, trimmed.length() - 1);
                labels[label] = current_address;
                std::cout << "Label: " << label << " -> " << current_address << std::endl;
            } else {
                current_address += 4;
            }
        }
    }

    void secondPass(const std::vector<std::string>& lines) {
        current_address = 0;
        
        for(const auto& line : lines) {
            std::string trimmed = line;
            size_t comment = trimmed.find("//");
            if(comment != std::string::npos) {
                trimmed = trimmed.substr(0, comment);
            }
            
            size_t start = trimmed.find_first_not_of(" \t");
            size_t end = trimmed.find_last_not_of(" \t");
            if(start == std::string::npos) continue;
            trimmed = trimmed.substr(start, end - start + 1);
            
            if(trimmed.empty()) continue;
            if(trimmed.back() == ':') continue; // 레이블 스킵
            
            inst_t inst = parseInstruction(trimmed);
            instructions.push_back(inst);
            current_address += 4;
        }
    }

    int parseCSR(const std::string& name) {
        if(name == "EPC") return 0;
        if(name == "CAUSE") return 1;
        if(name == "STATUS") return 2;
        if(name == "IVTBR") return 3;
        if(name == "IMASK") return 4;
        if(name == "IPENDING") return 5;
        if(name == "SEPC") return 6;
        if(name == "SSTATUS") return 7;
        if(name == "SCRATCH") return 8;
        if(name == "CYCLE") return 9;
        if(name == "TPERIOD") return 10;
        if(name == "TCONTROL") return 11;
        if(name == "TCOUNTER") return 12;
        if(name == "EFLAGS") return 13;
        if(name == "SP") return 14;
        return -1;
    }

    bool isImm(const std::string& arg) {
        if(arg.empty()) return false;
        return std::isdigit(arg[0]) || 
               (arg[0] == '0' && arg.size() > 1 && arg[1] == 'x') || 
               (arg[0] == '-' && arg.size() > 1 && std::isdigit(arg[1])); 
    }

    inst_t parseInstruction(const std::string& line) {
        size_t space = line.find(' ');
        if(space == std::string::npos) {
            std::string opcode = line;
            if(opcode == "ret") { return RET(); }
            if(opcode == "hlt") { return HLT(); }
            if(opcode == "nop") { return NOP(); }
            if(opcode == "syscall") { return SYSCALL(); }
            if(opcode == "iret") { return IRET(); }
            return NOP();
        }

        std::string opcode = line.substr(0, space);
        std::string rest = line.substr(space + 1);

        size_t start = rest.find_first_not_of(" \t");
        size_t end = rest.find_last_not_of(" \t");
        if(start != std::string::npos) {
            rest = rest.substr(start, end - start + 1);
        }

        auto args = split(rest, ',');
        if(opcode == ".word") { return static_cast<inst_t>(parseImmediate(args[0])); }
        
        // ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, SHL, SHR
        if(opcode == "add" || opcode == "sub" || opcode == "mul" || opcode == "div" || 
           opcode == "and" || opcode == "or" || opcode == "xor" ||
           opcode == "shl" || opcode == "shr") {
            
            int rd = parseRegister(args[0]);
            int rs1 = parseRegister(args[1]);
            int rs2 = parseRegister(args[2]);
            
            if(opcode == "add") return ADD(rd, rs1, rs2, Mode::REGISTER);
            if(opcode == "sub") return SUB(rd, rs1, rs2, Mode::REGISTER);
            if(opcode == "mul") return MUL(rd, rs1, rs2, Mode::REGISTER);
            if(opcode == "div") return DIV(rd, rs1, rs2, Mode::REGISTER);
            if(opcode == "and") return AND(rd, rs1, rs2, Mode::REGISTER);
            if(opcode == "or") return OR(rd, rs1, rs2, Mode::REGISTER);
            if(opcode == "xor") return XOR(rd, rs1, rs2, Mode::REGISTER);
            if(opcode == "shl") return SHL(rd, rs1, rs2, Mode::REGISTER);
            if(opcode == "shr") return SHR(rd, rs1, rs2, Mode::REGISTER);
        }
        
        // CMP
        if(opcode == "cmp") {
            int rs1 = parseRegister(args[0]);
            if(args[1][0] == 'r') {
                int rs2 = parseRegister(args[1]);
                return CMP(rs1, rs2, Mode::REGISTER);
            } else {
                int imm = parseImmediate(args[1]);
                return CMP(rs1, imm, Mode::IMMEDIATE);
            }
        }
        
        // MOV
        if(opcode == "mov") {
            int rd = parseRegister(args[0]);
            
            if(args[1][0] == 'r') {
                int rs1 = parseRegister(args[1]);
                return MOV(rd, rs1);
            } else {
                int imm = parseImmediate(args[1]);
                return LOADI(rd, imm);
            }
        }
        
        // JMP
        if(opcode == "jmp") {
            std::string target = args[0];
            int offset = labels[target] - current_address;
            return JMP(offset);
        }
        
        // BJMP
        if(opcode == "bjmp") {
            std::string cond_str = args[0];
            std::string target = args[1];
            
            Cond cond;
            if(cond_str == "EQ") cond = Cond::EQ;
            else if(cond_str == "NE") cond = Cond::NE;
            else if(cond_str == "LT") cond = Cond::LT;
            else if(cond_str == "LE") cond = Cond::LE;
            else if(cond_str == "GT") cond = Cond::GT;
            else if(cond_str == "GE") cond = Cond::GE;
            
            int offset = labels[target] - current_address;
            return BJMP(cond, offset);
        }
        
        if(opcode == "call") {
            std::string target = args[0];
            int offset = labels[target] - current_address;
            return CALL(offset);
        }
        
        // LOADW, STOREW
        if(opcode == "loadw" || opcode == "storew") {
            int rd = parseRegister(args[0]);
            std::string addr = args[1];
            size_t paren = addr.find('(');
            int offset = parseImmediate(addr.substr(0, paren));
            int rs1 = parseRegister(addr.substr(paren + 1, addr.find(')') - paren - 1));
            
            if(opcode == "loadw") return LOADW(rd, rs1, offset);
            if(opcode == "storew") return STOREW(rd, rs1, offset);
        }
        
        // CSRR, CSRW
        if(opcode == "csrr") {
            int rd = parseRegister(args[0]);
            int csr = parseCSR(args[1]);
            return CSRR(rd, csr, Mode::REGISTER); 
        }
        
        if(opcode == "csrw") {
            int csr = parseCSR(args[0]);
            if(isImm(args[1])) {
                int imm = parseImmediate(args[1]);
                return CSRW(imm, csr, Mode::IMMEDIATE);
            } else {
                int rs = parseRegister(args[1]);
                return CSRW(rs, csr, Mode::REGISTER);
            }
        }
        
        if(opcode == "push") {
            int rs = parseRegister(args[0]);
            return PUSH(rs);
        }

        if(opcode == "pop") {
            int rs = parseRegister(args[0]);
            return POP(rs);
        }
        return NOP();
    }

public:
    std::vector<inst_t> assemble(const std::string& filename) {
        std::ifstream file(filename);
        if(!file.is_open()) {
            std::cerr << "Failed to open: " << filename << std::endl;
            return {};
        }
        
        std::vector<std::string> lines;
        std::string line;
        while(std::getline(file, line)) {
            lines.push_back(line);
        }
        file.close();
        
        std::cout << "=== First Pass ===" << std::endl;
        firstPass(lines);
        
        std::cout << "\n=== Second Pass ===" << std::endl;
        secondPass(lines);
        
        return instructions;
    }

    void error(int line, const std::string& msg) {
        std::cerr << "Error (line " << line << "):" << msg << std::endl;
    }
};

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: assembler <input.asm>" << std::endl;
        return 1;
    }
    
    Assembler assembler;
    std::vector<inst_t> program = assembler.assemble(argv[1]);
    
    std::cout << "\n=== Generated " << program.size() << " instructions ===" << std::endl;
    
    // 바이너리 파일로 저장
    std::ofstream out("program.bin", std::ios::binary);
    out.write(reinterpret_cast<const char*>(program.data()), program.size() * sizeof(inst_t));
    out.close();
    
    std::cout << "Binary saved to: program.bin" << std::endl;
    
    return 0;
}