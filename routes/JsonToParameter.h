#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <rapidjson/document.h>
#include <FlowUtils/FlowJson.h>

class JsonToParameter : public Route {
public:
    JsonToParameter() : Route(".*", ".*") {}

private:
    virtual bool Run(Request &request, Response &response, Socket &socket) {
        if ((request.ContentType() == "application/json" ||
             request.ContentType() == "application/json;charset=UTF-8" ||
             request.ContentType() == "application/json;charset=utf-8") && request.Parameter.count("body")) {
            auto body = request.GetParameter("body");

            if(body.empty())
                return false;

            auto doc = FlowJson::parseJson(body);

            if(doc.GetType() != rapidjson::kObjectType)
                return false;

            for (auto &value : doc.GetObject()) {
                std::string key = value.name.GetString();
                std::string value_str = FlowJson::toString(value.value);
                request.AddParameter(key, value_str);
            }
        }
        return false;
    }

};