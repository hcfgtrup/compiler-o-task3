#include "Parser.h"
#include "../sourceCodeDriver/Driver.h"
#include "../scanner/Scanner.h"
#include "../error/Error.h"
#include "../table/TableOfName.h"
#include "../table/Item.h"
#include "../virtualMachine/generateCode.h"

#include <string>
#include <algorithm>

Parser::Parser(Scanner &scanner, std::shared_ptr<Error> errorPtr) : 
    scanner(scanner), table(), errorPtr(std::move(errorPtr)), generateCode() {
    scanner.nextLex();
} 

void Parser::compile() 
{
    table.openScope();
    table.addItem(table.functionItem("ABS", Item::ItemTypes::Integer));
    table.addItem(table.functionItem("MIN", Item::ItemTypes::Integer));
    table.addItem(table.functionItem("MAX", Item::ItemTypes::Integer));
    table.addItem(table.functionItem("ODD", Item::ItemTypes::Boolean));

    table.addItem(table.procedureItem("HALT"));
    table.addItem(table.procedureItem("INC"));
    table.addItem(table.procedureItem("DEC"));
    
    table.addItem(table.moduleItem("In"));
    table.addItem(table.moduleItem("Out"));
    table.addItem(table.procedureItem("In.Open"));
    table.addItem(table.procedureItem("In.Int"));
    table.addItem(table.procedureItem("Out.Int"));
    table.addItem(table.procedureItem("Out.Ln"));

    table.addItem(table.typeItem("INTEGER", Item::ItemTypes::Integer));
    
    table.openScope();
    modulePrc();
    table.closeScope();
    table.closeScope();

    generateCode.printCode();
    generateCode.runCode();
}

void Parser::modulePrc() 
{
    checkLex(Scanner::Lex::MODULE);
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    std::string moduleName = scanner.nameValue;
    table.newItem(table.moduleItem(moduleName));
    scanner.nextLex();
    checkLex(Scanner::Lex::SEMI);

    if (scanner.lex == Scanner::Lex::IMPORT) importPrc();

    sequenceDeclarationsPrc();

    if (scanner.lex == Scanner::Lex::BEGIN) 
    {
        scanner.nextLex();
        sequenceStatementsPrc();
    }

    checkLex(Scanner::Lex::END);
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    Item* item = table.findItem(scanner.nameValue);
    if (item->typeOfItem != "module") errorPtr->syntaxError("имя модуля");
    else if (item->name != moduleName) errorPtr->syntaxError("имя модуля " + scanner.nameValue);

    scanner.nextLex();
    checkLex(Scanner::Lex::DOT);
    generateCode.genSTOP();
    loccateVariables();
}

void Parser::loccateVariables()
{
    generateCode.genSTOP();
}

void Parser::importPrc()
{
    checkLex(Scanner::Lex::IMPORT);
    contextImportPrc();
    while (scanner.lex == Scanner::Lex::COMMA) {
        checkLex(Scanner::Lex::COMMA);
        contextImportPrc();
    }
    checkLex(Scanner::Lex::SEMI);
}

void Parser::sequenceDeclarationsPrc() 
{
    while (scanner.lex == Scanner::Lex::CONST || scanner.lex == Scanner::Lex::VAR)
    {
        if (scanner.lex == Scanner::Lex::CONST) 
        {
            checkLex(Scanner::Lex::CONST);
            while (scanner.lex == Scanner::Lex::NAME) 
            {
                constDeclarationPrc();
                checkLex(Scanner::Lex::SEMI);
            }
        }
        else 
        {
            checkLex(Scanner::Lex::VAR);
            while (scanner.lex == Scanner::Lex::NAME) 
            {
                varDeclarationPrc();
                checkLex(Scanner::Lex::SEMI);
            }
        }
    }
}

void Parser::constDeclarationPrc()
{
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    std::string constName = scanner.nameValue;
    scanner.nextLex();

    checkLex(Scanner::Lex::EQ);
    int constValue = constExpresionPrc();

    table.newItem(table.constItem(constName, Item::ItemTypes::Integer, std::to_string(constValue)));
}

int Parser::constExpresionPrc() 
{
    int sign = 1;
    
    if (scanner.lex == Scanner::Lex::MINUS) { checkLex(Scanner::Lex::MINUS); sign = -1; }
    else if (scanner.lex == Scanner::Lex::PLUS) checkLex(Scanner::Lex::PLUS);

    if (scanner.lex == Scanner::Lex::NAME)
    {
        Item* item = table.findItem(scanner.nameValue);
        checkLex(Scanner::Lex::NAME);
        if (item->typeOfItem != "const") errorPtr->contextError("имя модуля");
        else return std::stoi(item->value) * sign;
    }
    else if (scanner.lex == Scanner::Lex::NUM)
    {
        int constValue = scanner.numValue * sign;
        checkLex(Scanner::Lex::NUM);
        return constValue;
    }
    else errorPtr->syntaxError("имя константы или число");

    return 0;
}

