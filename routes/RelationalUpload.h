#pragma once

#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../FlowAsio.h"
#include <functional>
#include "../Request.h"
#include "../Socket.h"
#include <functional>
#include <boost/filesystem.hpp>
#include <FlowUtils/FlowUUID.h>

class RelationalUpload : public Route {
public:
    RelationalUpload(const std::string &basePath = ".", const std::string &route = ".*"
    ) : Route(route, "POST") {
        this->basePath = basePath;
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) override {
        using namespace FlowAsio;

        const size_t BUFFER_SIZE = 8 * 1024;
//        const size_t BUFFER_SIZE = 124;
//        const size_t BUFFER_SIZE = 65536;
        const size_t WRITE_TO_DISK_AT = 8 * 1024 * 1024;

        vector<unsigned char> caBuf;

        boost::system::error_code error;

        auto path = request.Path();
        string fullPath = FlowFile::combinePath(basePath, path);

        if (stoul(request.ContentLength()) < WRITE_TO_DISK_AT) {
            while (request.ParserState != HttpParserState::END) {
                size_t readed = readToVector(socket, caBuf, error, BUFFER_SIZE);
                caBuf.resize(readed);
                request << caBuf;
            }

            FlowFile::createDirIfNotExist(fullPath, false);
            for (auto &d : request.fields) {
                if (d.FileName().empty())
                    continue;

                d.FullPath = FlowFile::combinePath(fullPath, d.FileName());

                FlowFile::writeBinaryVector(d.FullPath, d.data);
            }
        } else {
            request.ParseToDisk = true;
            request.WriteFolder = fullPath;
            FlowFile::createDirIfNotExist(request.WriteFolder, false);

            for (auto &d : request.fields) {
                if (d.FileName().empty())
                    continue;

                d.FullPath = FlowFile::combinePath(fullPath, d.FileName());

                FlowFile::writeBinaryVector(d.FullPath, d.data);
            }


            while (request.ParserState != HttpParserState::END) {
                size_t readed = readToVector(socket, caBuf, error, BUFFER_SIZE);
                caBuf.resize(readed);
                request << caBuf;
            }
        }
        sendOk(socket);
        return true;
    }

private:
    std::string basePath;
};