#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include <FlowUtils/FlowJson.h>
#include "../request_object/TestObject.h"


class TestRoute : public Route {
public:
    TestRoute() : Route(".*", ".*") {}

private:

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        TestObject testObject;
        const auto schema = testObject.CreateSchema();
        LOG_INFO << "Schema:\n" << schema;
        bool error;
        const auto object = FlowJson::parseJson(request.GetBody(), schema, error);
        if(error){
            LOG_WARNING << "Schema Failed";
        } else {
            LOG_INFO << "Schema Succeed!";
        }
        FlowAsio::sendOk(socket);
        request.CloseConnection();
        return true;
    }


};