void Parser::varDeclarationPrc() 
{
    contextVarPrc();
    while (scanner.lex == Scanner::Lex::COMMA)
    {
        checkLex(Scanner::Lex::COMMA);
        contextVarPrc();
    }

    checkLex(Scanner::Lex::COLON);
    typePrc();
}

void Parser::contextVarPrc()
{
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    
    static int nextVarAddr = 1000;
    std::string addr = std::to_string(nextVarAddr++);
    
    table.newItem(table.varItem(scanner.nameValue, Item::ItemTypes::Integer, addr));
    scanner.nextLex();
}

void Parser::typePrc() 
{
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    Item* item = table.findItem(scanner.nameValue);

    if (item->typeOfItem != "type") errorPtr->contextError("Необьявленное имя типа");

    scanner.nextLex();
}

void Parser::sequenceStatementsPrc() 
{
    statementsPrc();
    while (scanner.lex == Scanner::Lex::SEMI)
    {
        scanner.nextLex();
        statementsPrc();
    }
}

void Parser::statementsPrc()
{
    if (scanner.lex == Scanner::Lex::NAME) variableOrCallPrc();
    else if (scanner.lex == Scanner::Lex::IF) ifStatementPrc();
    else if (scanner.lex == Scanner::Lex::WHILE) whileStatementPrc();
}

void Parser::variableOrCallPrc()
{
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    Item* item = table.findItem(scanner.nameValue);
    scanner.nextLex();

    if (item->typeOfItem == "var")
    {
        generateCode.genAddress(*item);
        checkLex(Scanner::Lex::ASS);
        Item::ItemTypes expressionType = expressionPrc();
        if (item->type != expressionType) errorPtr->contextError("Неверный тип при присваивании");
        generateCode.genSave();
    }
    else if (item->typeOfItem == "procedure" || item->typeOfItem == "module")
    {
        if (scanner.lex == Scanner::Lex::DOT)
        {
            if (item->typeOfItem != "module") errorPtr->contextError("Ожидается имя модуля");
            scanner.nextLex();
            errorIfNotExpectedLex(Scanner::Lex::NAME);
            std::string procedureName = item->name + "." + scanner.nameValue;
            item = table.findItem(procedureName);
            if (item->typeOfItem != "procedure") errorPtr->contextError("Ожидается процедура");
            scanner.nextLex();
        }
        else if (item->typeOfItem != "procedure") errorPtr->contextError("Ожидается имя процедуры");

        if (scanner.lex == Scanner::Lex::LPAR)
        {
            scanner.nextLex();
            checkProcParameters(*item);
            checkLex(Scanner::Lex::RPAR);  
        }
        else if (item->name == "Out.Ln") generateCode.genOutLn();
        else if (item->name != "Out.Ln" && item->name != "In.Open") 
            errorPtr->contextError("Ожидается скобка");
    }
    else errorPtr->contextError("Ожидается имя перменной или процедуры");
}

// ИЗМЕНЕНО: правильная обработка OR
void Parser::ifStatementPrc()
{
    checkLex(Scanner::Lex::IF);
    
    generateCode.resetGaps();
    
    Item::ItemTypes expressionType = logicalExpressionPrc();
    checkBoolType(expressionType);
    checkLex(Scanner::Lex::THEN);

    // Запоминаем заглушки выхода
    int exitGaps = generateCode.getLastGap();
    
    // Запоминаем адрес начала тела
    int bodyPosition = generateCode.getCmdCounter();
    generateCode.setOrBodyTarget(bodyPosition);
    
    // Заполняем OR-заглушки (прыжок в тело и на проверку B)
    generateCode.fillOrGaps();
    
    generateCode.resetGaps();

    sequenceStatementsPrc();

    if (scanner.lex == Scanner::Lex::ELSE)
    {
        int gotoEnd = generateCode.getCmdCounter();
        generateCode.genGoTo(0);
        
        generateCode.fillGaps(exitGaps);
        
        checkLex(Scanner::Lex::ELSE);
        sequenceStatementsPrc();
        
        generateCode.fillGaps(gotoEnd);
    }
    else
    {
        generateCode.fillGaps(exitGaps);
    }

    checkLex(Scanner::Lex::END);
}

