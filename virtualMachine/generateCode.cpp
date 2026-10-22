#include "generateCode.h"
#include "ovm.h"
#include "../table/Item.h"

#include <limits>

GenerateCode::GenerateCode() : ovm() {
    cmdCounter = 0;
    lastGapAddress = 0;
    lastOrCheckGap = 0;
    orBodyTarget = 0;
    orCheckTarget = 0;
} 

void GenerateCode::gen(int cmd)
{
    ovm.getMemory()[cmdCounter] = cmd;
    cmdCounter++;
}

void GenerateCode::genConst(int constValue)
{
    if (constValue >= 0)
    {
        gen(constValue);
    }
    else 
    {
        gen(-constValue);
        gen(ovm.NEG);
    }
}

void GenerateCode::genVar(Item& item)
{
    gen(std::stoi(item.addr));
    gen(ovm.LOAD);
}

void GenerateCode::genAddress(Item& item)
{
    gen(std::stoi(item.addr));
}

void GenerateCode::genFunc(std::string func)
{
    if (func == "ABS")
    {                       // x
        gen(ovm.DUP);       // x, x
        gen(0);             // x, x, 0
        gen(cmdCounter+3);  // x, x, 0, Addr
        gen(ovm.IFGE);      // x
        gen(ovm.NEG);       // -x
    }
    else if (func == "MAX")
    {
        gen(std::numeric_limits<int>::max());
    }
    else if (func == "MIN")
    {
        gen(std::numeric_limits<int>::max());
        gen(ovm.NEG);
        gen(1);
        gen(ovm.SUB);
    }
    else if (func == "ODD")
    {                   // x
        gen(2);         // x, 2
        gen(ovm.MOD);   // x MOD 2
        gen(0);         // x MOD 2, 0
        gen(0);         // x MOD 2, 0 , Addr
        gen(ovm.IFEQ);
    }
}

void GenerateCode::genOperation(std::string operation)
{
    if (operation == "DIV") gen(ovm.DIV);
    else if (operation == "MULT") gen(ovm.MULT);
    else if (operation == "MOD") gen(ovm.MOD);
}

void GenerateCode::genNegative() { gen(ovm.NEG); }
void GenerateCode::genAddition() { gen(ovm.ADD); }
void GenerateCode::genSubstraction() { gen(ovm.SUB); }
void GenerateCode::genSave() { gen(ovm.SAVE); }

void GenerateCode::genHalt(int exitCode)
{
    genConst(exitCode);
    gen(ovm.STOP);
}

void GenerateCode::genDup() { gen(ovm.DUP); }
void GenerateCode::genLoad() { gen(ovm.LOAD); }

void GenerateCode::genInInt()
{
    gen(ovm.IN);
    gen(ovm.SAVE);
}

void GenerateCode::genOutInt() { gen(ovm.OUT); }
void GenerateCode::genOutLn() { gen(ovm.LN); }

// ИЗМЕНЕНО: заглушка хранит адрес предыдущей (цепочка выхода)
void GenerateCode::genComparison(std::string operation)
{
    int gapAddress = cmdCounter;
    gen(lastGapAddress);
    lastGapAddress = gapAddress;

    if (operation == "=") gen(ovm.IFNE);
    else if (operation == "#") gen(ovm.IFEQ);
    else if (operation == "<") gen(ovm.IFGE);
    else if (operation == "<=") gen(ovm.IFGT);
    else if (operation == ">") gen(ovm.IFLE);
    else if (operation == ">=") gen(ovm.IFLT);
}

void GenerateCode::genGoTo(int code)
{
    gen(code);
    gen(ovm.GOTO);
}

// Заполняет цепочку выхода
void GenerateCode::fillGaps(int lastGap)
{
    while (lastGap > 0)
    {
        int prev = ovm.getMemory()[lastGap];
        ovm.getMemory()[lastGap] = cmdCounter;
        lastGap = prev;
    }
    lastGapAddress = 0;
}

// ДОБАВЛЕНО: заполняет цепочки OR (body и check)
void GenerateCode::fillOrGaps()
{
    for (int gap : orTrueGaps)
    {
        ovm.getMemory()[gap] = orBodyTarget;
    }
    orTrueGaps.clear();

    for (int gap : orCheckGaps)
    {
        ovm.getMemory()[gap] = orCheckTarget;
    }
    orCheckGaps.clear();
}

void GenerateCode::genSTOP() { gen(ovm.STOP); }

int GenerateCode::getCmdCounter() { return cmdCounter; }

void GenerateCode::runCode() { ovm.run(); }

void GenerateCode::printCode() { ovm.printCode(cmdCounter); }

// AND — ничего не генерируем
void GenerateCode::genLogicalAnd()
{
    // AND реализован через последовательные переходы.
}

// ДОБАВЛЕНО: правильная реализация OR
void GenerateCode::genLogicalOr()
{
    if (lastGapAddress > 0)
    {
        int prev = ovm.getMemory()[lastGapAddress];
        orCheckGaps.push_back(lastGapAddress);
        lastGapAddress = prev;
    }
    
    int gapAddress = cmdCounter;
    gen(0);
    gen(ovm.GOTO);
    orTrueGaps.push_back(gapAddress);
    
    orCheckTarget = cmdCounter;
}

// NOT — инверсия последней команды перехода
void GenerateCode::genLogicalNot()
{
    if (cmdCounter <= 0) return;

    int& lastCmd = ovm.getMemory()[cmdCounter - 1];

    if (lastCmd == ovm.IFNE)      lastCmd = ovm.IFEQ;
    else if (lastCmd == ovm.IFEQ) lastCmd = ovm.IFNE;
    else if (lastCmd == ovm.IFGE) lastCmd = ovm.IFLT;
    else if (lastCmd == ovm.IFLT) lastCmd = ovm.IFGE;
    else if (lastCmd == ovm.IFGT) lastCmd = ovm.IFLE;
    else if (lastCmd == ovm.IFLE) lastCmd = ovm.IFGT;
}