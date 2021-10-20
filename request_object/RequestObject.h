#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include "RequestEntry.h"


class RequestObject : public RequestEntry {
public:
    explicit RequestObject(std::string name) : RequestEntry(std::move(name)) {}

    bool AdditionalProperties = false;

    std::string CreateSchema() const override {
        std::stringstream rtn;

        rtn << "{";
        rtn << R"("type": "object", )";
        if (!AdditionalProperties) {
            rtn << R"("additionalProperties": false, )";
        }
        rtn << R"("properties": {)";


        auto counter = entries.size();
        std::vector<std::string> requiredEntries;
        for (const auto entry : entries) {
            if (entry.second->Required) {
                requiredEntries.emplace_back(entry.first);
            }

            rtn << '"' << entry.first << R"(": )";
            rtn << entry.second->CreateSchema();
            if (--counter != 0) {
                rtn << ',';
            }
        }

        std::stringstream required;
        required << R"("required": [)";
        for (int i = 0; i < requiredEntries.size(); ++i) {
            required << '"' << requiredEntries[i] << '"';
            if (i + 1 != requiredEntries.size()) {
                required << ',';
            }
        }
        required << ']';

        rtn << "}, ";
        rtn << required.view();
        rtn << '}';
        return rtn.str();
    }

    std::unordered_map<std::string, std::shared_ptr<RequestEntry>> entries;
};
