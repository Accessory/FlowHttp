#pragma once

#include <string>
#include <sstream>

class RequestEntry {
public:
    RequestEntry(std::string name, bool required = true) :
            Required(required) {}

    bool Required;

    virtual std::string CreateSchema() const {
        std::ostringstream rtn;
        rtn << '{';
        rtn << R"("type": ["string","boolean",)";
        rtn << R"("null", "integer", "array", "object"])";
        rtn << '}';
        return rtn.str();
    }
};
