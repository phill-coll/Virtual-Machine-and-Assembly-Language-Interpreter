// ===== vm.h =====
#ifndef VM_H
#define VM_H

#include <string>
#include <fstream>

using namespace std;

class VirtualMachine {
public:
    VirtualMachine();                                  // Constructor
    void runFromFile(const string &filename);          // Runs instructions from file
    void dump();                                       // Writes final output to output.txt

private:
    char dataRegisters[8];                             // 8 general-purpose registers
    char memory[64];                                   // 64 bytes of memory
    char programCounter;                               // Program counter (PC)

    // Flags to track arithmetic/logic conditions
    bool carryFlag, overflowFlag, underflowFlag, zeroFlag;

    ofstream executeLog;                               // Stream for logging execution steps

    // Internal helpers
    void updateFlags(int result);                      // Updates status flags
    void trim(string &s);                              // Removes surrounding whitespace
    void toUpper(string &s);                           // Converts string to uppercase
    int getRegisterIndex(const string &s);             // Parses register name (e.g. R2)
    int resolveValue(const string &s);                 // Resolves literals, registers, or memory
    
    void binaryOp(const string &a, const string &b, char op); // ADD/SUB/MUL/DIV
    void inc_decrement(const string &reg, int delta);        // INC/DEC

    void mov(const string &src, const string &dst);
    void load(const string &addr, const string &dst);
    void store(const string &src, const string &addr);
    void input(const string &reg);
    void display(const string &reg);
    void rol(const string &reg, const string &cnt);
    void ror(const string &reg, const string &cnt);
    void shift(const string &reg, const string &cnt, bool left); // SHL/SHR
    void cleanOperand(string &s);                      // Removes non-printable chars/spaces
    bool parseInstruction(const string &line, string &opcode, string &op1, string &op2); // Splits line

    // Logging/output
    void printRegisters();                             // Write register values to log
    void printChangedMemory(char oldMemory[]);         // Show memory changes
    void processInstructionLine(string &line);         // Parse + execute line
    void execute(const string &opcode, const string &op1, const string &op2);
    void printRegistersToFile(ostream &out);           // For output.txt
    void printMemoryToFile(ostream &out);              // For output.txt
    void writeToFile(const string &filename);          // Writes to output.txt
};

#endif // VM_H