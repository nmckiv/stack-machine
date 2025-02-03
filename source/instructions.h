#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

//Define enums and functions for each instruction here

// enum Instructions {
//     ADD, BRT, BRZ, CALL, DIV, DUP, END, EQ, GE, GT, 
//     JUMP, LABEL, LOAD, LT, MOD, MUL, POP, PRINT, PRINTS, PUSH,
//     READ, READS, RET, RETV, SAVE, STORE, SUB
// }

int add(std::string args);
int brt(std::string args);
int brz(std::string args);
int call(std::string args);
int div(std::string args);
int dup(std::string args);
int end(std::string args);
int eq(std::string args);
int ge(std::string args);
int gt(std::string args);
int jump(std::string args);
int load(std::string args);
int lt(std::string args);
int mod(std::string args);
int mul(std::string args);
int pop(std::string args);
int print(std::string args);
int prints(std::string args);
int push(std::string args);
int read(std::string args);
int reads(std::string args);
int ret(std::string args);
int retv(std::string args);
int save(std::string args);
int store(std::string args);
int sub(std::string args);


#endif