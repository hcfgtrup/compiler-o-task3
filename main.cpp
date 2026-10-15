#include "iostream"
#include "sourceCodeDriver/Driver.h"
#include "parser/Parser.h"
#include "error/Error.h"
#include "scanner/Scanner.h"

int main(int argc, char const *argv[])
{   
    std::cout << "Компилятор языка O" << std::endl;
    std::shared_ptr<Error> errorPtr = std::make_shared<Error>();
    Driver driver(errorPtr);

    if (argc < 2) 
    {
        errorPtr->error("В качестве аргумента должен быть указан файл *.o");
    }

    driver.resetText(argv[1]);

    Scanner scanner(driver, errorPtr);
    Parser parser(scanner, errorPtr);
    parser.compile();

    std::cout << std::endl << "Скомпилировано" << std::endl;

    return 0;
}