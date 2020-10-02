#pragma once

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

struct Cookie {
    Cookie(const std::string &name, const std::string &value, const std::time_t &expires, const std::string &path,
           const std::size_t &maxAge) {
        Name = name;
        Value = value;
        Expires = expires;
        Path = path;
        MaxAge = maxAge;
    }

    Cookie(const std::string &name, const std::string &value) {
        Name = name;
        Value = value;
        HasExpires = false;
        Path = "";
        MaxAge = 0;
    }


    std::string Name;
    std::string Value;
    std::time_t Expires;
    bool HasExpires;
    std::string Path;
    std::size_t MaxAge;

    std::string ToString() {
        std::stringstream rtn;
        rtn << Name << '=' << Value << ';';
        if (HasExpires)
            rtn << std::put_time(std::gmtime(&Expires), "%a, %d %m %Y %T GMT")<< ';';

        if (MaxAge != 0)
            rtn << "Max-Age=" << std::to_string(MaxAge)<< ';';

        if (!Path.empty())
            rtn << "Path=" << Path<< ';';


        return rtn.str();
    }
};