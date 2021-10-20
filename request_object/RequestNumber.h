#pragma once

#include "RequestEntry.h"

class RequestNumber : public RequestEntry {
public:
    RequestNumber(std::string name) : RequestEntry(std::move(name)) {}

    std::string CreateSchema() const override{
        std::ostringstream rtn;
        rtn << '{';
        rtn << R"("type": "number")";
        rtn << '}';
        return rtn.str();
    }

};