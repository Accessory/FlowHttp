#pragma once

#include "RequestEntry.h"

class RequestString : public RequestEntry {
public:
    RequestString(std::string name) : RequestEntry(std::move(name)) {}

    std::string CreateSchema() const override{
        std::ostringstream rtn;
        rtn << '{';
        rtn << R"("type": "string")";
        rtn << '}';
        return rtn.str();
    }

};