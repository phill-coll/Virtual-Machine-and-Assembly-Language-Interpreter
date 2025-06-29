// ===== vm.cpp =====
#include "VirtualMachine.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <bitset>
using namespace std;

// Constructor initializes memory, registers, flags
VirtualMachine::VirtualMachine()
    : programCounter(0), carryFlag(false), overflowFlag(false), underflowFlag(false), zeroFlag(false) {
    fill(begin(dataRegisters), end(dataRegisters), 0);
    fill(begin(memory), end(memory), 0);
}

// Updates processor flags based on result #BARAA
void VirtualMachine::updateFlags(int result) {
    if (result > 127 || result < -128)
        carryFlag = true;
    else
        carryFlag = false;

    if (result > 127)
        overflowFlag = true;
    else
        overflowFlag = false;

    if (result < -128)
        underflowFlag = true;
    else
        underflowFlag = false;

    if (result == 0)
        zeroFlag = true;
    else
        zeroFlag = false;
}

// Trims whitespace from string ends #IHSAN
void VirtualMachine::trim(string &s) {
    while (!s.empty() && isspace(s.front()))
        s.erase(s.begin());
    while (!s.empty() && isspace(s.back()))
        s.pop_back();
}

// Converts string to uppercase #IHSAN
void VirtualMachine::toUpper(string &s) {
    transform(s.begin(), s.end(), s.begin(), ::toupper);
}

// Converts register name to index (R0-R7)  #COLLINS
int VirtualMachine::getRegisterIndex(const string &s) {
    if (s.size() == 2 && (s[0] == 'R' || s[0] == 'r') && isdigit(s[1])) {
        int index = s[1] - '0';
        if (index >= 0 && index < 8) return index;
    }
    return -1;
}

// Resolves operand value: literal, register, or memory address  #COLLINS
int VirtualMachine::resolveValue(const string &s) {
    if (s.size() >= 3 && s.front() == '[' && s.back() == ']') {
        string inner = s.substr(1, s.size() - 2);
        int address = 0;
        if (isdigit(inner[0])) address = stoi(inner);
        else {
            int reg = getRegisterIndex(inner);
            if (reg != -1) address = (unsigned char)dataRegisters[reg];
        }
        if (address >= 0 && address < 64) return memory[address];
        return 0;
    }
    int reg = getRegisterIndex(s);
    return reg != -1 ? dataRegisters[reg] : stoi(s);
}

// Executes a binary operation (ADD, SUB, MUL, DIVIDE)#IHSAN
void VirtualMachine::binaryOp(const string &a, const string &b, char op) {
    int aIndex = getRegisterIndex(a);
    int bIndex = getRegisterIndex(b);
    // Ensure one operand is a register
    if (aIndex == -1 && bIndex == -1) {
        executeLog << "Error: At least one operand must be a register: " << a << ", " << b << endl;
        return;
    }
    // Determine destination register (must be one of them)
    int destIndex = (bIndex != -1) ? bIndex : aIndex;
    string srcOperand = (bIndex != -1) ? a : b;
    int srcVal = resolveValue(srcOperand);
    int destVal = dataRegisters[destIndex];
    int result = 0;
    switch (op) {
        case '+': result = destVal + srcVal; break;
        case '-': result = destVal - srcVal; break;
        case '*': result = destVal * srcVal; break;
        case '/':
            if (srcVal == 0) {
                executeLog << "Error: Division by zero.\n";
                return;
            }
            result = srcVal / destVal;
            break;
        default:
            executeLog << "Error: Unknown arithmetic operator '" << op << "'\n";
            return;
    }
    dataRegisters[destIndex] = result;
    updateFlags(result);
    executeLog << "Executed: " << op << " " << a << ", " << b 
               << " -> R" << destIndex << " = " << result << endl;
}

// Executes unary operations like INC/DEC  #COLLINS
void VirtualMachine::inc_decrement(const string &reg, int delta) {
        int i = getRegisterIndex(reg);
        if (i == -1) 
        return;
        int before = dataRegisters[i];
        dataRegisters[i] += delta;
        updateFlags(dataRegisters[i]);

        string bBefore = bitset<8>(static_cast<unsigned char>(before)).to_string();
        string bAfter = bitset<8>(static_cast<unsigned char>(dataRegisters[i])).to_string();
        executeLog << (delta > 0 ? "INC" : "DEC") << " R" << i << ": "
                   << bBefore << " -> " << bAfter << "\n";
    }

// MOV instruction: copies value to register  #COLLINS
void VirtualMachine::mov(const string &src, const string &dst) {
    int dest = getRegisterIndex(dst);
    if (dest == -1) return;
    int value = resolveValue(src);
    dataRegisters[dest] = value;
    updateFlags(dataRegisters[dest]);


     string binary = bitset<8>(static_cast<unsigned char>(value)).to_string();
    executeLog << "MOV " << src << " -> R" << dest << " = " << binary << "\n";
}

