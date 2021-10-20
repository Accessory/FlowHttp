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


class GetRoute : public Route {
public:
    GetRoute(const std::string &basePath, const std::string &urlBase = "", const bool isDlOnly = false, const std::string& path = ".*") : Route(
            path) {
        this->basePath = basePath;
        this->urlBase = urlBase;
        this->isDlOnly = isDlOnly;
    }

    bool Run(Request &request, Response &response, Socket &socket) override {
        using namespace FlowAsio;
        using namespace std;

        const std::string HTML_SUFFIX = ".html";
        const std::string INDEX_HTML = "index.html";
        const size_t BUFFER_SIZE = 128 * 1024;

        std::string path = request.Path();

        UrlEscape::UrlDecode(path);

        string normalPath = FlowString::subStrAt(path, urlBase, urlBase.size());
        string fullPath = FlowFile::combinePath(basePath, normalPath);
        string fileName = FlowFile::getFilename(fullPath);
        if (fileName == "." || FlowFile::isDirectory(fullPath)) {
            if (path.back() != '/') {
                sendFound(response, socket, path + '/');
                return true;
            }

            fileName = INDEX_HTML;
            fullPath = FlowFile::combinePath(fullPath, INDEX_HTML);
        } else if (FlowFile::fileExist(fullPath + HTML_SUFFIX)) {
            fullPath = fullPath + HTML_SUFFIX;
        }
        const auto extension = FlowFile::getFileExtension(fullPath);
        const auto mime = MimeTypes::extension_to_type(extension);
        if (!FlowFile::fileExist(fullPath)) {
            return false;
        }

        LOG_INFO << "Get file " << fullPath;

        //Content-Disposition
        if (isDlOnly)
            response["Content-Disposition"] = "attachment; filename=" + fileName;


        response.ContentType = mime;
        response.SetFile(fullPath);

        //Range
        if (request.count("Range"))
            response.Range(request.at("Range"));
        unsigned char data1[BUFFER_SIZE];
        unsigned char data2[BUFFER_SIZE];
        unsigned char *data = data1;
        bool dataSwitch = false;

        Semaphore m;

        string header = response.Header();
        m.lock();
        std::thread([&] {
            write(socket, header);
            m.unlock();
        }).detach();
        size_t readBytes = 0;
        bool hasError = false;
        while ((readBytes = response.ReadData(data, BUFFER_SIZE)) && !hasError) {
            m.lock();
            std::thread([&, data, readBytes] {
                if (!FlowAsio::write(socket, data, readBytes)) {
                    m.unlock();
                    hasError = true;
                    return;
                }
                m.unlock();
            }).detach();
            data = dataSwitch ? data1 : data2;
            dataSwitch = !dataSwitch;
        }
        m.lock();

        return true;
    }


private:
    std::string basePath;
    std::string urlBase;
    bool isDlOnly;
};