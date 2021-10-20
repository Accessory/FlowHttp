#pragma once

#include "RequestObject.h"
#include "RequestString.h"
#include "RequestInteger.h"
#include "RequestArray.h"

class TestObject : public RequestObject {
public:
    TestObject() : RequestObject("test") {
        this->entries.emplace("Ping", ping);
        this->entries.emplace("Pong", pong);
        this->entries.emplace("Array", jang);
    }

    std::shared_ptr<RequestString> ping = std::make_shared<RequestString>("Ping");
    std::shared_ptr<RequestInteger> pong = std::make_shared<RequestInteger>("Pong");
    std::shared_ptr<RequestArray> jang = std::make_shared<RequestArray>("Array");

};