#pragma once

#include <FlowUtils/FlowFile.h>
#include <FlowUtils/FlowUUID.h>
#include "../Request.h"


namespace UploadUtil {
    bool uploadTo(Request &request, Socket &socket, const std::string &filepath) {
        using namespace FlowAsio;

        const size_t BUFFER_SIZE = 8 * 1024;
        const size_t WRITE_TO_DISK_AT = 8 * 1024 * 1024;

        vector<unsigned char> caBuf;

        boost::system::error_code error;

        auto path = request.Path();

        if (stoul(request.ContentLength()) < WRITE_TO_DISK_AT) {
            while (request.ParserState != HttpParserState::END) {
                size_t readed = readToVector(socket, caBuf, error, BUFFER_SIZE);
                caBuf.resize(readed);
                request << caBuf;
            }

            FlowFile::createDirIfNotExist(filepath, false);
            for (auto &d : request.fields) {
                if (d.FileName().empty())
                    continue;

                if (request.FileNamingFunction == nullptr)
                    d.FullPath = FlowFile::combinePath(filepath, d.FileName());
                else
                    d.FullPath = request.FileNamingFunction(&d);

                FlowFile::writeBinaryVector(d.FullPath, d.data);
            }
        } else {
            request.ParseToDisk = true;
            request.WriteFolder = filepath;
            FlowFile::createDirIfNotExist(request.WriteFolder, false);

            for (auto &d : request.fields) {
                if (d.FileName().empty())
                    continue;

                d.FullPath = FlowFile::combinePath(filepath, d.FileName());

                FlowFile::writeBinaryVector(d.FullPath, d.data);
            }

            while (request.ParserState != HttpParserState::END) {
                size_t readed = readToVector(socket, caBuf, error, BUFFER_SIZE);
                caBuf.resize(readed);
                request << caBuf;
            }
        }
        return true;
    }
}