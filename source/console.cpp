#include <iostream>

#include "console.h"
#include "machine.h"

int Console() {
    int status = 0;
    int curr;
    while ((curr = executionStep()) == 0);
    if (curr > 0) {status++;}
    return status;
}