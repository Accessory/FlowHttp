#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include <functional>
#include <iomanip>
#include <time.h>
#include "../MimeTypes.h"
#include "../Socket.h"
#include <FlowUtils/Semaphore.h>
#include <FlowUtils/UrlEscape.h>
#include <FlowUtils/FlowFile.h>


class IfModifiedSince : public Route {
public:
    IfModifiedSince(const std::string &basePath, const std::string &urlBase = "", const bool isDlOnly = false) : Route(
            ".*") {
        this->basePath = basePath;
        this->urlBase = urlBase;
    }

    bool Run(Request &request, Response &response, Socket &socket) override {
        using namespace FlowAsio;
        using namespace std;

        std::string path = request.Path();
        UrlEscape::UrlDecode(path);
        string normalPath = FlowString::subStrAt(path, urlBase, urlBase.size());
        string fullPath = FlowFile::combinePath(basePath, normalPath);

        //Last-Modified
        string ifModSince = request.IfModifiedSince();
        if (!ifModSince.empty() && ifModSince.substr(ifModSince.size() - 3) != "GMT") {
            response.StatusCode = HttpStatusCode::BadRequest;
            write(socket, response.ToString());
            return true;
        }
        istringstream requestStream(request.IfModifiedSince());
        std::tm requestTime = {};
        requestStream >> std::get_time(&requestTime, "%a, %d %m %Y %T GMT");

        std::stringstream lastModified;

        if(!FlowFile::fileExist(fullPath))
            return false;

        auto time = FlowFile::getLastModified(fullPath);
        lastModified << std::put_time(std::gmtime(&time), "%a, %d %m %Y %T GMT");
        string lmTime = lastModified.str();
        response.LastModified(lastModified.str());
        if (request.IfModifiedSince() == lmTime) {
            response.StatusCode = HttpStatusCode::NotModified;
            write(socket, response.ToString());
            return true;
        }

        return false;
    }

private:
    std::string basePath;
    std::string urlBase;
};