#include "ovm.h"
#include "../table/Item.h"

#include <iostream>

#pragma once

class GenerateCode
{
public:
    GenerateCode();

    void gen(int cmd);
    void genConst(int constValue);
    void genVar(Item& item);
    void genFunc(std::string func);
    void genOperation(std::string operation);
    void genNegative();
    void genAddition();
    void genSubstraction();
    void genComparison(std::string operation);
    void genHalt(int exitCode);
    void genInInt();
    void genOutInt();
    void genOutLn();
    void genGoTo(int code);
    void fillGaps(int to);
    void genDup();
    void genLoad();
    void genSTOP();
    void genAddress(Item& item);
    void genSave();

    // ДОБАВЛЕНО: три метода для логических операций.
    // Реализация — через работу с цепочками переходов, а не через арифметику ОВМ,
    // потому что сравнения в этом компиляторе генерируют условные ПЕРЕХОДЫ
    // (IFNE, IFEQ, IFLT и т.д.) с адресами-заглушками, а не числа 0/1 на стеке.
    // Поэтому MULT/ADD над ними ломают память ОВМ (Assertion failed в vector).
    void genLogicalAnd();
    void genLogicalOr();
    void genLogicalNot();

    int getCmdCounter();

    void runCode();
    void printCode();


private:
    int cmdCounter; 
    OVM ovm;
};