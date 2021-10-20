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
#include <FlowUtils/FlowUUID.h>
#include "../FlowAsio.h"

class SimpleUploadRoute : public Route {
public:
    SimpleUploadRoute(const std::string &basePath = ""
    ) : Route(".*", "POST") {
        this->basePath = basePath;
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) override {
        using namespace FlowAsio;

        const std::string INDEX_HTML = "index.html";
        const size_t BUFFER_SIZE = 8 * 1024;
        const size_t WRITE_TO_DISK_AT = 8 * 1024 * 1024;
        std::vector<unsigned char> caBuf;
        boost::system::error_code error;

        if (stoul(request.ContentLength()) < WRITE_TO_DISK_AT) {
            while (request.ParserState != HttpParserState::END) {
                size_t readed = readToVector(socket, caBuf, error, BUFFER_SIZE);
                caBuf.resize(readed);
                request << caBuf;
            }

            for (auto &d : request.fields) {
                if (d.FileName().empty())
                    continue;
                d.FullPath = FlowFile::combinePath(basePath, d.FileName());
                FlowFile::writeBinaryVector(d.FullPath, d.data);
            }
        } else {
            request.ParseToDisk = true;
            request.WriteFolder = basePath;
            FlowFile::createDirIfNotExist(request.WriteFolder, false);
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