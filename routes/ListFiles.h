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


class ListFiles : public Route {
public:
    ListFiles(const std::string &basePath, const std::string &urlBase = "", const std::string & path = ".*") : Route(
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
        string pppath = path.substr(0, path.find_last_of("/\\", path.size() - 2) + 1);
        if (normalPath.size() > 0 && pppath.size() < path.size())
            rs << "<a href=\"" << pppath << "\">" << ".." << "</a><br>";
        vector<string> files = FlowFile::getFilesInDirectory(fullPath);
        for (string file : files) {
//            string refFile = file.substr(basePath.size());
            string fileName = FlowFile::getFilename(file);
            size_t fileSize = FlowFile::getFileSize(file);
            string fileHR = FlowFile::byteToHumanReadable(fileSize);
            rs << "<a href=\"" << fileName << "\">" << fileName << " (" << fileHR << ")" << "</a><br>";
        }

        vector<string> folders = FlowFile::getFoldersInDirectory(fullPath);
        for (string folder : folders) {
            auto toCutPass = folder.find_last_of("/\\") + 1;
            string refFolder = folder.substr(toCutPass);
            string folderName = refFolder.substr(refFolder.find_last_of("/\\") + 1);
            rs << "<a href=\"" << refFolder << "/\">" << folderName << "/</a><br>";
        }
        response.Data(rs.str());
        write(socket, response.ToString());

        return true;
    }


private:
    std::string basePath;
    std::string urlBase;
};