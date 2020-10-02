#pragma once

#include <map>
#include <vector>
#include <string>
#include <boost/uuid/uuid.hpp>

struct Session{
    Session(const boost::uuids::uuid& uuid) {
        id = uuid;
    }

    boost::uuids::uuid id;
    bool IsAuthorized;
    std::map<std::string,std::vector<std::string>> infos;

    bool operator<(const boost::uuids::uuid& uuid) {
        return this->id < uuid;
    }
};