// ИЗМЕНЕНО: правильная обработка OR в WHILE
void Parser::whileStatementPrc()
{
    int whilePosition = generateCode.getCmdCounter();

    checkLex(Scanner::Lex::WHILE);
    
    generateCode.resetGaps();
    
    Item::ItemTypes expressionType = logicalExpressionPrc();
    checkBoolType(expressionType);

    int exitGaps = generateCode.getLastGap();
    
    // Заполняем OR-заглушки
    int bodyPosition = generateCode.getCmdCounter();
    generateCode.setOrBodyTarget(bodyPosition);
    generateCode.fillOrGaps();
    
    generateCode.resetGaps();

    checkLex(Scanner::Lex::DO);
    sequenceStatementsPrc();
    checkLex(Scanner::Lex::END);

    generateCode.genGoTo(whilePosition);
    generateCode.fillGaps(exitGaps);
}

void Parser::parameterPrc() { expressionPrc(); }

Item::ItemTypes Parser::expressionPrc()
{
    Item::ItemTypes simpleExpressionType = simpleExpressionPrc();
    
    if (scanner.lex == Scanner::Lex::EQ ||
        scanner.lex == Scanner::Lex::NE ||
        scanner.lex == Scanner::Lex::LT ||
        scanner.lex == Scanner::Lex::LE ||
        scanner.lex == Scanner::Lex::GT ||
        scanner.lex == Scanner::Lex::GE)
    {
        auto operation = scanner.lex;
        checkIntType(simpleExpressionType);
        scanner.nextLex();
        simpleExpressionType = simpleExpressionPrc();
        checkIntType(simpleExpressionType);
        generateCode.genComparison(scanner.getStringNameOfLex(operation));
        return Item::ItemTypes::Boolean;
    }
    return simpleExpressionType;
}

Item::ItemTypes Parser::logicalExpressionPrc()
{
    Item::ItemTypes leftType = expressionPrc(); 
    checkBoolType(leftType);

    while (scanner.lex == Scanner::Lex::AND || scanner.lex == Scanner::Lex::OR)
    {
        auto operation = scanner.lex;
        scanner.nextLex();
        
        // ИЗМЕНЕНО: для OR вызываем genLogicalOr ДО разбора правого операнда,
        // чтобы он извлёк заглушку ЛЕВОГО операнда из lastGapAddress.
        // Иначе B перезапишет lastGapAddress, и genLogicalOr возьмёт не ту заглушку.
        if (operation == Scanner::Lex::OR) {
            generateCode.genLogicalOr();
        }
        
        Item::ItemTypes rightType = expressionPrc();
        checkBoolType(rightType);

        if (operation == Scanner::Lex::AND) generateCode.genLogicalAnd();
        // для OR уже вызвали выше
    }
    
    return Item::ItemTypes::Boolean;
}

Item::ItemTypes Parser::simpleExpressionPrc()
{
    Item::ItemTypes termType;
    if (scanner.lex == Scanner::Lex::PLUS || scanner.lex == Scanner::Lex::MINUS)
    {
        auto operation = scanner.lex;
        scanner.nextLex();
        termType = termPrc();
        checkIntType(termType);
        if (operation == Scanner::Lex::MINUS) generateCode.genNegative();
    }
    else
    {
        termType = termPrc();
        if (scanner.lex == Scanner::Lex::PLUS || scanner.lex == Scanner::Lex::MINUS) checkIntType(termType);
    }

    while (scanner.lex == Scanner::Lex::PLUS || scanner.lex == Scanner::Lex::MINUS)
    {
        auto operation = scanner.lex;
        scanner.nextLex();
        termType = termPrc();
        checkIntType(termType);
        if (operation == Scanner::Lex::PLUS) generateCode.genAddition();
        else generateCode.genSubstraction();
    }
    return termType;
}

Item::ItemTypes Parser::termPrc()
{
    Item::ItemTypes multiplierType = multiplierPrc();

    if (scanner.lex == Scanner::Lex::DIV || scanner.lex == Scanner::Lex::MOD || scanner.lex == Scanner::Lex::MULT)
        checkIntType(multiplierType);

    while (scanner.lex == Scanner::Lex::DIV || scanner.lex == Scanner::Lex::MOD || scanner.lex == Scanner::Lex::MULT)
    {
        auto operation = scanner.lex;
        scanner.nextLex();
        multiplierType = multiplierPrc();
        checkIntType(multiplierType);
        generateCode.genOperation(scanner.getStringNameOfLex(operation));
    }
    return multiplierType;
}

