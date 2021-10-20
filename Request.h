#pragma once

#include <map>
#include <string>
#include <vector>
#include "MulPaField.h"
#include "HttpParserState.h"
#include <sstream>
#include <functional>

#include <FlowUtils/FlowCParser.h>
#include <FlowUtils/FlowParser.h>
#include <FlowUtils/FlowString.h>
#include <FlowUtils/FlowVParser.h>
#include <FlowUtils/FlowLog.h>
#include <string_view>

class Request : public std::unordered_map<std::string, std::string> {
public:

    std::function<std::string(MulPaField *)> FileNamingFunction = nullptr;

    std::string HttpVersion() {
        return Header("HTTP_Version");
    }

    void HttpVersion(const std::string &version) {
        this->operator[]("HTTP_Version") = version;
    }

    void Method(const std::string &version) {
        this->operator[]("Method") = version;
    }

    std::string Method() {
        return Header("Method");
    }

    std::string Protocol() {
        return Header("Protocol");
    }

    void Protocol(const std::string &version) {
        this->operator[]("Protocol") = version;
    }

    std::string Path() {
        return Header("Path");
    }

    void Path(const std::string &path) {
        this->operator[]("Path") = path;
    }

    void AddParameter(const std::string &key, const std::string &value) {
        Parameter[key] = value;
    }

    std::string GetBody() {
        return GetParameter("$Body");
    }

    std::string GetParameter(const std::string &key) {
        auto itr = Parameter.find(key);
        if (itr == Parameter.end())
            return "";
        return Parameter.find(key)->second;
    }

    std::string ContentLength() {
        return Header("Content-Length");
    }

    void ContentLength(const std::string &contentLength) {
        this->operator[]("Content-Length") = contentLength;
    }

    std::string ContentType() {
        return Header("Content-Type");
    }

    void ContentType(const std::string &contentType) {
        this->operator[]("Content-Type") = contentType;
    }

    std::string Header(const std::string &key) {
        auto keyItr = this->find(key);
        if (keyItr == this->end()) {
            std::string key_c = key;
            FlowString::toLower(key_c);
            keyItr = this->find(key_c);
            if (keyItr == this->end())
                return "";
        }
        return keyItr->second;
    }

    bool ExpectContinue() {
        return Expect() == "100-continue";
    }

    std::string LastModified() {
        return Header("Last-Modified");
    }

    std::string IfModifiedSince() {
        return Header("If-Modified-Since");
    }

    std::string Expect() {
        return Header("Expect");
    }

    void Expect(const std::string &value) {
        this->operator[]("Content-Expect") = value;
    }

    void InsertField(MulPaField &field) {

    }

//    Request &operator<<(const std::string &data) {
//        ParserState = parseRequest(data, *this, ParserState);
//        return *this;
//    }

    Request &operator<<(std::vector<unsigned char> &data) {
        ParserState = parseRequest(data, *this, ParserState);
        return *this;
    }

    std::string ToString() {
        std::stringstream ss;
        ss << this->Method() << " " << this->Path() << " " << this->Protocol() << "/" << this->HttpVersion();
        for (auto item: *this) {
            ss << std::endl << item.first << ": " << item.second;
        }
        return ss.str();
    }

    Request() {
        ParserState = HttpParserState::START;
    }

    void CloseFiles() {
        for (auto &field: fields)
            field.CloseFile();
    }

    void CloseConnection() {
        this->operator[]("Connection") = "";
    }


    std::map<std::string, std::string> Parameter;
    std::string Boundary;
    std::vector<MulPaField> fields;
    HttpParserState ParserState;
    bool ParseToDisk = false;
    std::string WriteFolder;

private:

    std::vector<unsigned char> prefBuffer;

    void parseFieldHeaderPart(const std::string &req, size_t &pos, MulPaField &field) {
        while (pos < req.size() && !FlowParser::isDoubleNewLine(req, pos)) {
            if (pos >= req.size()) break;
            const auto key = FlowParser::goTo(req, ":", pos);
            FlowParser::gotoNextNonWhite(req, ++pos);
            const auto value = FlowParser::goToNewLine(req, pos);
            field[key] = value;
        }
    }

