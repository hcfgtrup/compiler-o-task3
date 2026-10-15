#include "Error.h"
#include "../sourceCodeDriver/Driver.h"
#include "../scanner/Scanner.h"

void Error::error(const std::string msg)
{
    std::cerr << "ERROR !!! " << msg << std::endl;
    exit(0);
}

void Error::skip(const unsigned int position)
{
    Driver driver;

    while (Driver::ch != Driver::chEOT && Driver::ch != Driver::chEOL)
    {
        driver.nextCh();
    }

    std::cout << std::endl << std::string((position == 0) ? position : (position - 1), ' ') << '^' << std::endl;
}

void Error::lexError(const std::string msg) 
{
    skip(Driver::position);
    std::cout << msg << std::endl;
    exit(0);
}

void Error::syntaxError(const std::string msg) 
{
    skip(Scanner::lexPosition);
    std::cout << "Ожидается: " + msg << std::endl;
    exit(0);
}

void Error::contextError(const std::string msg) 
{
    skip(Scanner::lexPosition);
    std::cout << "Контекстная ошибка: " + msg << std::endl;
    exit(0);
}