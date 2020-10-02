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
#include <rapidjson/document.h>
#include <FlowUtils/FlowJson.h>


class ListFilesJson : public Route {
public:
    ListFilesJson(const std::string &basePath, const std::string &urlBase = "", const std::string &path = ".*") : Route(
            path) {
        this->basePath = basePath;
        this->urlBase = urlBase;
    }

    bool Run(Request &request, Response &response, Socket &socket) override {
        using namespace FlowAsio;
        using namespace std;
        using namespace rapidjson;
        using namespace FlowJson;

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


        response.ContentType = "application/json";

        Document rs;
        auto &alloc = rs.GetAllocator();
        rs.SetObject();
        rs.AddMember("path", cS(path, rs), alloc);
        string pppath = path.substr(0, path.find_last_of("/\\", path.size() - 2) + 1);
        if (normalPath.size() > 0 && pppath.size() < path.size())
            rs.AddMember("parentPath", cS(pppath, rs), alloc);
        vector<string> files = FlowFile::getFilesInDirectory(fullPath);
        Value filesValues(kArrayType);
        for (string file : files) {
////            string refFile = file.substr(basePath.size());
            string fileName = FlowFile::getFilename(file);
            size_t fileSize = FlowFile::getFileSize(file);
//            string fileHR = FlowFile::byteToHumanReadable(fileSize);

//            rs << "<a href=\"" << fileName << "\">" << fileName << " (" << fileHR << ")" << "</a><br>";
            Value fileValue(kObjectType);
            fileValue.AddMember("fileName", cS(fileName, rs), alloc);
            fileValue.AddMember("fileSize", cS(std::to_string(fileSize), rs), alloc);
            filesValues.PushBack(fileValue.Move(), alloc);
        }
        rs.AddMember("files", filesValues, alloc);

        vector<string> folders = FlowFile::getFoldersInDirectory(fullPath);
        Value folderValues(kArrayType);
        for (string folder : folders) {
            auto toCutPass = folder.find_last_of("/\\") + 1;
            string refFolder = folder.substr(toCutPass);
            string folderName = refFolder.substr(refFolder.find_last_of("/\\") + 1);
            Value folderValue(kObjectType);
            folderValue.AddMember("name", cS(folderName, rs), alloc);
            folderValue.AddMember("reference", cS(refFolder, rs), alloc);
            folderValues.PushBack(folderValue.Move(), alloc);
//            rs << "<a href=\"" << refFolder << "/\">" << folderName << "/</a><br>";
        }

        rs.AddMember("folders", folderValues, alloc);
        response.Data(docToPrettyString(rs));
        write(socket, response.ToString());

        return true;
    }


private:
    std::string basePath;
    std::string urlBase;
};