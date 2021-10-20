#pragma once

#include <string>
#include "Cookie.h"
#include <FlowUtils/FlowParser.h>

namespace CookieUtil {

    std::string parseCookieValue(const std::string &cookies, const std::string &name) {
        size_t start = cookies.find(name + "=");
        if (start == std::string::npos)
            return "";

        start += name.size() + 1;
        size_t end = cookies.find(';', start);
        if (end == std::string::npos) {
            return cookies.substr(start);
        }

        return cookies.substr(start, end - start);
    }

};