    HttpParserState parseHTML2(const std::string &req, size_t &pos, Request &request) {
        const std::string HTML2_PRI_CHECK = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
        if (req.substr(0, HTML2_PRI_CHECK.length()) != HTML2_PRI_CHECK) {
            return HttpParserState::BAD_REQUEST;
        }
        request.AddParameter("data", req.substr(HTML2_PRI_CHECK.length()));
        return HttpParserState::END;
    }

    void parseHeaderPart(const std::string &req, size_t &pos, Request &request) {
        while (pos < req.size() && !FlowParser::isDoubleNewLine(req, pos)) {
            if (pos >= req.size()) break;
            const auto key = FlowParser::goTo(req, ":", pos);
            FlowParser::gotoNextNonWhite(req, ++pos);
            const auto value = FlowParser::goToNewLine(req, pos);
            request[key] = value;
        }
    }

    HttpParserState
    parseRequest(std::vector<unsigned char> &data, Request &request, HttpParserState state) {
        using namespace FlowVParser;

        if (!prefBuffer.empty()) {
            data.insert(data.begin(), prefBuffer.begin(), prefBuffer.end());
            prefBuffer.clear();
        }

        std::vector<unsigned char>::iterator pos = data.begin();
        if (state == HttpParserState::START) {

            if (pos == data.end() || !isOneOf(pos,
                                              {"GET", "POST", "PUT", "PATCH", "DELETE", "OPTION", "HEAD", "CONNECT",
                                               "TRACE", "COPY", "LOCK", "MKCOL", "MOVE", "PROPFIND", "PROPPATCH",
                                               "UNLOCK"})) {
                state = HttpParserState::BAD_REQUEST;
                return state;
            }

            auto method = gotoNextNonAlpha(data, pos);
            FlowString::toUpper(method);
            request.Method(method);
            if (pos == data.end() || method.empty()) {
                state = HttpParserState::END;
                return state;
            }
            const auto test = gotoNextNonWhite(data, pos);
            const auto path = goToOne(data, " ?", pos);
            request.Path(path);
            while (*pos == '?' || *pos == '&') {

                const auto key = goToOne(data, " =", ++pos);
                if (key.empty()) {
                    ++pos;
                    continue;
                }
                const auto value = goToOne(data, " &", ++pos);
                if (!key.empty() && !value.empty()) {
                    request.AddParameter(key, value);
                }
            }
            gotoNextNonWhite(data, pos);
            const auto protocol = goTo(data, "/", pos);
            request.Protocol(protocol);
            const auto httpVersion = goToNewLine(data, ++pos);
            request.HttpVersion(httpVersion);

            state = HttpParserState::HEADER;
        }
        if (state == HttpParserState::HEADER) {
            state = parseHeaderPart(data, pos, request);
            if (state == HttpParserState::HEADER_END) {
                if (!request.ContentLength().empty() && request.ContentLength() != "0")
                    state = HttpParserState::CONTENT_START;
                else
                    state = HttpParserState::END;
            }
        }

        if (state == HttpParserState::CONTENT_START
            && request.ContentType() == "application/x-www-form-urlencoded") {
            gotoNextNonWhite(data, pos);
            while (pos != data.end()) {
                const auto key = goTo(data, "=", pos);
                const auto value = goToOne(data, " &", ++pos);
                request.AddParameter(key, value);
                if (pos != data.end())
                    ++pos;
            }
            state = HttpParserState::END;
        }


        while (pos != data.end() && (state == HttpParserState::CONTENT_DATA
                                     || state == HttpParserState::CONTENT_START
                                     || state == HttpParserState::CONTENT_HEADER
                                     || state == HttpParserState::CONTENT_HEADERFIELD)) {
            if (state == HttpParserState::CONTENT_START) {
                size_t ctPos = request.ContentType().find("multipart/form-data;");
                if (ctPos == std::string::npos) {
                    goToNextLine(data, pos);
                    request.AddParameter("$Body", {pos, data.end()});
                    if (request.GetBody().length() == std::stoull(request.ContentLength())) {
                        state = HttpParserState::END;
                    } else {
                        state = HttpParserState::CONTENT_START;
                    }
                    return state;
                }
                FlowParser::goTo(request.ContentType(), "=", ctPos);
                string boundary = FlowParser::goToEnd(request.ContentType(), ++ctPos);
                request.Boundary = boundary;


                goToNewLine(data, pos);
                goToNextLine(data, pos);
                state = HttpParserState::CONTENT_HEADER;
            }

            if (state == HttpParserState::CONTENT_HEADER && pos != data.end()) {
                goTo(data, request.Boundary, pos);
                if (pos == data.end())
                    return state;

                state = HttpParserState::CONTENT_HEADERFIELD;
            }
            if (state == HttpParserState::CONTENT_HEADERFIELD && pos != data.end()) {
                auto savePos = pos;
                goToNewLine(data, pos);
                goToNextLine(data, pos);

                if (!hasNextDoubleNewLine(data, pos)) {
                    prefBuffer.assign(savePos, data.end());
                    return state;
                }

                MulPaField &field = request.fields.emplace_back();
                parseFieldHeaderPart(data, pos, field);
                state = HttpParserState::CONTENT_DATA;
                if (pos != data.end())
                    ++pos;
            }

            if (state == HttpParserState::CONTENT_DATA && pos != data.end()) {
                auto start = pos;
                goTo(data, request.Boundary, pos);
                MulPaField &field = request.fields.back();
                if (pos == data.end()) {
                    pos = pos - request.Boundary.size() - 3;
                    goTo(data, "\n--", pos);

                    if (pos != data.end()) {
                        auto toTestPos = pos;
                        auto dataEnding = FlowVParser::findLastData(pos);
                        prefBuffer.assign(dataEnding + 1, data.end());
                        pos = dataEnding;
                    }


                    if (request.ParseToDisk) {
                        writeToDisk(request, field, start, pos);
                    } else {
                        copy(start, pos, back_inserter(field.data));
                    }
                    return state;
                } else {
                    auto dataEnding = FlowVParser::findLastData(pos, data.begin());
                    if (request.ParseToDisk) {
                        writeToDisk(request, field, start, dataEnding);
                    } else {
                        copy(start, dataEnding, back_inserter(field.data));
                    }
                    pos += request.Boundary.size();
                    if (*pos == '-' && *++pos == '-' &&
                        ((*++pos == '\r' && *++pos == '\n') || *++pos == '\n') &&
                        ++pos == data.end())

                        state = HttpParserState::END;
                    else
                        state = HttpParserState::CONTENT_HEADERFIELD;

                    field.CloseFile();
                }
            }
        }

        return state;
    }

