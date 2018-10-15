#include "bingen.h"

#include <sstream>
#include <string>
#include <utility>

std::map<std::string, int> regmap =
{
    { "zero", 0 },
    { "ra", 1 },
    { "sp", 2 },
    { "gp", 3 },
    { "tp", 4 },
    { "t0", 5 },
    { "t1", 6 },
    { "t2", 7 },
    { "s0", 8 },
    { "fp", 8 },
    { "s1", 9 },
    { "a0", 10 },
    { "a1", 11 },
    { "a2", 12 },
    { "a3", 13 },
    { "a4", 14 },
    { "a5", 15 },
    { "a6", 16 },
    { "a7", 17 },
    { "s2", 18 },
    { "s3", 19 },
    { "s4", 20 },
    { "s5", 21 },
    { "s6", 22 },
    { "s7", 23 },
    { "s8", 24 },
    { "s9", 25 },
    { "s10", 26 },
    { "s11", 27 },
    { "t3", 28 },
    { "t4", 29 },
    { "t5", 30 },
    { "t6", 31 }
};

BinGen::BinGen(std::ofstream ofs)
  : ofs_(std::move(ofs)) {}

uint32_t BinGen::lui (std::string rd, uint32_t imm) {
    CheckImmediate(imm, 20, "lui");
    Fields fields;
    fields.emplace_back(7, 0b0110111);
    fields.emplace_back(5, regmap[rd]);
    fields.emplace_back(20, imm);
    return Pack(fields);
}

uint32_t BinGen::auipc (std::string rd, uint32_t imm) {
    CheckImmediate(imm, 20, "auipc");
    Fields fields;
    fields.emplace_back(7, 0b0010111);
    fields.emplace_back(5, regmap[rd]);
    fields.emplace_back(20, imm);
    return Pack(fields);
}

uint32_t BinGen::jal (std::string rd, uint32_t imm) {
    CheckImmediate(imm, 20, "jal");
    Fields fields;
    fields.emplace_back(7, 0b1101111);
    fields.emplace_back(5, regmap[rd]);
    fields.emplace_back(8, imm & 0x7f800);
    fields.emplace_back(1, imm & 0x400);
    fields.emplace_back(10, imm & 0x3ff);
    fields.emplace_back(1, imm & 0x80000);
    return Pack(fields);
}

uint32_t BinGen::jalr (std::string rd, std::string rs1, uint32_t imm) {
    CheckImmediate(imm, 12, "jalr");
    Fields fields;
    fields.emplace_back(7, 0b1100111);
    fields.emplace_back(5, regmap[rd]);
    fields.emplace_back(3, 0);
    fields.emplace_back(5, regmap[rs1]);
    fields.emplace_back(12, imm);
    return Pack(fields);
}

// beq, bne, blt, bge, bltu, bgeu
uint32_t BinGen::branch (std::string mnemo, std::string rs1, std::string rs2, uint32_t offset) {
    CheckImmediate(offset, 12, "jalr");
    uint32_t funct3;
    if (mnemo == "beq") funct3 = 0b000;
    if (mnemo == "bne") funct3 = 0b001;
    if (mnemo == "blt") funct3 = 0b100;
    if (mnemo == "bge") funct3 = 0b101;
    if (mnemo == "bltu") funct3 = 0b110;
    if (mnemo == "bgeu") funct3 = 0b111;
    Fields fields;
    fields.emplace_back(7, 0b1100011);
    fields.emplace_back(1, offset & 0x400);
    fields.emplace_back(4, offset & 0xf);
    fields.emplace_back(3, funct3);
    fields.emplace_back(5, regmap[rs1]);
    fields.emplace_back(5, regmap[rs2]);
    fields.emplace_back(6, offset & 0x3f0);
    fields.emplace_back(1, offset & 0x800);
    return Pack(fields);
}

// lb, lh, lw, lbu, lhu
uint32_t BinGen::load (std::string mnemo, std::string rd, std::string rs1, uint32_t offset) {
    CheckImmediate(offset, 12, "load");
    uint32_t funct3;
    if (mnemo == "lb") funct3 = 0b000;
    if (mnemo == "lh") funct3 = 0b001;
    if (mnemo == "lw") funct3 = 0b010;
    if (mnemo == "lbu") funct3 = 0b100;
    if (mnemo == "lhu") funct3 = 0b101;
    Fields fields;
    fields.emplace_back(7, 0b0000011);
    fields.emplace_back(5, regmap[rd]);
    fields.emplace_back(3, 0);
    fields.emplace_back(5, regmap[rs1]);
    fields.emplace_back(12, offset);
    return Pack(fields);
}