// LOADs value from memory to register  #COLLINS
void VirtualMachine::load(const string &addr, const string &dst) {
    int dest = getRegisterIndex(dst);
    if (dest == -1) {
        executeLog << "Error: Invalid destination register for LOAD: " << dst << endl;
        return;
    }
    int address = -1;
    // Handle memory access: [number] or [R2]
    if (addr.front() == '[' && addr.back() == ']') {
        string inner = addr.substr(1, addr.size() - 2);
        int regIndex = getRegisterIndex(inner);
        if (regIndex != -1) {
            address = (unsigned char)dataRegisters[regIndex]; // Indirect: [R2]
        } else {
            address = stoi(inner); // Direct: [20]
        }
    }
    if (address >= 0 && address < 64) {
        dataRegisters[dest] = memory[address];
        updateFlags(dataRegisters[dest]);
        executeLog << "Loaded memory[" << address << "] = " << (int)dataRegisters[dest]
                   << " into R" << dest << endl;
    } else {
        executeLog << "Error: Invalid address for LOAD: " << addr << endl;
    }
}

// STOREs register value into memory  #COLLINS
void VirtualMachine::store(const string &src, const string &addr) {
    int srcIndex = getRegisterIndex(src);
    if (srcIndex == -1) {
        executeLog << "Error: Invalid source register for STORE: " << src << endl;
        return;
    }

    int address = -1;
    if (addr.front() == '[' && addr.back() == ']') {
        // Indirect addressing: [R2]
        string inner = addr.substr(1, addr.size() - 2);
        int regIndex = getRegisterIndex(inner);
        if (regIndex != -1) {
            address = (unsigned char)dataRegisters[regIndex];
        }
    } else {
        // Direct addressing: 43
        address = stoi(addr);
    }
    if (address >= 0 && address < 64) {
        memory[address] = dataRegisters[srcIndex];
        executeLog << "Stored R" << srcIndex << " = " << (int)dataRegisters[srcIndex]
                   << " into memory[" << address << "]\n";
    } else {
        executeLog << "Error: Invalid memory address for STORE: " << addr << endl;
    }
}


// Gets input into register from user  #COLLINS
void VirtualMachine::input(const string &reg) {
    int i = getRegisterIndex(reg);
     if (i == -1) {
        executeLog << "Error: Invalid register for INPUT: " << reg << endl;
        return;
    }
    string in;
    cout << "? ";
    cin >> in;
    try {
        int val = stoi(in);
        dataRegisters[i] = val;
        updateFlags(val);
    } catch (...) {
        executeLog << "Error: Invalid input value: " << in << endl;
    }
}

// Displays register value to log  #COLLINS
void VirtualMachine::display(const string &reg) {
    int i = getRegisterIndex(reg);
    if (i != -1)
        executeLog << "R" << i << " = " << (int)dataRegisters[i] << endl;
}

// ROL operation: rotate bits left  #COLLINS
void VirtualMachine::rol(const string &reg, const string &cnt) {
    int i = getRegisterIndex(reg);
    if (i == -1) return;
    int c = stoi(cnt) % 8;
    unsigned char val = static_cast<unsigned char>(dataRegisters[i]);
    string before = bitset<8>(val).to_string();
    val = (val << c) | (val >> (8 - c));
    dataRegisters[i] = static_cast<char>(val);
    updateFlags(dataRegisters[i]);

    string after = bitset<8>(val).to_string();
    executeLog << "ROL R" << i << ": " << before << " -> " << after << " (by " << c << ")\n";
}

// ROR operation: rotate bits right  #COLLINS
void VirtualMachine::ror(const string &reg, const string &cnt) {
    int i = getRegisterIndex(reg);
    if (i == -1) return;
    int c = stoi(cnt) % 8;
    unsigned char val = static_cast<unsigned char>(dataRegisters[i]);
    string before = bitset<8>(val).to_string();

    val = (val >> c) | (val << (8 - c));
    dataRegisters[i] = static_cast<char>(val);
    updateFlags(dataRegisters[i]);

    string after = bitset<8>(val).to_string();
    executeLog << "ROR R" << i << ": " << before << " -> " << after << " (by " << c << ")\n";
}

// SHL/SHR operations: shift left or right #COLLINS
void VirtualMachine::shift(const string &reg, const string &cnt, bool left) {
        int i = getRegisterIndex(reg);
        if (i == -1) return;
        int c = stoi(cnt) % 8;
        unsigned char regVal = static_cast<unsigned char>(dataRegisters[i]);
        string beforeBinary = bitset<8>(regVal).to_string();

        unsigned char result = left ? (regVal << c) : (regVal >> c);
        dataRegisters[i] = static_cast<char>(result);
        updateFlags(dataRegisters[i]);

        string afterBinary = bitset<8>(result).to_string();
        executeLog << "Shift " << (left ? "left" : "right") << " on R" << i << ": "
                   << beforeBinary << " -> " << afterBinary << " (by " << c << ")\n";
    }

// Cleans operand of unwanted characters #BARAA
void VirtualMachine::cleanOperand(string &s) {
    string cleaned;
    for (unsigned char c : s) {
        if (isprint(c) && !isspace(c) && c != ' ') {
            cleaned += c;
        }
    }
    s = cleaned;
}

