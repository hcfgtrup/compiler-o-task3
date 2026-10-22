#include "ovm.h"
#include "../table/Item.h"

#include <iostream>
#include <vector>

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
    void fillGaps(int lastGap);
    void fillOrGaps();
    void genDup();
    void genLoad();
    void genSTOP();
    void genAddress(Item& item);
    void genSave();

    // Логические операции
    void genLogicalAnd();
    void genLogicalOr();
    void genLogicalNot();

    // Управление цепочками заглушек
    int getLastGap() { return lastGapAddress; }
    void resetGaps() { 
        lastGapAddress = 0; 
        lastOrCheckGap = 0; 
        orCheckGaps.clear();
        orTrueGaps.clear();
    }
    
    // Целевые адреса для OR (заполняются в Parser)
    void setOrBodyTarget(int target) { orBodyTarget = target; }
    void setOrCheckTarget(int target) { orCheckTarget = target; }

    int getCmdCounter();

    void runCode();
    void printCode();

private:
    int cmdCounter; 
    OVM ovm;
    
    // Цепочка для выхода из IF (AND, NOT, обычные условия)
    int lastGapAddress = 0;
    
    // Цепочка для перехода от A к B (OR: если A ложно — проверить B)
    int lastOrCheckGap = 0;
    std::vector<int> orCheckGaps;
    
    // Цепочка для перехода в тело (OR: если A истинно — пропустить B)
    std::vector<int> orTrueGaps;
    
    // Целевые адреса (устанавливаются из Parser)
    int orBodyTarget = 0;   // адрес тела
    int orCheckTarget = 0;  // адрес начала B
};