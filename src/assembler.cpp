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
            if(reg == "r_temp") return 31; // 임시 레지스터
            return std::stoi(reg.substr(1));
        }
        return 0;
    }

    int parseImmediate(const std::string& imm) {
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
            
            // 레이블 체크
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

    inst_t parseInstruction(const std::string& line) {
        auto tokens = split(line, ' ');
        if(tokens.empty()) return 0;
        
        std::string opcode = tokens[0];
        
        // ADD, SUB, MUL, DIV, MOD, AND, OR, XOR, SHL, SHR
        if(opcode == "add" || opcode == "sub" || opcode == "mul" || opcode == "div" || 
           opcode == "and" || opcode == "or" || opcode == "xor" ||
           opcode == "shl" || opcode == "shr") {
            
            auto args = split(tokens[1], ',');
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
            auto args = split(tokens[1], ',');
            int rs1 = parseRegister(args[0]);
            int rs2 = parseRegister(args[1]);
            return CMP(rs1, rs2, Mode::REGISTER);
        }
        
        // MOV
        if(opcode == "mov") {
            auto args = split(tokens[1], ',');
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
            std::string target = tokens[1];
            int offset = labels[target] - (current_address + 4);
            return JMP(offset);
        }
        
        // BJMP
        if(opcode == "bjmp") {
            auto args = split(tokens[1], ',');
            std::string cond_str = args[0];
            std::string target = args[1];
            
            Cond cond;
            if(cond_str == "EQ") cond = Cond::EQ;
            else if(cond_str == "NE") cond = Cond::NE;
            else if(cond_str == "LT") cond = Cond::LT;
            else if(cond_str == "LE") cond = Cond::LE;
            else if(cond_str == "GT") cond = Cond::GT;
            else if(cond_str == "GE") cond = Cond::GE;
            
            int offset = labels[target] - (current_address + 4);
            return BJMP(cond, offset);
        }
        
        // CALL
        if(opcode == "call") {
            std::string target = tokens[1];
            int offset = labels[target] - (current_address + 4);
            return CALL(offset);
        }
        
        // RET
        if(opcode == "ret") {
            return RET();
        }
        
        // LOADW, STOREW
        if(opcode == "loadw" || opcode == "storew") {
            auto args = split(tokens[1], ',');
            int rd = parseRegister(args[0]);
            
            // offset(rs1) 파싱
            std::string addr = args[1];
            size_t paren = addr.find('(');
            int offset = parseImmediate(addr.substr(0, paren));
            int rs1 = parseRegister(addr.substr(paren + 1, addr.find(')') - paren - 1));
            
            if(opcode == "loadw") return LOADW(rd, rs1, offset);
            if(opcode == "storew") return STOREW(rd, rs1, offset);
        }
        
        // CSRR, CSRW
        if(opcode == "csrr") {
            auto args = split(tokens[1], ',');
            int rd = parseRegister(args[0]);
            // CSR 이름 파싱 필요 (SP 등)
            return CSRR(rd, 0, Mode::REGISTER); // TODO: CSR 번호 매핑
        }
        
        if(opcode == "csrw") {
            auto args = split(tokens[1], ',');
            int rs = parseRegister(args[0]);
            return CSRW(rs, 0, Mode::REGISTER); // TODO: CSR 번호 매핑
        }
        
        // HLT
        if(opcode == "hlt") {
            return HLT();
        }
        
        // NOP
        if(opcode == "nop") {
            return NOP();
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