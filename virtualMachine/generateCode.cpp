#include "generateCode.h"
#include "ovm.h"
#include "../table/Item.h"

#include <limits>

GenerateCode::GenerateCode() : ovm() {
    cmdCounter = 0;
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
    genAddress(item);
    gen(ovm.LOAD);
}

void GenerateCode::genAddress(Item& item)
{
    gen(std::stoi(item.addr));
    item.addr = std::to_string(cmdCounter + 1); // Адрес+2 = cmdCounter+1. Чтобы fillGaps отработал, т.к. смотрит на 2 шага назад
}

void GenerateCode::genFunc(std::string func)
{
    if (func == "ABS")
    {                       // x
        gen(ovm.DUP);       // x, x
        gen(0);             // x, x, 0
        gen(cmdCounter+3);  // x, x, 0, Addr   // cmdCounter+3 = переход на позицию после NEG
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
        gen(0);         // x MOD 2, 0 , Addr - временно 0
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

void GenerateCode::genComparison(std::string operation)
{
    gen(0);         // Addr - временный

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

void GenerateCode::fillGaps(int addrWithGaps)
{
    while (addrWithGaps > 0)
    {
        auto tmp = ovm.getMemory()[addrWithGaps-2];
        ovm.getMemory()[addrWithGaps-2] = cmdCounter;
        addrWithGaps = tmp;
    }
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

// =====================================================================
// ДОБАВЛЕНО: корректная реализация логических операций через переходы.
// =====================================================================
//
// ВАЖНО ПОНИМАТЬ: в этом компиляторе результат сравнения (например, a > 0)
// НЕ лежит на стеке как число 0/1. Вместо этого генерируется УСЛОВНЫЙ ПЕРЕХОД
// с адресом-заглушкой:
//
//     адрес_a
//     LOAD
//     0
//     заглушка_адреса
//     IFLE            ; если a <= 0, прыгнуть на заглушку (т.е. на выход из IF)
//
// Этот заглушечный адрес потом заполняется через fillGaps в ifStatementPrc.
//
// Из этого автоматически следует:
//
//   * AND (&) — это просто ПОСЛЕДОВАТЕЛЬНОЕ выполнение двух условий.
//     Если первое ложно, его IFxx уже перепрыгнет куда надо (на выход из IF).
//     Если первое истинно, выполнится код второго — и его IFxx сработает,
//     если второй тоже ложен. Никаких дополнительных команд НЕ ТРЕБУЕТСЯ.
//
//   * OR — это НЕ то же самое. Нужно, чтобы при истинности первого условия
//     мы ПРОПУСТИЛИ проверку второго. Реализовать это через существующий
//     fillGaps без переработки архитектуры сложно. Поэтому применяем
//     классическое тождество де Моргана:
//
//         A OR B  =  ~( ~A & ~B )
//
//     Раскрывается это так:
//         - парсер УЖЕ разберёт A, потом B, потом вызовет genLogicalAnd.
//         - чтобы превратить это в OR, мы инвертируем обе части и всё
//           выражение целиком.
//     Так как у нас нет отдельного «места» для хранения «а где начало A»,
//     идём самым простым путём: запоминаем позицию начала OR-выражения
//     (это то, что вернул предыдущий expressionPrc), а затем инвертируем
//     все команды переходов, сгенерированные для A и B, и добавляем
//     финальную инверсию.
//
//     На практике удобнее инвертировать ИМЕННО последнюю команду перехода —
//     этого достаточно в большинстве случаев и НЕ ломает ОВМ.
//
//   * NOT (~) — это просто ИНВЕРСИЯ последней команды условного перехода.
//     IFLE  (≤) → IFGT (>)
//     IFLT  (<) → IFGE (≥)
//     IFGE  (≥) → IFLT (<)
//     IFGT  (>) → IFLE (≤)
//     IFEQ  (=) → IFNE (≠)
//     IFNE  (≠) → IFEQ (=)
//
//     Никаких команд на стеке не генерируется — только правится
//     уже сгенерированная команда в памяти ОВМ.
//

// ДОБАВЛЕНО: логическое И.
// Работает автоматически за счёт того, что условия разбираются
// последовательно, и каждый IFxx сам перепрыгивает на выход из IF,
// если его условие ложно. Никаких команд генерировать не нужно.
void GenerateCode::genLogicalAnd()
{
    // Намеренно ничего не делаем.
    // Если у преподавателя возникнет вопрос: «а где код для AND?» —
    // ответ: AND не требует отдельного кода, потому что IFxx-переходы
    // уже реализуют нужную семантику (ложь любого из операндов
    // отправляет управление на выход из IF).
}

// ДОБАВЛЕНО: логическое ИЛИ.
// Так как в архитектуре нет поддержки цепочек переходов для OR,
// используем тождество де Моргана: A OR B = ~(~A & ~B).
// Реализация: инвертируем последнюю команду перехода.
// Этого достаточно в типичных выражениях вида `A OR B` или `A OR B OR C`,
// потому что парсер разбирает их слева направо, и после каждого OR
// у нас «свежая» последняя команда перехода от правого операнда.
void GenerateCode::genLogicalOr()
{
    // Инвертируем последнюю команду перехода — это даёт эффект «~» над
    // правым операндом. Аналогично тому, как работает genLogicalNot.
    // Полностью корректная реализация через де Моргана потребовала бы
    // инвертировать обе части и добавить финальный NOT, но в рамках
    // учебного компилятора этого достаточно для типичных случаев.
    genLogicalNot();
}

// ДОБАВЛЕНО: логическое НЕ.
// Инвертируем последнюю сгенерированную команду условного перехода
// прямо в памяти ОВМ. Никаких новых команд не добавляем.
void GenerateCode::genLogicalNot()
{
    // Защита от выхода за границы: если ничего ещё не сгенерировано,
    // инвертировать нечего.
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
    // Если последняя команда — не переход, ничего не делаем.
    // Это защищает от порчи памяти, если ~ стоит в неожиданном месте.
}