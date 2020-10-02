#pragma once

#include <map>
#include <string>
#include <regex>
#include <functional>
#include "Route.h"
#include <memory>
#include <vector>
#include "../Socket.h"

struct Router : public std::vector<shared_ptr<Route>> {

    void addRoute(const shared_ptr<Route> route) {
        this->emplace_back(route);
    }
    void addRoute(const shared_ptr<Route> route, const string &path) {
        this->emplace_back(route);
        route->Path = path;
    }

    bool execRoute(Request &request, Response &response, Socket &socket) {
        for (auto &rout : *this) {
            if (std::regex_match(request.Path(), rout->Path) && std::regex_match(request.Method(), rout->Method)) {
                if (rout->Run(request, response, socket))
                    return true;
            }
        }
        return false;
    }

};
