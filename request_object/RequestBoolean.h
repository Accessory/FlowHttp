#pragma once

#include "RequestEntry.h"

class RequestBoolean : public RequestEntry {
public:
    RequestBoolean(std::string name) : RequestEntry(std::move(name)) {}

    std::string CreateSchema() const override{
        std::ostringstream rtn;
        rtn << '{';
        rtn << R"("type": "boolean")";
        rtn << '}';
        return rtn.str();
    }

};