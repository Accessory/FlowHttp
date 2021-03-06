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
#include <FlowUtils/FlowFile.h>

class StaticUploadRoute : public Route
{
  public:
    StaticUploadRoute(const std::string &basePath = boost::filesystem::temp_directory_path().string(),
                      const std::function<void(Request &, Response &, Socket &)> &callback = [](Request &, Response &,
                                                                                                Socket &) {}) : Route("/upload",
                                                                                                                      "POST")
    {
        this->basePath = basePath;
        this->callback = callback;
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) override
    {
        using namespace FlowAsio;

        const size_t BUFFER_SIZE = 8 * 1024;
        const size_t WRITE_TO_DISK_AT = 8 * 1024 * 1024;

        std::vector<unsigned char> caBuf;

        boost::system::error_code error;

        if (stoul(request.ContentLength()) < WRITE_TO_DISK_AT)
        {
            while (request.ParserState != HttpParserState::END)
            {
                size_t readed = readToVector(socket, caBuf, error, BUFFER_SIZE);
                caBuf.resize(readed);
                request << caBuf;
            }

            for (auto &d : request.fields)
            {
                if (d.FileName().empty())
                    continue;
                std::string uploadPath = FlowFile::combinePath(basePath, FlowUUID::UUID_String());
                d.FullPath = FlowFile::combinePath(uploadPath, d.FileName());
                FlowFile::writeBinaryVector(d.FullPath, d.data);
            }
        }
        else
        {
            request.ParseToDisk = true;
            request.WriteFolder = FlowFile::combinePath(basePath, FlowUUID::UUID_String());
            FlowFile::createDirIfNotExist(request.WriteFolder, false);
            while (request.ParserState != HttpParserState::END)
            {
                size_t readed = readToVector(socket, caBuf, error, BUFFER_SIZE);
                caBuf.resize(readed);
                request << caBuf;
            }
        }
        sendOk(socket);
        callback(request, response, socket);
        return true;
    }

  private:
    std::string basePath;
    std::function<void(Request &, Response &, Socket &)> callback;
};