#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unordered_map>

#include "machine.h"
#include "instructions.h"

int pc;
int dataStackPointer;
int callStackPointer;
int stackPointer;
int frameStackPointer;
int gpreg;
int top;
int instructionCount;
std::string currInst;
std::unordered_map<std::string, int> labels;
std::string errorString;
std::string outputString;
std::string interface;

std::string programMemory[PROGRAM_MEMORY_SIZE];
int dataStack[DATA_STACK_SIZE];
int callStack[CALL_STACK_SIZE];
int frameStack[FRAME_STACK_SIZE];

void reset() {
    // std::cout << "Resetting stack machine" << std::endl;
    pc = dataStackPointer = callStackPointer = stackPointer = frameStackPointer = gpreg = top = 0;
    for (int x = 0; x < PROGRAM_MEMORY_SIZE; x++) {
        programMemory[x] = "";
    }
}

// enum Instructions {
//     ADD, BRT, BRZ, CALL, DIV, DUP, END, EQ, GE, GT, 
//     JUMP, LABEL, LOAD, LT, MOD, MUL, POP, PRINT, PRINTS, PUSH,
//     READ, READS, RET, RETV, SAVE, STORE, SUB
// }

int processLabels() {

    //Extract opcode and args
    std::string opcode = "";
    std::string args = "";
    int status = 0;
    for (int x = 0; x < instructionCount; x++) {
        std::istringstream iss(programMemory[x]);
        iss >> opcode;
        if (!std::getline(iss, args)) {
            args = "";
        }

        std::all_of(args.begin(), args.end(), [](unsigned char c) {return std::isspace(c);});

        //Check if opcode is valid
        //If not treat it as a label
        if (opcode != "add" && opcode != "brt" && opcode != "brz" && opcode != "call" && opcode != "div" && opcode != "dup"
            && opcode != "end" && opcode != "eq" && opcode != "ge" && opcode != "gt" && opcode != "jump"
            && opcode != "load" && opcode != "lt" && opcode != "mod" && opcode != "mul" && opcode != "pop"
            && opcode != "print" && opcode != "prints" && opcode != "push" && opcode != "read" && opcode != "reads"
            && opcode != "ret" && opcode != "retv" && opcode != "save" && opcode != "store" && opcode != "sub") {
                if (std::all_of(args.begin(), args.end(), [](unsigned char c) {return std::isspace(c);})) {
                    //Process as a label
                    std::transform(opcode.begin(), opcode.end(), opcode.begin(), [](unsigned char c) {return std::tolower(c);});                    
                    if (opcode == "pc" || opcode == "top" || opcode == "sp") {
                        errorString += "Invalid label '" + opcode + "' at instruction " + std::to_string(x) + " of input file, cannot match reserved labels PC, TOP, or SP\n";
                        status++;
                    }
                    else if (labels.find(opcode) != labels.end()) {
                        errorString += "Duplicate label '" + opcode + "' at instructions " + std::to_string(labels.find(opcode)->second) + " and " + std::to_string(x) + "\n";
                        status++;
                    }
                    else {
                        labels[opcode] = x;
                    }
                }
            }
    }
    return status;
}

int load_file(std::string path) {

    int status = 0;

    // Open file
    std::ifstream file(path);
    if (!file.is_open()) {
        errorString += ("File '" + path + "' not found\n");
        status++;
    }

    else {
        std::string line;
        instructionCount = 0;
        while (getline(file, line)) {
            if (!std::all_of(line.begin(), line.end(), ::isspace)) {
                programMemory[instructionCount++] = line;
            }
        }
        status += processLabels();
    }

    return status;
}

//Perform fetch, increment, execute sequence
int executionStep() {


    if (pc >= instructionCount) {
        errorString += "Program counter has exceeded instructions.  Please terminate with END.\n";
        return 1;
    }

    //Fetch instruction
    currInst = programMemory[pc];

    //Increment program counter
    pc++;

    //Execute instruction and return status
    // std::transform(currInst.begin(), currInst.end(), currInst.begin(), [](unsigned char c) {return std::tolower(c);});
    return executeInstruction(currInst);
}

int executeInstruction(std::string instruction) {

    std::string opcode;
    std::string args;

    std::istringstream iss(instruction);
    iss >> opcode;
    std::getline(iss, args);

    //Remove leading and trailing whitespace
    args.erase(0, args.find_first_not_of(" \t"));
    args.erase(args.find_last_not_of(" \t\r\n") + 1);

    //Switch on the opcode
    // std::transform(opcode.begin(), opcode.end(), opcode.begin(), [](unsigned char c) {return std::tolower(c);});

    int status = 0;

    if      (opcode == "add")       status += add(args);
    else if (opcode == "brt")       status += brt(args);
    else if (opcode == "brz")       status += brz(args);
    else if (opcode == "call")      status += call(args);
    else if (opcode == "div")       status += div(args);
    else if (opcode == "dup")       status += dup(args);
    else if (opcode == "end")       status += end(args);
    else if (opcode == "eq")        status += eq(args);
    else if (opcode == "ge")        status += ge(args);
    else if (opcode == "gt")        status += gt(args);
    else if (opcode == "jump")      status += jump(args);
    else if (opcode == "load")      status += load(args);
    else if (opcode == "lt")        status += lt(args);
    else if (opcode == "mod")       status += mod(args);
    else if (opcode == "mul")       status += mul(args);
    else if (opcode == "pop")       status += pop(args);
    else if (opcode == "print")     status += print(args);
    else if (opcode == "prints")    status += prints(args);
    else if (opcode == "push")      status += push(args);
    else if (opcode == "read")      status += read(args);
    else if (opcode == "reads")     status += reads(args);
    else if (opcode == "ret")       status += ret(args);
    else if (opcode == "retv")      status += retv(args);
    else if (opcode == "save")      status += save(args);
    else if (opcode == "store")     status += store(args);
    else if (opcode == "sub")       status += sub(args);

    return status;
}