    HttpParserState
    parseHeaderPart(std::vector<unsigned char> &data, std::vector<unsigned char>::iterator &pos,
                    Request &request) {
        while (pos != data.end()) {
            if (FlowVParser::isDoubleNewLine(pos)) {
                return HttpParserState::HEADER_END;
            }
            if (pos == data.end()) break;
            const auto key = FlowVParser::goTo(data, ":", pos);
            FlowVParser::gotoNextNonWhite(data, ++pos);
            const auto value = FlowVParser::goToNewLine(data, pos);
            request[key] = value;
        }

        return HttpParserState::HEADER;
    }

    void
    parseFieldHeaderPart(std::vector<unsigned char> &data, std::vector<unsigned char>::iterator &pos,
                         MulPaField &field) {
        while (pos != data.end() && !FlowVParser::isDoubleNewLine(pos)) {
            if (pos == data.end()) break;
            const auto key = FlowVParser::goTo(data, ":", pos);
            FlowVParser::gotoNextNonWhite(data, ++pos);
            const auto value = FlowVParser::goToNewLine(data, pos);
            field[key] = value;
        }
    }

    bool
    hasNextDoubleNewLine(std::vector<unsigned char> &data, std::vector<unsigned char>::iterator pos) {

        const std::string wnl("\r\n\r\n");
        auto toCheck = std::search(pos, data.end(), wnl.begin(), wnl.end());
        if (toCheck == data.end()) {
            const std::string unl("\n\n");
            toCheck = std::search(pos, data.end(), unl.begin(), unl.end());
        }

        return toCheck != data.end();
    }

    void
    writeToDisk(const Request &request, MulPaField &field, std::vector<unsigned char>::iterator &start,
                std::vector<unsigned char>::iterator &end) {

        std::ofstream *file;
        if (field.FileIsOpen()) {
            file = field.File();
        } else {
            file = field.OpenIn(request.WriteFolder, FileNamingFunction);
            if (!field.data.empty()) {
                std::ostream_iterator<unsigned char> ofitr(*file);
                copy(field.data.begin(), field.data.end(), ofitr);
                field.data.clear();
            }
        }

        std::ostream_iterator<unsigned char> ofitr(*file);
        copy(start, end, ofitr);
    }

};