// Parses opcode and operands from line #IHSAN
bool VirtualMachine::parseInstruction(const string &line, string &opcode, string &op1, string &op2) {
    string tempLine = line;  // Make a modifiable copy

    // Remove comments
    size_t comment = tempLine.find(';');
    if (comment != string::npos)
        tempLine = tempLine.substr(0, comment);

    trim(tempLine); // trim whitespace from ends
    replace(tempLine.begin(), tempLine.end(), ',', ' '); // replace commas with space

    stringstream ss(tempLine);
    ss >> opcode >> op1 >> op2;

    trim(opcode);
    trim(op1);
    trim(op2);

    cleanOperand(op1);
    cleanOperand(op2);

    return !opcode.empty();  // true if opcode found
}
// Prints register values to execution log #SHARAF
void VirtualMachine::printRegisters() {
    executeLog << "Registers: ";
    for (int i = 0; i < 8; i++) {
        executeLog << setw(2) << setfill('0') << (int)(unsigned char)dataRegisters[i];
        if (i < 7) executeLog << " ";
    }
    executeLog << "#\n";
}

// Compares and logs any changed memory #SHARAF
void VirtualMachine::printChangedMemory(char oldMemory[]) {
    bool changed = false;
    for (int i = 0; i < 64; ++i) {
        if (memory[i] != oldMemory[i]) {
            executeLog << "Memory[" << i << "] = " << (int)(unsigned char)memory[i] << endl;
            changed = true;
        }
    }
    if (!changed) executeLog << "Memory: Unchanged\n";
}

// Processes each instruction line #IHSAN
void VirtualMachine::processInstructionLine(string &line) {
        trim(line);
        toUpper(line);
        if (line.empty()) return;
        // Fix spacing for comma
        size_t comma = line.find(',');
        if (comma != string::npos && line[comma + 1] != ' ')
            line.insert(comma + 1, " ");
        string opcode, op1, op2;
        if (!parseInstruction(line, opcode, op1, op2)) return;
        cleanOperand(op1);
        cleanOperand(op2);
        executeLog << "[" << (int)(programCounter + 1) << "] Executing: " << opcode;
        if (!op1.empty()) executeLog << " " << op1;
        if (!op2.empty()) executeLog << ", " << op2;
        executeLog << endl;
        char oldMemory[64];
        copy(begin(memory), end(memory), oldMemory);
        execute(opcode, op1, op2);
        printRegisters();
        printChangedMemory(oldMemory);
    }

// Executes the parsed instruction #IHSAN
void VirtualMachine::execute(const string &opcode, const string &op1, const string &op2) {
    if (opcode == "MOV") mov(op1, op2);
    else if (opcode == "ADD") binaryOp(op1, op2, '+');
    else if (opcode == "SUB") binaryOp(op1, op2, '-');
    else if (opcode == "MUL") binaryOp(op1, op2, '*');
    else if (opcode == "DIV") binaryOp(op1, op2, '/');
    else if (opcode == "INC") inc_decrement(op1, 1);
    else if (opcode == "DEC") inc_decrement(op1, -1);
    else if (opcode == "LOAD") load(op1, op2);
    else if (opcode == "STORE") store(op1, op2);
    else if (opcode == "INPUT") input(op1);
    else if (opcode == "DISPLAY") display(op1);
    else if (opcode == "ROL") rol(op1, op2);
    else if (opcode == "ROR") ror(op1, op2);
    else if (opcode == "SHL") shift(op1, op2, true);
    else if (opcode == "SHR") shift(op1, op2, false);
}

// Writes registers to a file #SHARAF
void VirtualMachine::printRegistersToFile(ostream &out) {
    for (int i = 0; i < 8; i++) {
        out << setw(2) << setfill('0') << (int)(unsigned char)dataRegisters[i];
        if (i < 7) out << " ";
    }
    out << "#\n";
}

// Writes entire memory block to file SHARAF
void VirtualMachine::printMemoryToFile(ostream &out) {
    for (int i = 0; i < 64; ++i) {
        out << setw(2) << setfill('0') << (int)(unsigned char)memory[i] << " ";
        if ((i + 1) % 8 == 0) out << "\n";
    }
    out << "#\n";
}

// Saves all final state to output file #BARAA
void VirtualMachine::writeToFile(const string &filename) {
    ofstream out(filename);
    out << "Registers: ";
    printRegistersToFile(out);
    out << "Flags    : " << carryFlag << "  " << overflowFlag << "  " << underflowFlag << "  " << zeroFlag << "#\n";
    out << "PC       : " << (int)programCounter << "\n\nMemory   :\n";
    printMemoryToFile(out);
}

// Reads and executes instructions from a file #IHSAN
void VirtualMachine::runFromFile(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open file: " << filename << endl;
        return;
    }

    executeLog.open("execute.txt");
    if (!executeLog.is_open()) {
        cerr << "Could not open log file: execute.txt" << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        processInstructionLine(line);
        programCounter++;
    }

    programCounter++;
    dump();
    file.close();
    executeLog.close();   
}

// Optional manual dump to output.txt #BARAA
void VirtualMachine::dump() {
    writeToFile("output.txt");
}