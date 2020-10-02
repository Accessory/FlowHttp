#pragma once

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include "HttpStatus.h"
#include "Response.h"
#include "Cookie.h"
#include <algorithm>
#include <FlowUtils/FlowSParser.h>
#include <iterator>

class Response : public std::map<std::string, std::string> {
private:
    std::unique_ptr<std::ifstream> stream;
    std::string filePath;
    std::size_t contentSize;
    std::size_t leftToRead;
    std::vector<unsigned char> data;
    std::vector<Cookie> cookies;

public:
    std::string HttpVersion;
    HttpStatusCode StatusCode;
    std::string ContentType;


    Response() {
        HttpVersion = "HTTP/1.1";
        StatusCode = HttpStatusCode::OK;
        ContentType = "text/plain";
        contentSize = 0;
    }

    Response(bool isSSL) {
        HttpVersion = isSSL ? "HTTPS/1.1" : "HTTP/1.1";
        StatusCode = HttpStatusCode::OK;
        ContentType = "text/plain";
        contentSize = 0;
    }

    inline void SetContentSize(const size_t &contentSize) {
        this->contentSize = contentSize;
    }

    inline void AddCookie(const Cookie &cookie) {
        cookies.emplace_back(cookie);
    }

    inline void AddCookie(const std::string &name, const std::string &value) {
        cookies.emplace_back(name, value);
    }

    inline std::string ToString() const {
        using namespace std;

        stringstream rtn;
        rtn << Header();

        if (!data.empty()) {
            copy(data.begin(), data.end(), ostream_iterator<unsigned char>(rtn));
        }

        return rtn.str();
    }

    inline bool Range(const std::string &range) {
        using namespace FlowSParser;
        if (!startWith(range, "bytes="))
            return false;

        size_t np;
        size_t startOffset = nextSizeT(range.substr(6), &np);

        if (startOffset < contentSize) {
            stream->seekg(startOffset);
            leftToRead -= startOffset;
        } else {
            return false;
        }

        size_t endOffset = contentSize;
        if (range.length() > np + 7 && range.at(np + 6) == '-') {
            endOffset = nextSizeT(range.substr(np + 7), &np);
            if (endOffset < contentSize && endOffset > startOffset) {
                leftToRead = endOffset - startOffset;
            }
        }

        this->emplace("Content-Range", "bytes " + to_string(startOffset) + "-" + to_string(endOffset) + "/" +
                                       to_string(contentSize));

        return true;
    }

    inline size_t ReadData(unsigned char *data, const size_t bufferSize) {

        if (leftToRead > bufferSize) {
            stream->read(reinterpret_cast<char *>(&data[0]), bufferSize);
            leftToRead -= bufferSize;
            return bufferSize;
        }
        stream->read(reinterpret_cast<char *>(&data[0]), leftToRead);
        auto toRtn = static_cast<size_t>(leftToRead);
        leftToRead = 0;
        return toRtn;
    }

    inline void Data(const std::vector<unsigned char> &data) {
        this->data.insert(this->data.end(), data.begin(), data.end());
    }

    inline void Data(const std::string &data) {
        copy(data.begin(), data.end(), back_inserter(this->data));
    }

    inline void LastModified(const std::string &lastModified) {
        this->emplace("Last-Modified", lastModified);
    }

    inline std::string Header() const {
        using namespace std;
        stringstream rtn;
        rtn << HttpVersion << " " << static_cast<int>(StatusCode) << " " << HttpStatus::codeToString(StatusCode);

        for (auto kv : *this) {
            rtn << endl << kv.first << ": " << kv.second;
        }
        rtn << endl;
        if (HttpStatus::isError(StatusCode)) {
            rtn << endl;
            return rtn.str();
        }

        auto data_size = std::max(data.size(), contentSize);

        rtn << "Content-Length: " << data_size << endl;
        rtn << "Content-Type: " << ContentType;
        for (auto cookie : cookies)
            rtn << endl << "Set-Cookie: " << cookie.ToString();

        rtn << endl << endl;

        return rtn.str();
    }

    inline void SetFile(const std::string &path) {
        filePath = path;

        stream = std::make_unique<std::ifstream>(filePath, std::ifstream::ate | std::ios::binary);
        contentSize = static_cast<size_t>(stream->tellg());
        stream->seekg(0, stream->beg);
        leftToRead = contentSize;
    }

    inline std::vector<unsigned char> ToBytes() {

        auto header = Header();
        std::vector<unsigned char> rtn(header.begin(), header.end());

        if (!data.empty())
            rtn.insert(rtn.end(), data.begin(), data.end());


        return rtn;
    }
};


