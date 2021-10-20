#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include "RequestEntry.h"


class RequestArray : public RequestEntry {
public:
    explicit RequestArray(std::string name) : RequestEntry(std::move(name)) {}

    std::string CreateSchema() const override {
        std::stringstream rtn;

        rtn << '{';
        rtn << R"("type": "array",)";
        rtn << R"("items": [)";

        for (size_t i = 0; i < items.size(); ++i) {
            rtn << R"("type": ")" << items[0] << '"';
            if(i != (items.size() - 1)){
                rtn << ',';
            }
        }
        rtn << ']';
        rtn << '}';
        return rtn.str();
    }

    std::vector<std::shared_ptr<RequestEntry>> items;
};
