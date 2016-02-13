#include <iostream>
#include "util.h"

std::map<std::string, std::string> parseAdditional(std::string additional)
{
    std::map<std::string, std::string> addTable;

    if (additional.size())
    {
        auto keyValuePairs = split(additional, ';');
        for (std::string kvPair : keyValuePairs)
        {
            if (kvPair.size() > 2)
            {
                auto kvPairParts = split(kvPair, '=', 2);
                if (kvPairParts.size() == 2)
                    addTable.insert(std::make_pair(kvPairParts[0], kvPairParts[1]));
                else
                    Out::warning("Invalid kvPair: " + kvPair);
            }
        }
    }

    return addTable;
}
