#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>

using namespace std;

// Display contents
char R[8];               // Registers
char memory[64] = {0};   // Memory
char ProgramCounter = 0; // Program Counter
bool carry = false;      // Flags
bool zero = false;
bool overflow = false;
bool underflow = false;

vector<string> lines; // hold lines from .asm file

void OpenFile() // Open program file
{
    ifstream asmfile;
    asmfile.open("program.asm");
    // if program file not found in directory give error
    if (!asmfile.is_open())
    {
        cout << "File not found..." << endl;
        exit(-1);
    }
    // following code purpose is to extract each line then split the line into opcode and operands
    string commands;
    while (getline(asmfile, commands))
    {
        if (commands.empty())
            continue; // this will skip empty lines in program file
        // convert the commands in .asm file into uppercase commands
        for (int x = 0; x < commands.size(); x++)
        {
            commands[x] = toupper(commands[x]);
        }
        lines.push_back(commands); // this will save commands lines into 'lines' vector
    }

    asmfile.close();
}

void ArithmeticOperations(string opcode, string operand1, string operand2)
{
    if (opcode == "ADD")
    {
        cout << "Add " << operand1 << " to " << operand2 << endl;
    }
    else if (opcode == "SUB")
    {
        cout << "Sub " << operand1 << " to " << operand2 << endl;
    }
    else if (opcode == "MUL")
    {
        cout << "Mul " << operand1 << " to " << operand2 << endl;
    }
    else if (opcode == "DIV")
    {
        cout << "Div " << operand1 << " to " << operand2 << endl;
    }
}

void IncrementDecrement(string opcode, string operand1, string operand2)
{
    if (opcode == "INC")
    {
        cout << "INC " << operand1 << " " << operand2 << " times " << endl;
    }
    else if (opcode == "DEC")
    {
        cout << "DEC " << operand1 << " " << operand2 << " times " << endl;
    }
}

void Transfer(string opcode, string operand1, string operand2)
{
    if (opcode == "MOV")
    {
        cout << "Move " << operand1 << " into " << operand2 << endl;
    }
    else if (opcode == "STORE")
    {
        cout << "Store " << operand1 << " into " << operand2 << endl;
    }
    else if (opcode == "LOAD")
    {
        cout << "Load from " << operand1 << " into " << operand2 << endl;
    }
}

void Rotate(string opcode, string operand1, string operand2)
{
    if (opcode == "ROL")
    {
        cout << "Rotate " << operand1 << " into left " << operand2 << " times" << endl;
    }
    else if (opcode == "ROR")
    {
        cout << "Rotate " << operand1 << " into right " << operand2 << " times" << endl;
    }
}

void Shift(string opcode, string operand1, string operand2)
{
    if (opcode == "SHL")
    {
        cout << "Shift " << operand1 << " into left " << operand2 << " times" << endl;
    }
    else if (opcode == "SHR")
    {
        cout << "Shift " << operand1 << " into right " << operand2 << " times" << endl;
    }
}

void InOut(string opcode, string operand1, string operand2)
{
    if (opcode == "INPUT")
    {
        cout << "input into " << operand1 << endl;
    }
    else if (opcode == "DISPLAY")
    {
        cout << "Display value in " << operand1 << endl;
    }
};

// A if statement which branches into different functions according to the opcode type
void opbranching(string opcode, string operand1, string operand2)
{
    if (opcode == "ADD" || opcode == "SUB" || opcode == "MUL" || opcode == "DIV")
    {
        ArithmeticOperations(opcode, operand1, operand2);
    }
    else if (opcode == "INC" || opcode == "DEC")
    {
        IncrementDecrement(opcode, operand1, operand2);
    }
    else if (opcode == "LOAD" || opcode == "STORE" || opcode == "MOV")
    {
        Transfer(opcode, operand1, operand2);
    }
    else if (opcode == "ROL" || opcode == "ROR") // eg: ROL, R0, 1
    {
        Rotate(opcode, operand1, operand2);
    }
    else if (opcode == "SHL" || opcode == "SHR") // eg: SHL R0, 1
    {
        Shift(opcode, operand1, operand2);
    }
    else if (opcode == "INPUT" || opcode == "DISPLAY")
    {
        InOut(opcode, operand1, operand2);
    }
}

/*Im trying to make a function which checks the opcode if valid,
if not its supposed to terminate the program whitout reading any line*/
bool ValidateOpcode(string opcode, string operand1, string operand2)
{

    if (!(opcode == "ADD" || opcode == "SUB" || opcode == "MUL" || opcode == "DIV" || opcode == "INC" || opcode == "DEC" || opcode == "LOAD" || opcode == "STORE" || opcode == "MOV" || opcode == "ROL" || opcode == "ROR" || opcode == "SHL" || opcode == "SHR" || opcode == "INPUT" || opcode == "DISPLAY"))
    {
        // exit(1);
        return false;
    }
    else
        // opbranching(opcode, operand1, operand2);
        return true;
}

// this function will split each line in the .asm file into 3 parts (opcode and two operands)
void SplitOperands()
{
    // loop to every line through the .asm file
    for (int x = 0; x < lines.size(); x++)
    {
        string opcode, operand1, operand2, extra;
        stringstream linesbehalf(lines[x]);
        getline(linesbehalf, opcode, ' ');
        getline(linesbehalf, operand1, ',');
        getline(linesbehalf, operand2, ',');
        getline(linesbehalf, extra);                        // check and take in extra content on the same line if found
        extra.erase(0, extra.find_first_not_of(" \t\r\n")); // erase the space after the comma for both extra and operand2
        operand2.erase(0, operand2.find_first_not_of(" \t\r\n"));
        // if extra content found after operand2 then display and error and terminate program
        if (!extra.empty())
        {
            cout << "line " << x + 1 << " Invalid format" << endl;
            exit(1);
        }
        ValidateOpcode(opcode, operand1, operand2);
        if (!ValidateOpcode(opcode, operand1, operand2))
        {
            cout << "Invalid command on line " << x + 1 << " ... Please fix the error for an outcome" << endl;
            exit(1);
        }
        opbranching(opcode, operand1, operand2);
    }
}

int main()
{
    OpenFile();
    SplitOperands();
    return 0;
}
// fix problem where fucntion opbranching exceeds 30 lines