// sb, sh, sw
uint32_t BinGen::store (std::string mnemo, std::string rs2, std::string rs1, uint32_t offset) {
    CheckImmediate(offset, 12, "store");
    uint32_t funct3;
    if (mnemo == "sb") funct3 = 0b000;
    if (mnemo == "sh") funct3 = 0b001;
    if (mnemo == "sw") funct3 = 0b010;
    Fields fields;
    fields.emplace_back(7, 0b0100011);
    fields.emplace_back(5, offset & 0x1f);
    fields.emplace_back(3, funct3);
    fields.emplace_back(5, regmap[rs1]);
    fields.emplace_back(5, regmap[rs2]);
    fields.emplace_back(7, offset & 0xfe);
    return Pack(fields);
}

// addi, slti, sltiu, xori, ori, andi
uint32_t BinGen::op_imm (std::string mnemo, std::string rd, std::string rs1, uint32_t imm) {
    CheckImmediate(imm, 12, "op_imm");
    uint32_t funct3;
    if (mnemo == "addi")  funct3 = 0b000;
    if (mnemo == "slti")  funct3 = 0b010;
    if (mnemo == "sltiu") funct3 = 0b011;
    if (mnemo == "xori")  funct3 = 0b100;
    if (mnemo == "ori")   funct3 = 0b110;
    if (mnemo == "andi")  funct3 = 0b111;
    Fields fields;
    fields.emplace_back(7, 0b0010011);
    fields.emplace_back(5, regmap[rd]);
    fields.emplace_back(3, funct3);
    fields.emplace_back(5, regmap[rs1]);
    fields.emplace_back(12, imm);
    return Pack(fields);
}

// slli, srli, srai
uint32_t BinGen::op_imm_shift (std::string mnemo, std::string rd, std::string rs1, uint32_t shamt) {
    CheckImmediate(shamt, 5, "op_imm_shift");
    uint32_t funct3 = (mnemo == "slli") ? 0b001 : 0b101;
    uint32_t funct7 = (mnemo == "srai") ? 0b0100000 : 0b0000000;
    Fields fields;
    fields.emplace_back(7, 0b0010011);
    fields.emplace_back(5, regmap[rd]);
    fields.emplace_back(3, funct3);
    fields.emplace_back(5, regmap[rs1]);
    fields.emplace_back(5, shamt);
    fields.emplace_back(7, funct7);
    return Pack(fields);
}

// add, sub, sll, slt, sltu, xor, srl, sra, or, and
uint32_t BinGen::op (std::string mnemo, std::string rd, std::string rs1, std::string rs2) {
    uint32_t funct3;
    if (mnemo == "add")  funct3 = 0b000;
    if (mnemo == "sub")  funct3 = 0b000;
    if (mnemo == "sll")  funct3 = 0b001;
    if (mnemo == "slt")  funct3 = 0b010;
    if (mnemo == "sltu") funct3 = 0b011;
    if (mnemo == "xor")  funct3 = 0b100;
    if (mnemo == "srl")  funct3 = 0b101;
    if (mnemo == "sra")  funct3 = 0b101;
    if (mnemo == "or")   funct3 = 0b110;
    if (mnemo == "and")  funct3 = 0b111;
    uint32_t funct7 = (mnemo == "sub" || mnemo == "sra") ? 0b0100000 : 0b0000000;
    Fields fields;
    fields.emplace_back(7, 0b0110011);
    fields.emplace_back(5, regmap[rd]);
    fields.emplace_back(3, funct3);
    fields.emplace_back(5, regmap[rs1]);
    fields.emplace_back(5, regmap[rs2]);
    fields.emplace_back(7, funct7);
    return Pack(fields);
}

void BinGen::WriteData(uint32_t data) {
    char d[4];
    *d = data;
    ofs_.write(d, 4);
}

void BinGen::ReadLabels(std::string input) {
    if (input.back() != ':') {
        // The input wasn't a label.

        std::istringstream istr(input);
        std::string mnemo;
        istr >> mnemo;

        // Some pseudo-instructions will expand to two instrs
        if (mnemo == "la" || mnemo == "ret" || mnemo == "call") {
            nline_ += 2;
            return;
        }

        nline_++;
        return;
    }
    input.pop_back();
    std::cout << input << std::endl;
    label_map_[input] = nline_;
}

