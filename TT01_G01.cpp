// ===== runner.cpp ===== 
#include "VirtualMachine.h" 
#include <iostream> 
using namespace std;

int main() { 
    VirtualMachine vm; 
    vm.runFromFile("program2.asm"); 
    vm.dump();
    cout << "Program executed and has been saved to 'output.txt' and 'execute.txt'" << endl;
    return 0; 
}