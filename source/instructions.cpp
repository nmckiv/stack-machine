#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <regex>

#include "instructions.h"
#include "machine.h"
#include "tui.h"

std::vector<std::string> argsBuffer;

int split(std::string args) {
    argsBuffer.clear();
    std::istringstream iss(args);
    std::string word;
    while (iss >> word) {
        argsBuffer.push_back(word);
    }
    return argsBuffer.size();
}

int extract(std::string& str) {
    if (str.size() < 2 || str.front() != '"' || str.back() != '"') {
        return 1;
    }
    str = str.substr(1, str.size() - 2);

    std::string result;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '\\' && i + 1 < str.size()) {
            switch (str[i + 1]) {
                case 'n':
                    result += '\n';
                    ++i;
                    break;
                case 't':
                    result += '\t';
                    ++i;
                    break;
                // Add more cases here if needed
                default:
                    result += str[i];
                    break;
            }
        } else {
            result += str[i];
        }
    }
    str = result;
    return 0;
}

int pushFrame(int frame) {
    if (frameStackPointer > FRAME_STACK_SIZE - 1) {
        errorString += "Max stack frames of " + std::to_string(FRAME_STACK_SIZE) + " exceeded\n";
        return 1;
    }
    frameStack[frameStackPointer++] = stackPointer;
    return 0;
}
int popFrame() {
    if (frameStackPointer == 0) {
        errorString += "No stack frame to pop\n";
        return -1;
    }
    return frameStack[--frameStackPointer];
}


bool isInt(std::string str) {
    return std::regex_match(str, std::regex("^[0-9]+$"));
}

