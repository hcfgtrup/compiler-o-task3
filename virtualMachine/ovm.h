#include <iostream>
#include <vector>
#include <unordered_map>

#pragma once

class OVM
{
public:
    OVM();

    void run();
    std::vector<int>& getMemory();

    void test_input();
    void printCode(int cmdCounter);

    enum operation {
        STOP = -1,
        ADD = -2,
        SUB = -3,
        MULT = -4,
        DIV = -5,
        MOD = -6,
        NEG = -7,
        LOAD = -8,
        SAVE = -9,
        DUP = -10,
        DROP = -11,
        SWAP = -12,
        OVER = -13,
        GOTO = -14,
        IFLT = -15,
        IFLE = -16,
        IFGT = -17,
        IFGE = -18,
        IFEQ = -19,
        IFNE = -20,
        IN = -21,
        OUT = -22,
        LN = -23
    };
private:
    int pc;
    int sp;
    std::vector<int> memory;
    static constexpr size_t MEMORY_SIZE = 8*1024;
    int tickCounter;

    const std::unordered_map<int, std::string> operationToString = {
        {STOP, "STOP"},
        {ADD, "ADD"},
        {SUB, "SUB"},
        {MULT, "MULT"},
        {DIV, "DIV"},
        {MOD, "MOD"},
        {NEG, "NEG"},
        {LOAD, "LOAD"},
        {SAVE, "SAVE"},
        {DUP, "DUP"},
        {DROP, "DROP"},
        {SWAP, "SWAP"},
        {OVER, "OVER"},
        {GOTO, "GOTO"},
        {IFLT, "IFLT"},
        {IFLE, "IFLE"},
        {IFGT, "IFGT"},
        {IFGE, "IFGE"},
        {IFEQ, "IFEQ"},
        {IFNE, "IFNE"},
        {IN, "IN"},
        {OUT, "OUT"},
        {LN, "LN"}
    };

    std::string operation_to_string(int op);
};