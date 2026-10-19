#include "generateCode.h"
#include "ovm.h"
#include "../table/Item.h"

#include <limits>

GenerateCode::GenerateCode() : ovm() {
    cmdCounter = 0;
    lastGapAddress = 0;
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
    // ИЗМЕНЕНО: теперь просто загружаем переменную по её реальному адресу
    gen(std::stoi(item.addr));
    gen(ovm.LOAD);
}

// ИЗМЕНЕНО: сразу пишем реальный адрес переменной, без заглушек
void GenerateCode::genAddress(Item& item)
{
    gen(std::stoi(item.addr));
    // item.addr уже установлен в Parser::contextVarPrc или loccateVariables
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
    if (operation == "DIV")
    {
        gen(ovm.DIV);
    }
    else if (operation == "MULT")
    {
        gen(ovm.MULT);
    }
    else if (operation == "MOD")
    {
        gen(ovm.MOD);
    }
}

void GenerateCode::genNegative()
{
    gen(ovm.NEG);
}

void GenerateCode::genAddition()
{
    gen(ovm.ADD);
}

void GenerateCode::genSubstraction()
{
    gen(ovm.SUB);
}

void GenerateCode::genSave()
{
    gen(ovm.SAVE);
}

void GenerateCode::genHalt(int exitCode)
{
    genConst(exitCode);
    gen(ovm.STOP);
}

void GenerateCode::genDup()
{
    gen(ovm.DUP);
}

void GenerateCode::genLoad()
{
    gen(ovm.LOAD);
}

void GenerateCode::genInInt()
{
    gen(ovm.IN);
    gen(ovm.SAVE);
}

void GenerateCode::genOutInt()
{
    gen(ovm.OUT);
}

void GenerateCode::genOutLn()
{
    gen(ovm.LN);
}

// ИЗМЕНЕНО: заглушка хранит адрес предыдущей заглушки (цепочка)
void GenerateCode::genComparison(std::string operation)
{
    int gapAddress = cmdCounter;
    gen(lastGapAddress);            // в заглушку кладём предыдущую
    lastGapAddress = gapAddress;    // обновляем "последнюю"

    if (operation == "=")
    {
        gen(ovm.IFNE);
    }
    else if (operation == "#")
    {
        gen(ovm.IFEQ);
    }
    else if (operation == "<")
    {
        gen(ovm.IFGE);
    }
    else if (operation == "<=")
    {
        gen(ovm.IFGT);
    }
    else if (operation == ">")
    {
        gen(ovm.IFLE);
    }
    else if (operation == ">=")
    {
        gen(ovm.IFLT);
    }
}

void GenerateCode::genGoTo(int code)
{
    gen(code);
    gen(ovm.GOTO);
}

// ИЗМЕНЕНО: проходит по всей цепочке заглушек и заполняет их
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

void GenerateCode::genSTOP()
{
    gen(ovm.STOP);
}

int GenerateCode::getCmdCounter()
{
    return cmdCounter;
}

void GenerateCode::runCode()
{
    ovm.run();
}

void GenerateCode::printCode()
{
    ovm.printCode(cmdCounter);
}

// ДОБАВЛЕНО: AND работает автоматически
void GenerateCode::genLogicalAnd()
{
    // AND реализован через последовательные переходы.
    // Ничего генерировать не нужно.
}

// ДОБАВЛЕНО: OR через де Моргана
void GenerateCode::genLogicalOr()
{
    genLogicalNot();
}

// ДОБАВЛЕНО: NOT — инверсия последней команды перехода
void GenerateCode::genLogicalNot()
{
    if (cmdCounter <= 0)
    {
        return;
    }

    int& lastCmd = ovm.getMemory()[cmdCounter - 1];

    if (lastCmd == ovm.IFNE)      lastCmd = ovm.IFEQ;
    else if (lastCmd == ovm.IFEQ) lastCmd = ovm.IFNE;
    else if (lastCmd == ovm.IFGE) lastCmd = ovm.IFLT;
    else if (lastCmd == ovm.IFLT) lastCmd = ovm.IFGE;
    else if (lastCmd == ovm.IFGT) lastCmd = ovm.IFLE;
    else if (lastCmd == ovm.IFLE) lastCmd = ovm.IFGT;
}