int add(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'add' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = top = dataStack[dataStackPointer - 2] + dataStack[dataStackPointer - 1];
            dataStackPointer--;
        }
    }
    else {
        errorString += "Invalid args for instruction 'add " + args + "'\n";
        status++;
    }
    return status;
}
int brt(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'brt' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            if (dataStack[dataStackPointer - 2] != 0) {
                pc = dataStack[dataStackPointer - 1];
            }
            dataStackPointer -= 2;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else if (size == 1) {
        if (dataStackPointer - stackPointer < 1) {
            errorString += "Instruction 'brt <register>' requires at least one item on the current stack frame\n";
            status++;
        }
        else {
            if (isInt(argsBuffer[0])) {
            if (dataStack[dataStackPointer - 1] != 0) {
                pc = stoi(argsBuffer[0]);
                dataStackPointer--;
                top = dataStack[dataStackPointer - 1];
            }
        }
        else {
            auto dest = labels.find(argsBuffer[0]);
                if (dest != labels.end()) {
                    if (dataStack[dataStackPointer - 1] != 0) {
                        pc = dest->second;
                    }
                    dataStackPointer--;
                    top = dataStack[dataStackPointer - 1];
                }
                else {
                    errorString += "No label '" + argsBuffer[0] + "' found\n";
                    status++;
                }
            }   
        }
    }
    else {
        errorString += "Invalid args for instruction 'brt " + args + "'\n";
        status++;
    }
    return status;
}
int brz(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'brt' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            if (dataStack[dataStackPointer - 2] == 0) {
                pc = dataStack[dataStackPointer - 1];
            }
            dataStackPointer -= 2;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else if (size == 1) {
        if (dataStackPointer - stackPointer < 1) {
            errorString += "Instruction 'brt <register>' requires at least one item on the current stack frame\n";
            status++;
        }
        else {
            if (isInt(argsBuffer[0])) {
            if (dataStack[dataStackPointer - 1] == 0) {
                pc = stoi(argsBuffer[0]);
                dataStackPointer--;
                top = dataStack[dataStackPointer - 1];
            }
        }
        else {
            auto dest = labels.find(argsBuffer[0]);
                if (dest != labels.end()) {
                    if (dataStack[dataStackPointer - 1] == 0) {
                        pc = dest->second;
                    }
                    dataStackPointer--;
                    top = dataStack[dataStackPointer - 1];
                }
                else {
                    errorString += "No label '" + argsBuffer[0] + "' found\n";
                    status++;
                }
            }   
        }
    }
    else {
        errorString += "Invalid args for instruction 'brt " + args + "'\n";
        status++;
    }
    return status;
}
int call(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        //Check if stack is big enough to contain arguments
        if ((dataStackPointer - 2 - dataStack[dataStackPointer - 2]) < stackPointer) {
            errorString += "Stack frame is not big enough to contain arguments\n";
            status++;
        }
        // if (dataStack[dataStackPointer - 2] < 0) {
        //     errorString += "Stack is not big enough to contain arguments\n";
        //     status++;
        // }
        else {
            // stackPointer += dataStack[dataStackPointer - 2];
            pushFrame(stackPointer);
            stackPointer = dataStackPointer - 2 - dataStack[dataStackPointer - 2];
            // lexStack[callStackPointer] = 1; // Assume first-order functions unless explicitly specified with calllex instruction
            callStack[callStackPointer++] = pc;
            pc = dataStack[dataStackPointer - 1];
            dataStackPointer -= 2;
            top = dataStack[dataStackPointer - 1];
            }
    }
    else if (size == 1) {
        auto dest = labels.find(argsBuffer[0]);
        if (dest != labels.end()) {
            if ((dataStackPointer - 1 - dataStack[dataStackPointer - 1]) < stackPointer) {
                errorString += "Stack frame is not big enough to contain arguments\n";
                status++;
            }
            // if (dataStack[dataStackPointer - 1] < 0) {
            //     errorString += "Stack is not big enough to contain arguments\n";
            //     status++;
            // }
            else {
                // stackPointer += dataStack[dataStackPointer - 1];
                pushFrame(stackPointer);
                stackPointer = dataStackPointer - 1 - dataStack[dataStackPointer - 1];
                // lexStack[callStackPointer] = 1; // Assume first-order functions unless explicitly specified with calllex instruction
                callStack[callStackPointer++] = pc;
                pc = labels.find(argsBuffer[0])->second;
                dataStackPointer--;
                top = dataStack[dataStackPointer - 1];
            }
        }
        else {
            errorString += "No label '" + argsBuffer[0] + "' found for instruction 'call " + args + "'\n";
            status++;
        }
    }
    else {
        errorString += "Invalid args for instruction 'call " + args + "'\n";
        status++;
    }
    return status;
}
int calllex(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 3) {
        auto dest = labels.find(argsBuffer[0]);
        if (dest != labels.end()) {
            stackPointer = dataStackPointer - stoi(argsBuffer[1]);
            lexStack[frameStackPointer] = stoi(argsBuffer[2]);
            pushFrame(stackPointer);
            callStack[callStackPointer++] = pc;
            pc = labels.find(argsBuffer[0])->second;
            // dataStackPointer--;
            top = dataStack[dataStackPointer - 1];
        }
        else {
            errorString += "No label '" + argsBuffer[0] + "' found for instruction 'calllex " + args + "'\n";
            status++;
        }
    }
    else {

        // auto dest = labels.find(argsBuffer[0]);
        // if (dest != labels.end()) {
        //     if ((dataStackPointer - 1 - dataStack[dataStackPointer - 1]) < stackPointer) {
        //         errorString += "Stack frame is not big enough to contain arguments\n";
        //         status++;
        //     }
        //     else {
        //         pushFrame(stackPointer);
        //         stackPointer = dataStackPointer - 1 - dataStack[dataStackPointer - 1];
        //         lexStack[callStackPointer] = stoi(argsBuffer[1]);
        //         callStack[callStackPointer++] = pc;
        //         pc = labels.find(argsBuffer[0])->second;
        //         dataStackPointer--;
        //         top = dataStack[dataStackPointer - 1];
        //     }
        // }
        // else {
        //     errorString += "No label '" + argsBuffer[0] + "' found for instruction 'call " + args + "'\n";
        //     status++;
        // }
        errorString += "Invalid arguments for instruction 'calllex " + args + "'\n";
        status++;
    }
    return status;
}
int div(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'div' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = dataStack[dataStackPointer - 2] / dataStack[dataStackPointer - 1];
            dataStackPointer--;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'div " + args + "'\n";
        status++;
    }
    return status;
}
int dup(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer == stackPointer) {
            errorString += "Stack frame is empty - nothing to duplicate\n";
            status++;
        }
        else {
            dataStack[dataStackPointer++] = gpreg = dataStack[dataStackPointer - 1];
            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'dup " + args + "'\n";
        status++;
    }
    return status;
}
int end(std::string args) {
    //Return code for end of instruction block
    return -1;
}
int eq(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'eq' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = (dataStack[dataStackPointer - 2] == dataStack[dataStackPointer - 1]) ? 1 : 0;
            dataStackPointer--;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'eq " + args + "'\n";
        status++;
    }
    return status;
}
int neq(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'eq' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = (dataStack[dataStackPointer - 2] != dataStack[dataStackPointer - 1]) ? 1 : 0;
            dataStackPointer--;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'eq " + args + "'\n";
        status++;
    }
    return status;
}
int ge(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'ge' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = (dataStack[dataStackPointer - 2] >= dataStack[dataStackPointer - 1]) ? 1 : 0;
            dataStackPointer--;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'ge " + args + "'\n";
        status++;
    }
    return status;
}
int gt(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'gt' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = (dataStack[dataStackPointer - 2] > dataStack[dataStackPointer - 1]) ? 1 : 0;
            dataStackPointer--;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'gt " + args + "'\n";
        status++;
    }
    return status;
}
int jump(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 1) {
            errorString += "Instruction 'jump' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            pc = dataStack[--dataStackPointer];
            top = dataStack[dataStackPointer - 1];
        }
    }
    else if (size == 1) {
        //Jump to argument
        //Check if argument is an integer
        if (isInt(argsBuffer[0])) {
            pc = stoi(argsBuffer[0]);
        }
        //Otherwise treat it like a label
        else {
            auto dest = labels.find(argsBuffer[0]);
            //Search for label
            if (dest != labels.end()) {
                pc = labels.find(argsBuffer[0])->second;
            }
            //Label not found error
            else {
                errorString += "No label '" + argsBuffer[0] + "' found\n";
                status++;
            }
        }
    }
    else {
        errorString += "Invalid args for instruction 'jump " + args + "'\n";
        status++;
    }

    return status;
}
int load(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        dataStack[dataStackPointer - 1] = dataStack[dataStack[dataStackPointer - 1]];
        gpreg = top = dataStack[dataStack[dataStackPointer - 1]];
    }
    else if (size == 1) {
        if (argsBuffer[0] == "pc") {
            dataStack[dataStackPointer - 1] = gpreg = top = dataStack[dataStack[dataStackPointer - 1] + pc];
        }
        else if (argsBuffer[0] == "sp") {
            dataStack[dataStackPointer - 1] = gpreg = top = dataStack[dataStack[dataStackPointer - 1] + stackPointer];
        }
        else if (argsBuffer[0] == "top") {
            dataStack[dataStackPointer - 1] = gpreg = top = dataStack[dataStack[dataStackPointer - 1] + top];
        }
        else {
            int frame_backtrack;
                frame_backtrack = std::stoi(argsBuffer[0]);
                dataStack[dataStackPointer - 1] = gpreg = top = dataStack[callStack[callStackPointer - frame_backtrack] + dataStack[dataStackPointer - 1]];
                //printf("callstack at %d - %d is %d\n", callStackPointer, frame_backtrack, callStack[callStackPointer - frame_backtrack]);
        }
    }
    else {
        errorString += "Invalid args for instruction 'load " + args + "'\n";
        status++;
    }
    return 0;
}
int loadlex(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        int addr = -1;
        for (int x = callStackPointer - 1; x >= 0; x--) {
            // printf("lex stack at %d is %d\n", x, lexStack[x]);
            if (lexStack[x] == dataStack[dataStackPointer - 2]) {
                // printf("Breaking out of loop at index %d\n", x);
                addr = x;
                break;
            }
        }
        if (addr == -1) {
            errorString += "No lex frame found with scope " + argsBuffer[0] + "\n";
            status++;
        }
        else {
            int offset = dataStack[dataStackPointer - 1];
            dataStack[dataStackPointer - 2] = gpreg = top = dataStack[frameStack[addr] + offset];
            dataStackPointer -= 1;
        }
    }
    else if (size == 2) {
        // Get address of last call frame with specified scope
        int addr = -1;

        // Lexstack dump for debugging
        // for (int x = callStackPointer - 1; x >= 0; x--) {
        //     printf("%d?|?\n", lexStack[x]);
        // }

        // printf("Entering stackdump\n");
        for (int x = callStackPointer - 1; x >= 0; x--) {
            // printf("lex stack at %d is %d\n", x, lexStack[x]);
            if (lexStack[x] == stoi(argsBuffer[0])) {
                // printf("Breaking out of loop at index %d\n", x);
                addr = x;
                break;
            } 
        }
        // printf("Exiting stackdump\n");

        if (addr == -1) {
            errorString += "No lex frame found with scope " + argsBuffer[0] + "\n";
            status++;
        }
        else {
            int offset = stoi(argsBuffer[1]);
            dataStack[dataStackPointer++] = gpreg = top = dataStack[frameStack[addr] + offset];
        }
    }
    return 0;
}
int lt(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'lt' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = (dataStack[dataStackPointer - 2] < dataStack[dataStackPointer - 1]) ? 1 : 0;
            dataStackPointer--;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'lt " + args + "'\n";
        status++;
    }
    return status;
}
int le(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'le' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = (dataStack[dataStackPointer - 2] <= dataStack[dataStackPointer - 1]) ? 1 : 0;
            dataStackPointer--;
            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'le " + args + "'\n";
        status++;
    }
    return status;
}
int mod(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'mod' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = top = dataStack[dataStackPointer - 2] % dataStack[dataStackPointer - 1];
            dataStackPointer--;
        }
    }
    else {
        errorString += "Invalid args for instruction 'mod " + args + "'\n";
        status++;
    }
    return status;
}
int mul(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'mul' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = top = dataStack[dataStackPointer - 2] * dataStack[dataStackPointer - 1];
            dataStackPointer--;
        }
    }
    else {
        errorString += "Invalid args for instruction 'mul " + args + "'\n";
        status++;
    }
    return status;
}
int pop(std::string args) {
    int status = 0;
    int size = split(args);

    if (size == 0) {
        if (dataStackPointer - stackPointer > 0) {
            gpreg = dataStack[--dataStackPointer];
            top = dataStack[dataStackPointer - 1];
        }
        else {
            errorString += "Invalid pop instruction: stack frame is empty\n";
            status++;
        }
    }
    else {
        errorString += "Invalid args for instruction 'pop " + args + "'\n";
        status++;
    }

    return status;
}
int print(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer <= stackPointer) {
            errorString += "Stack frame must have at least one item to print\n";
            status++;
        }
        else {
            gpreg = top;
            if (interface == "TUI") {
                outputString += std::to_string(top);
            }
            else if (interface == "Console") {
                std::cout << top << std::flush;
            }
            top = dataStack[--dataStackPointer - 1];
        }
    }
    else {
        if (interface == "TUI") {
            errorString += "Invalid argument for print instruction\n";
            status++;
        }
        else if (interface == "Console") {
            std::cout << errorString << std::endl;
        }
        status++;
    }
    return status;
}
int prints(std::string args) {
    int status = 0;
    if (extract(args) == 0) {
        if (interface == "TUI") {
            outputString += args;
        }
        else if (interface == "Console") {
            std::cout << args << std::flush;
        }
    }
    else {
        if (interface == "TUI") {
            errorString += "Invalid argument for prints: must be nonempty and enclosed with \"\"\n";
        }
        else if (interface == "Console") {
            std::cout << errorString << std::endl;
        }
        status++;
    }
    return status;
}
int push(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        dataStack[dataStackPointer++] = top = gpreg;
    }
    else if (size == 1) {
        if (isInt(argsBuffer[0])) {
            dataStack[dataStackPointer++] = gpreg = top = stoi(argsBuffer[0]);
        }
        else {
            //Check specialized labels
            if (argsBuffer[0] == "pc") {
                dataStack[dataStackPointer++] = gpreg = top = pc;
            }
            else if (argsBuffer[0] == "sp") {
                dataStack[dataStackPointer++] = gpreg = top = pc;
            }
            else if (argsBuffer[0] == "top") {
                dataStack[dataStackPointer++] = gpreg = top;
            }
            else {
                auto dest = labels.find(argsBuffer[0]);
                //Search for label
                if (dest != labels.end()) {
                    dataStack[dataStackPointer++] = gpreg = top = labels.find(argsBuffer[0])->second;
                }
                //Label not found error
                else {
                    errorString += "No label '" + argsBuffer[0] + "' found\n";
                    status++;
                }
            }
        }
    }
    else {
        errorString += "Invalid args for instruction 'push " + args + "'\n";
        status++;
    }

    return status;
}
int read(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (interface == "TUI") {
            if (input) {
                input_string.pop_back();//Remove trailing newline from clicking enter
                if (isInt(input_string)) {
                    dataStack[dataStackPointer++] = gpreg = top = stoi(input_string);
                    outputString += input_string + '\n';
                    input = false;
                }
                else {
                    if (input_string != "") {
                        outputString += "\nNot an integer, try again:\t";
                        input_string.clear();
                    }
                    input = true;
                    pc--;
                }
            }
            else {
                input_string.clear();
                input = true;
                pc--;
            }
        }
        else if (interface == "Console") {
            std::cout << std::flush;
            std::string in;
            std::cin >> in;
            while (!isInt(in)) {
                if (in != "") {
                    std::cout << "Not an integer, try again: " << std::flush;
                }
                std::cin >> in;
            }
            dataStack[dataStackPointer++] = gpreg = top = stoi(in);
        }
    }
    else {
        errorString += "Invalid args for instruction 'read " + args + "'\n";
        status++;
    }
    return status;
}
int reads(std::string args) {
    // std::cout << "reads" << std::endl;
    return 0;
}
int ret(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        //Pop previous frame from frame stack
        int prev = popFrame();
        if (prev == -1) {
            errorString += "No stack frame to return to in instructon 'ret'\n";
            status++;
        }
        else {
            //Get return value
            int retval = dataStack[dataStackPointer - 1];
            //Clear stack entries between top and current stack pointer
            dataStackPointer = stackPointer;
            //Set stack pointer to previous stack frame value
            stackPointer = prev;
            //Pop return address and set program counter
            pc = callStack[--callStackPointer];

            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'read " + args + "'\n";
        status++;
    }
    return status;
}
int retlex(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        //Pop previous frame from frame stack
        int prev = popFrame();
        if (prev == -1) {
            errorString += "No stack frame to return to in instructon 'ret'\n";
            status++;
        }
        else {
            prev = frameStack[frameStackPointer - 1];
            frameStackPointer - 1;
            //Get return value
            int retval = dataStack[dataStackPointer - 1];
            //Clear stack entries between top and current stack pointer
            dataStackPointer = stackPointer;
            //Set stack pointer to previous stack frame value
            stackPointer = prev;
            //Pop return address and set program counter
            pc = callStack[--callStackPointer];

            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'read " + args + "'\n";
        status++;
    }
    return status;
}
int retv(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        //Pop previous frame from frame stack
        int prev = popFrame();
        if (prev == -1) {
            errorString += "No stack frame to return to in instructon 'ret'\n";
            status++;
        }
        else {
            //Get return value
            int retval = dataStack[dataStackPointer - 1];
            //Clear stack entries between top and current stack pointer
            dataStackPointer = stackPointer;
            //Set stack pointer to previous stack frame value
            stackPointer = prev;
            //Push return value
            dataStack[dataStackPointer++] = gpreg = top = retval;
            //Pop return address and set program counter
            pc = callStack[--callStackPointer];

            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'read " + args + "'\n";
        status++;
    }
    return status;
}
int retvlex(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        //Pop previous frame from frame stack
        int prev = popFrame();
        if (prev == -1) {
            errorString += "No stack frame to return to in instructon 'ret'\n";
            status++;
        }
        else {
            prev = frameStack[frameStackPointer - 1];
            frameStackPointer - 1;
            //Get return value
            int retval = dataStack[dataStackPointer - 1];
            //Clear stack entries between top and current stack pointer
            dataStackPointer = stackPointer;
            //Set stack pointer to previous stack frame value
            stackPointer = prev;
            //Push return value
            dataStack[dataStackPointer++] = gpreg = top = retval;
            //Pop return address and set program counter
            pc = callStack[--callStackPointer];

            top = dataStack[dataStackPointer - 1];
        }
    }
    else {
        errorString += "Invalid args for instruction 'read " + args + "'\n";
        status++;
    }
    return status;
}
int save(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer < 3) {
            errorString += "Stack must contain at least two items for instruction: save\n";
            status++;
        }
        else {
            if (dataStack[dataStackPointer - 1] >= 0 && dataStack[dataStackPointer - 1] < dataStackPointer) {
                dataStack[dataStack[dataStackPointer - 1]] = gpreg = dataStack[dataStackPointer - 2];
                dataStackPointer--;
                top = dataStack[dataStackPointer - 1];
            }
            else {
                errorString += "Invalid address " + std::to_string(dataStack[dataStackPointer - 1]) + " for save instruction\n";
                status++;
            }
        }
    }
    else if (size == 1) {
        if (argsBuffer[0] == "sp") {
            if ((dataStack[dataStackPointer - 1] + stackPointer) >= 0 && (dataStack[dataStackPointer - 1] + stackPointer) < dataStackPointer) {
                dataStack[dataStack[dataStackPointer - 1] + stackPointer] = gpreg = dataStack[dataStackPointer - 2];
                dataStackPointer--;
                top = dataStack[dataStackPointer - 1];
            }
            else {
                errorString += "Invalid address " + std::to_string(dataStack[dataStackPointer - 1]) + " for save instruction\n";
                status++;
            }
        }
        else {
            errorString += "Invalid register argument for instruction 'save " + args + "\n";
        }
    }
    else {
        errorString += "Invalid args for instruction 'save " + args + "'\n";
        status++;
    }
    return status;
}
int savelex(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer < 3) {
            errorString += "Stack must contain at least two items for instruction: save\n";
            status++;
        }
        else {
            if (dataStack[dataStackPointer - 1] >= 0 && dataStack[dataStackPointer - 1] < dataStackPointer) {
                dataStack[dataStack[dataStackPointer - 1]] = gpreg = dataStack[dataStackPointer - 2];
                dataStackPointer--;
                top = dataStack[dataStackPointer - 1];
            }
            else {
                errorString += "Invalid address " + std::to_string(dataStack[dataStackPointer - 1]) + " for save instruction\n";
                status++;
            }
        }
    }
    else if (size == 1) {
        if (argsBuffer[0] == "sp") {
            if ((dataStack[dataStackPointer - 1] + stackPointer) >= 0 && (dataStack[dataStackPointer - 1] + stackPointer) < dataStackPointer) {
                dataStack[dataStack[dataStackPointer - 1] + stackPointer] = gpreg = dataStack[dataStackPointer - 2];
                dataStackPointer--;
                top = dataStack[dataStackPointer - 1];
            }
            else {
                errorString += "Invalid address " + std::to_string(dataStack[dataStackPointer - 1]) + " for save instruction\n";
                status++;
            }
        }
        else {
            errorString += "Invalid register argument for instruction 'save " + args + "\n";
        }
    }
    else {
        errorString += "Invalid args for instruction 'save " + args + "'\n";
        status++;
    }
    return status;
}
int store(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer < 3) {
            errorString += "Stack must contain at least two items for instruction: store\n";
            status++;
        }
        else {
            if (dataStack[dataStackPointer - 1] >= 0 && dataStack[dataStackPointer - 1] < dataStackPointer) {
                dataStack[dataStack[dataStackPointer - 1]] = gpreg = dataStack[dataStackPointer - 2];
                dataStackPointer -= 2;
                top = dataStack[dataStackPointer - 1];
            }
            else {
                errorString += "Invalid address " + std::to_string(dataStack[dataStackPointer - 1]) + " for store instruction\n";
                status++;
            }
        }
    }
    else if (size == 1) {
        if (argsBuffer[0] == "sp") {
            if ((dataStack[dataStackPointer - 1] + stackPointer) >= 0 && (dataStack[dataStackPointer - 1] + stackPointer) < dataStackPointer) {
                dataStack[dataStack[dataStackPointer - 1] + stackPointer] = gpreg = dataStack[dataStackPointer - 2];
                dataStackPointer -= 2;
                top = dataStack[dataStackPointer - 1];
            }
            else {
                errorString += "Invalid address " + std::to_string(dataStack[dataStackPointer - 1]) + " for store instruction\n";
                status++;
            }
        }
        else {
            errorString += "Invalid register argument for instruction 'store " + args + "\n";
        }
    }
    else {
        errorString += "Invalid args for instruction 'store " + args + "'\n";
        status++;
    }
    return status;
}
int storelex(std::string args) {
    std::transform(args.begin(), args.end(), args.begin(), [](unsigned char c) {return std::tolower(c);});
    int status = 0;
    int size = split(args);
    if (size == 0) {
        int addr = -1;
        for (int x = callStackPointer - 1; x >= 0; x--) {
            if (lexStack[x] == dataStack[dataStackPointer - 3]) {
                addr = x;
                break;
            }
        }
        if (addr == -1) {
            errorString += "No lex frame found with scope " + argsBuffer[0] + "\n";
            status++;
        }
        else {
            int offset = dataStack[dataStackPointer - 2];
            dataStack[frameStack[addr] + offset] = dataStack[dataStackPointer - 1];
            dataStackPointer -= 3;
        }
    }
    else if (size == 2) {


        // Get address of last call frame with specified scope
        int addr = -1;
        for (int x = callStackPointer - 1; x >= 0; x--) {
            if (lexStack[x] == stoi(argsBuffer[0])) {
                addr = x;
                break;
            } 
        }
        if (addr == -1) {
            errorString += "No lex frame found with scope " + argsBuffer[0] + "\n";
            status++;
        }
        else {
            int offset = stoi(argsBuffer[1]);
            dataStack[frameStack[addr] + offset] = dataStack[dataStackPointer - 1];
            dataStackPointer--;
        }
    }
    else {
        errorString += "Invalid args for instruction 'storelex " + args + "'\n";
        status++;
    }
    return status;
}
int sub(std::string args) {
    int status = 0;
    int size = split(args);
    if (size == 0) {
        if (dataStackPointer - stackPointer < 2) {
            errorString += "Instruction 'sub' requires at least two items on the current stack frame\n";
            status++;
        }
        else {
            dataStack[dataStackPointer - 2] = gpreg = top = dataStack[dataStackPointer - 2] - dataStack[dataStackPointer - 1];
            dataStackPointer--;
        }
    }
    else {
        errorString += "Invalid args for instruction 'sub " + args + "'\n";
        status++;
    }
    return status;
}