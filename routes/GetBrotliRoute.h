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
#include <brotli/encode.h>


class GetBrotliRoute : public Route {
public:
    GetBrotliRoute(const std::string &basePath, const std::string &urlBase = "", const bool isDlOnly = false) : Route(
            ".*") {
        this->basePath = basePath;
        this->urlBase = urlBase;
        this->isDlOnly = isDlOnly;
    }

    bool Run(Request &request, Response &response, Socket &socket) override {
        using namespace FlowAsio;
        using namespace std;

        const auto acceptEncoding = request.Header("Accept-Encoding");
        if(acceptEncoding.find("br") == std::string::npos)
            return false;

        const std::string HTML_SUFFIX = ".html";
        const std::string INDEX_HTML = "index.html";
        const size_t BUFFER_SIZE = 1024 * 1024 * 16; // 16 MB Brotli limit

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


        if (!FlowFile::fileExist(fullPath)) {
            return false;
        }

        LOG_INFO << "Get file " << fullPath;
        string extension = FlowFile::getFileExtension(fullPath);
        if (extension != ".js"
            && extension != ".css"
            && extension != ".html"
            && extension != ".txt"
            && extension != ".txt"
            && extension != ".yaml")
            return false;
            string mime = MimeTypes::extension_to_type(extension);
        const auto file_size = FlowFile::getFileSize(fullPath);
        if (file_size > BUFFER_SIZE)
            return false;
        response.ContentType = mime;
        response.SetFile(fullPath);


        auto file_size_string = std::to_string(file_size);
        auto time = FlowFile::getLastModified(fullPath);
        std::stringstream lastModified;
        lastModified << std::put_time(std::gmtime(&time), "%a, %d %m %Y %T GMT");
        string lmTime = lastModified.str();

        auto file_hash = fullPath + ';' + file_size_string + ";" + lmTime;

        auto data = FlowFile::loadBytes(fullPath);

        vector<unsigned char> compressed_data;
        compressed_data.reserve(file_size);
        //unsigned char compressed_data[file_size];
        size_t encoded_size = file_size;
        auto result = BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW, BROTLI_DEFAULT_MODE,
                                            file_size,
                                            data.data(), &encoded_size, compressed_data.data());

        response.SetContentSize(encoded_size);


        response["Content-Encoding"] = "br";
        //Content-Disposition
        if (isDlOnly)
            response["Content-Disposition"] = "attachment; filename=" + fileName;

        auto header = response.Header();
        FlowAsio::write(socket, header);
        FlowAsio::write(socket, compressed_data.data(), encoded_size);
        return true;
    }


private:
    std::string basePath;
    std::string urlBase;
    bool isDlOnly;
};