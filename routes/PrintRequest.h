#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>

class PrintRequest : public Route {
public:
    PrintRequest() : Route(".*", ".*") {}

private:
    virtual bool Run(Request &request, Response &response, Socket &socket) {
        for(auto header : request){
            LOG_INFO << header.first << " : " << header.second;
        }
        for(auto parameter : request.Parameter){
            LOG_INFO << parameter.first << " : " << parameter.second;
        }

        return false;
    }

};