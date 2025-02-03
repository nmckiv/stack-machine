#ifndef MACHINE_H
#define MACHINE_H

#define PROGRAM_MEMORY_SIZE 1024
#define DATA_STACK_SIZE 1024
#define CALL_STACK_SIZE 1024
#define FRAME_STACK_SIZE 1024
#define REGISTER_COUNT 6

#include <string>
#include <unordered_map>

extern int pc;
extern int dataStackPointer;
extern int callStackPointer;
extern int stackPointer;
extern int frameStackPointer;
extern int gpreg;
extern int top;
extern int instructionCount;
extern std::string currInst;
extern std::unordered_map<std::string, int> labels;
extern std::string errorString;
extern std::string outputString;
extern std::string interface;

extern std::string programMemory[PROGRAM_MEMORY_SIZE];
extern int dataStack[DATA_STACK_SIZE];
extern int callStack[CALL_STACK_SIZE];
extern int frameStack[FRAME_STACK_SIZE];

void reset();
int load_file(std::string path);
int executeInstruction(std::string instruction);
int executionStep();
int pushFrame(int frame);
int popFrame();

#endif