Item::ItemTypes Parser::multiplierPrc()
{
    if (scanner.lex == Scanner::Lex::NOT)
    {
        scanner.nextLex();
        Item::ItemTypes type = multiplierPrc();
        checkBoolType(type);
        generateCode.genLogicalNot();
        return Item::ItemTypes::Boolean;
    }

    if (scanner.lex == Scanner::Lex::NAME)
    {
        Item* item = table.findItem(scanner.nameValue);
        if (item->typeOfItem == "const")
        {
            generateCode.genConst(std::stoi(item->value));
            scanner.nextLex();
            return item->type;
        }
        else if (item->typeOfItem == "var")
        {
            generateCode.genVar(*item);
            scanner.nextLex();
            return item->type;
        }
        else if (item->typeOfItem == "function")
        {
            scanner.nextLex();
            checkLex(Scanner::Lex::LPAR);
            checkFuncParameters(*item);
            checkLex(Scanner::Lex::RPAR);
            return item->type;
        }
        else errorPtr->contextError("Ожидается константа, имя или функция");
    }
    else if (scanner.lex == Scanner::Lex::NUM)
    {
        generateCode.gen(scanner.numValue);
        scanner.nextLex();
        return Item::ItemTypes::Integer;
    }
    else if (scanner.lex == Scanner::Lex::LPAR)
    {
        scanner.nextLex();
        Item::ItemTypes expressionType = expressionPrc();
        checkLex(Scanner::Lex::RPAR);
        return expressionType;
    }
    else errorPtr->syntaxError("Имя, число или '('");

    return Item::ItemTypes::Integer;
}

void Parser::contextImportPrc() 
{
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    if (scanner.nameValue == "In" || scanner.nameValue == "Out")
        table.newItem(table.moduleItem(scanner.nameValue));
    else errorPtr->contextError("Ожидается модуль `In` или `Out`");

    scanner.nextLex();
}

void Parser::checkProcParameters(Item item) 
{
    if (item.name == "HALT") { int value = constExpresionPrc(); generateCode.genHalt(value); }
    else if (item.name == "INC")
    {
        errorIsNotVariable();
        generateCode.genDup();
        generateCode.genLoad();
        if (scanner.lex == Scanner::Lex::COMMA) { scanner.nextLex(); Item::ItemTypes t = expressionPrc(); checkIntType(t); }
        else generateCode.gen(1);
        generateCode.genAddition();
        generateCode.genSave();
    }
    else if (item.name == "DEC")
    {  
        errorIsNotVariable();
        generateCode.genDup();
        generateCode.genLoad();
        if (scanner.lex == Scanner::Lex::COMMA) { scanner.nextLex(); Item::ItemTypes t = expressionPrc(); checkIntType(t); }
        else generateCode.gen(1);
        generateCode.genSubstraction();
        generateCode.genSave();
    }
    else if (item.name == "In.Open") {}
    else if (item.name == "In.Int") { errorIsNotVariable(); generateCode.genInInt(); }
    else if (item.name == "Out.Int")
    {
        Item::ItemTypes t = expressionPrc(); checkIntType(t);
        checkLex(Scanner::Lex::COMMA);
        t = expressionPrc(); checkIntType(t);
        generateCode.genOutInt();
    }
    else if (item.name == "Out.Ln") generateCode.genOutLn();
    else errorPtr->contextError("Неизвестная процедура");
}

void Parser::checkFuncParameters(Item item)
{
    if (item.name == "ABS") { Item::ItemTypes t = expressionPrc(); checkIntType(t); generateCode.genFunc("ABS"); }
    else if (item.name == "MIN") { typePrc(); generateCode.genFunc("MIN"); }
    else if (item.name == "MAX") { typePrc(); generateCode.genFunc("MAX"); }
    else if (item.name == "ODD") { Item::ItemTypes t = expressionPrc(); checkIntType(t); generateCode.genFunc("ODD"); }
    else errorPtr->contextError("Неизвестная функция");
}

void Parser::checkLex(Scanner::Lex lex)
{
    if (Scanner::lex == lex) scanner.nextLex();
    else errorPtr->syntaxError(scanner.getStringNameOfLex(lex));
}

void Parser::errorIfNotExpectedLex(Scanner::Lex lex)
{
    if (Scanner::lex != lex) errorPtr->syntaxError(scanner.getStringNameOfLex(lex));
}

void Parser::checkIntType(Item::ItemTypes type)
{
    if (type != Item::ItemTypes::Integer) errorPtr->contextError("Ожидается целый тип");
}

void Parser::checkBoolType(Item::ItemTypes type)
{
    if (type != Item::ItemTypes::Boolean) errorPtr->contextError("Ожидается логический тип");
}

void Parser::errorIsNotVariable() {
    errorIfNotExpectedLex(Scanner::Lex::NAME);
    Item* paramItem = table.findItem(scanner.nameValue);
    if (paramItem->typeOfItem != "var") errorPtr->contextError("Ожидается имя переменной");
    generateCode.genAddress(*paramItem);
    scanner.nextLex();
}