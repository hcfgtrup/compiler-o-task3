#include "iostream"
#pragma once

class Error
{
private:
    void skip(const unsigned int position);
public:
    void error(const std::string msg);
    void lexError(const std::string msg);
    void syntaxError(const std::string msg);
    void contextError(const std::string msg);
};