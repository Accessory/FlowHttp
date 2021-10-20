#pragma once

#include "RequestEntry.h"

class RequestInteger : public RequestEntry {
public:
    RequestInteger(std::string name) : RequestEntry(std::move(name)) {}

    std::string CreateSchema() const override{
        std::ostringstream rtn;
        rtn << '{';
        rtn << R"("type": "integer")";
        rtn << '}';
        return rtn.str();
    }

};