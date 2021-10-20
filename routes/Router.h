#pragma once

#include <map>
#include <string>
#include <regex>
#include <functional>
#include "Route.h"
#include <memory>
#include <vector>
#include "../Socket.h"

struct Router : public std::vector<std::shared_ptr<Route>> {

    void addRoute(const std::shared_ptr<Route> route) {
        this->emplace_back(route);
    }

    void addRoute(const std::shared_ptr<Route> route, const std::string &path) {
        this->emplace_back(route);
        route->Path = path;
    }

    bool execRoute(Request &request, Response &response, Socket &socket) {
        for (auto &route : *this) {
//            LOG_DEBUG << request.Path();
//            LOG_DEBUG << route->Path_String;
            if (std::regex_match(request.Path(), route->Path) && std::regex_match(request.Method(), route->Method)) {
                if (route->Run(request, response, socket))
                    return true;
            }
        }
        return false;
    }

//    std::vector<std::string> listRoutes() {
//        std::vector<std::string> rtn;
//        for(const auto& route : *this ){
//            rtn.emplace_back(route->Method_String + " " +route->Path_String);
//        }
//        return rtn;
//    }

};
