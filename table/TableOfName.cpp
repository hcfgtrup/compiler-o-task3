#include "TableOfName.h"
#include "../error/Error.h"
#include "Item.h"
#include <memory>

void TableOfName::openScope()
{
    table.push_back(std::unordered_map<std::string, Item>{});
}

void TableOfName::closeScope()
{
    table.pop_back();
}

void TableOfName::addItem(const Item&  item)
{
    table.back()[item.name] = item;
}

void TableOfName::newItem(const Item&  item)
{
    auto& last = table.back();
    if (last.find(item.name) != last.end())
    {
        std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
        errorPtr->contextError("Повторное обьявление имени");
    }
    else {
        addItem(item);
    }
}

Item* TableOfName::findItem(std::string name)
{
    for (auto it = table.rbegin(); it != table.rend(); ++it)
    {
        auto& block = *it;
        if (block.find(name) != block.end())
        {
            return &block[name];
        }
    }

    std::unique_ptr<Error> errorPtr = std::make_unique<Error>(); 
    errorPtr->contextError("Необъявленное имя");
}

Item TableOfName::moduleItem(std::string name)
{
    Item item;
    item.name = name;
    item.typeOfItem = "module";
    return item;
}

Item TableOfName::constItem(std::string name, Item::ItemTypes type, std::string value)
{
    Item item;
    item.name = name;
    item.type = type;
    item.value = value;
    item.typeOfItem = "const";
    return item;
}

Item TableOfName::varItem(std::string name, Item::ItemTypes type, std::string addr)
{
    Item item;
    item.name = name;
    item.type = type;
    item.addr = addr;
    item.typeOfItem = "var";
    return item;
}

Item TableOfName::typeItem(std::string name, Item::ItemTypes type)
{
    Item item;
    item.name = name;
    item.type = type;
    item.typeOfItem = "type";
    return item;
}

Item TableOfName::functionItem(std::string name, Item::ItemTypes type)
{
    Item item;
    item.name = name;
    item.type = type;
    item.typeOfItem = "function";
    return item;
}

Item TableOfName::procedureItem(std::string name)
{
    Item item;
    item.name = name;
    item.typeOfItem = "procedure";
    return item;
}

std::vector<std::reference_wrapper<Item>> TableOfName::getVars()
{
    std::vector<std::reference_wrapper<Item>> vars;
    std::unordered_map<std::string, Item>& lastScope = table.back();
    
    for (auto& [name, item] : lastScope) {
        if (item.typeOfItem == "var") {
            vars.push_back(item);
        }
    }

    return vars;
}