// dirty...
void BinGen::Parse(std::string input, std::string &mnemo, std::vector<std::string> &arg) {
    int curr_pos = 0;
    int start_pos = 0;
    while (input[curr_pos] == ' ' || input[curr_pos] == '\t') curr_pos++;

    // mnemonic (or label)
    start_pos = curr_pos;
    while (!(input[curr_pos] == ' ' || input[curr_pos] == '\t' || input[curr_pos] == '\0')) curr_pos++;
    mnemo = input.substr(start_pos, curr_pos - start_pos);
    while (input[curr_pos] == ' ' || input[curr_pos] == '\t') curr_pos++;
    if (input[curr_pos] == '\0') return;

    // arg[0]
    start_pos = curr_pos;
    while (!(input[curr_pos] == ' ' || input[curr_pos] == '\t' || input[curr_pos] == ',' || input[curr_pos] == '\0')) curr_pos++;
    arg.push_back(input.substr(start_pos, curr_pos - start_pos));
    while (input[curr_pos] == ' ' || input[curr_pos] == '\t') curr_pos++;
    if (input[curr_pos] == '\0') return;

    // arg[1]
    start_pos = curr_pos;
    while (!(input[curr_pos] == ' ' || input[curr_pos] == '\t' || input[curr_pos] == ',' || input[curr_pos] == '\0')) curr_pos++;
    arg.push_back(input.substr(start_pos, curr_pos - start_pos));
    while (input[curr_pos] == ' ' || input[curr_pos] == '\t') curr_pos++;
    if (input[curr_pos] == '\0') return;

    // arg[2]
    start_pos = curr_pos;
    while (!(input[curr_pos] == '\0')) curr_pos++;
    arg.push_back(input.substr(start_pos, curr_pos - start_pos));
}

void BinGen::Convert(std::string input) {
    // Parse the input.
    std::string mnemo;
    std::vector<std::string> arg;
    Parse(input, mnemo, arg);
    if (mnemo.back() == ':') {
        // Skip the labels.
        return;
    }

    // Note: Lack of arguments will cause crash here
    if (mnemo == "lui")
        WriteData(lui(arg[0], std::stoi(arg[1], nullptr, 16)));
    else if (mnemo == "auipc")
        WriteData(auipc(arg[0], std::stoi(arg[1], nullptr, 16)));
    else if (mnemo == "jal")
        WriteData(jal(arg[0], std::stoi(arg[1], nullptr, 16)));
    else if (mnemo == "jalr")
        WriteData(jalr(arg[0], arg[1], std::stoi(arg[2], nullptr, 16)));
    else if (mnemo == "beq" || mnemo == "bne" || mnemo == "blt" || mnemo == "bge" || mnemo == "bltu") {
        WriteData(branch(mnemo, arg[0], arg[1], MyStoi(arg[2])));
    }

    else if (mnemo == "lb" || mnemo == "lh" || mnemo == "lw" || mnemo == "lbu" || mnemo == "lhu") {
        std::string rs1; uint32_t offset;
        ParseOffset(arg[1], &rs1, &offset);
        WriteData(load(mnemo, arg[0], rs1, offset));
    }

    else if (mnemo == "sb" || mnemo == "sh" || mnemo == "sw") {
        std::string rs1; uint32_t offset;
        ParseOffset(arg[1], &rs1, &offset);
        WriteData(store(mnemo, arg[0], rs1, offset));
    }

    else if (mnemo == "addi" || mnemo == "slti" || mnemo == "sltiu" || mnemo == "xori" || mnemo == "ori" || mnemo == "andi")
        WriteData(op_imm(mnemo, arg[0], arg[1], std::stoi(arg[2], nullptr, 16)));
    else if (mnemo == "slli" || mnemo == "srli" || mnemo == "srai")
        WriteData(op_imm_shift(mnemo, arg[0], arg[1], std::stoi(arg[2], nullptr, 16)));
    else if (mnemo == "add" || mnemo == "sub" || mnemo == "sll" || mnemo == "slt" || mnemo == "sltu" || mnemo == "xor" ||
        mnemo == "srl" || mnemo == "sra" || mnemo == "or" || mnemo == "and")
        WriteData(op(mnemo, arg[0], arg[1], arg[2]));


    // Pseudo-instructions
    else if (mnemo == "ret") {
        WriteData(jalr("x0", "x1", 0u));
    }

    else if (mnemo == "call") {
        WriteData(auipc("x6", MyStoi(arg[0]) >> 12));
        WriteData(jalr("x1", "x6", MyStoi(arg[0]) & 0xfff));
    }
}

uint32_t BinGen::Pack(Fields fields) {
    uint32_t ret = 0;
    for (const auto& field : fields) {
        ret <<= field.first;
        ret += field.second;
    }
    return ret;
}

void BinGen::CheckImmediate(uint32_t imm, int range, std::string func_name) {
    if (imm >= (1 << range)) {
        std::cout << "ERROR(" << func_name << "): The immediate value should be smaller than 2 ^ " << range << std::endl;
        exit(1);
    }
}

void BinGen::ParseOffset(std::string arg, std::string* reg, uint32_t* offset) {
    size_t pos_lpar = arg.find("(");
    size_t pos_rpar = arg.find(")");
    *offset = std::stoi(arg.substr(0, pos_lpar));
    *reg = arg.substr(pos_lpar + 1, (pos_rpar - pos_lpar - 1));
}

uint32_t BinGen::MyStoi(std::string imm) {
    try {
        return std::stoi(imm, nullptr, 16);
    }
    catch (...) {
        // |imm| was a label.
        return label_map_[imm];
    }
}
