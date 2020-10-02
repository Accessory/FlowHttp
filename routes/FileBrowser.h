#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include "../Socket.h"
#include <sstream>
#include <FlowUtils/FlowFile.h>
#include <rapidjson/document.h>
#include <FlowUtils/FlowJsonGod.h>
#include <FlowUtils/FlowString.h>


class FileBrowser : public Route {
public:
    FileBrowser(const std::string &base,const std::string &urlBase) : Route(".*", ".*") {
        this->base = base;
        this->urlBase = urlBase;
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace FlowAsio;
        using namespace std;
        using namespace rapidjson;

        stringstream ss;
        string normPath = FlowString::subStrAt(request.Path(), this->urlBase, this->urlBase.size());
        LOG_INFO << request.Path();
        LOG_INFO << normPath;
        string fullPath = FlowFile::combinePath(this->base, normPath);
        LOG_INFO << fullPath;
        vector<string> files = FlowFile::getFilesInDirectory(fullPath, "");

        for (auto &file : files)
            file = FlowString::subStrAt(file, this->base, this->base.size());

        Document document(kArrayType);

        auto data = FlowJsonGod::valuesToArray(files);
        LOG_INFO << data;
        response.Data(data);

        write(socket, response.ToString());
        return true;
    }

private:
    std::string base;
    std::string urlBase;
};