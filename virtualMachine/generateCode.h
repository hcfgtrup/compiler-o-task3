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
    
    // ИЗМЕНЕНО: fillGaps теперь заполняет цепочку заглушек переходов
    void fillGaps(int lastGap);
    
    void genDup();
    void genLoad();
    void genSTOP();
    
    // ИЗМЕНЕНО: genAddress теперь просто пишет адрес, не создавая заглушек
    void genAddress(Item& item);
    
    void genSave();

    // ДОБАВЛЕНО: методы для логических операций
    void genLogicalAnd();
    void genLogicalOr();
    void genLogicalNot();

    // ДОБАВЛЕНО: работа с цепочкой заглушек переходов
    int getLastGap() { return lastGapAddress; }
    void resetGaps() { lastGapAddress = 0; }

    int getCmdCounter();

    void runCode();
    void printCode();


private:
    int cmdCounter; 
    OVM ovm;
    // ДОБАВЛЕНО: адрес последней созданной заглушки перехода
    int lastGapAddress = 0;
};