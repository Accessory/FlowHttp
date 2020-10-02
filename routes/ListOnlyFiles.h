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


class ListOnlyFiles : public Route {
public:
    ListOnlyFiles(const std::string &basePath, const std::string &urlBase = "", const std::string & path = ".*") : Route(
            path) {
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
        if (!FlowFile::isDirectory(fullPath)) {
            return false;
        }

        if (path.back() != '/') {
            sendFound(response, socket, path + '/');
            return true;
        }


        response.ContentType = "text/html";

        stringstream rs;
        rs << "<h2>Index of " << path << "</h2>";
        vector<string> files = FlowFile::getFilesInDirectory(fullPath);
        for (string file : files) {
            string fileName = FlowFile::getFilename(file);
            size_t fileSize = FlowFile::getFileSize(file);
            string fileHR = FlowFile::byteToHumanReadable(fileSize);
            rs << "<a href=\"" << fileName << "\">" << fileName << " (" << fileHR << ")" << "</a><br>";
        }
        response.Data(rs.str());
        write(socket, response.ToString());

        return true;
    }


private:
    std::string basePath;
    std::string urlBase;
};