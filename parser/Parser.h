#include "../scanner/Scanner.h"
#include "../table/TableOfName.h"
#include "../virtualMachine/generateCode.h"

#pragma once

class Parser
{
public:
    Parser(Scanner &scanner, std::shared_ptr<Error> errorPtr);
    void compile();

private:
    Scanner scanner;
    std::shared_ptr<Error> errorPtr;
    TableOfName table;
    GenerateCode generateCode;

    void modulePrc();
    void importPrc();
    void sequenceDeclarationsPrc();
    void sequenceStatementsPrc();
    void constDeclarationPrc();
    int constExpresionPrc();
    void varDeclarationPrc();
    void statementsPrc();
    void variableOrCallPrc();
    void typePrc();
    void ifStatementPrc();
    void whileStatementPrc();
    void parameterPrc();

    // ДОБАВЛЕНО: объявление функции для разбора логических выражений.
    // Без этой строки компилятор не будет знать о её существовании в Parser.cpp
    // и будет выдавать ошибку "was not declared in this scope".
    Item::ItemTypes logicalExpressionPrc();

    Item::ItemTypes expressionPrc();
    Item::ItemTypes simpleExpressionPrc();
    Item::ItemTypes termPrc();
    Item::ItemTypes multiplierPrc();

    void loccateVariables();

    void contextImportPrc();
    void contextVarPrc();
    void checkProcParameters(Item item);
    void checkFuncParameters(Item item);

    void checkLex(Scanner::Lex lex);
    void errorIfNotExpectedLex(Scanner::Lex lex);
    void checkIntType(Item::ItemTypes type);
    void checkBoolType(Item::ItemTypes type);
    void errorIsNotVariable();
};