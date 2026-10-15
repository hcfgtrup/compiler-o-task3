#include "iostream"
#include "Item.h"
#include "vector"
#include "unordered_map"

#pragma once

class TableOfName
{
public:

    TableOfName() = default;

    void openScope();
    void closeScope();
    void addItem(const Item& item);
    void newItem(const Item& item);
    Item* findItem(std::string);
    std::vector<std::reference_wrapper<Item>> getVars();

    Item moduleItem(std::string name);
    Item constItem(std::string name, Item::ItemTypes type, std::string value);
    Item varItem(std::string name, Item::ItemTypes type, std::string addr);
    Item typeItem(std::string name, Item::ItemTypes type);
    Item functionItem(std::string name, Item::ItemTypes type);
    Item procedureItem(std::string name);

private:
    std::vector<std::unordered_map<std::string, Item>> table;
};