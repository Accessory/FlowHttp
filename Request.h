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

class Request : public std::map<std::string, std::string> {
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

    void Path(const std::string &version) {
        this->operator[]("Path") = version;
    }

    void AddParameter(const std::string &key, const std::string &value) {
        Parameter[key] = value;
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

    Request &operator<<(vector<unsigned char> &data) {
        ParserState = parseRequest(data, *this, ParserState);
        return *this;
    }

    std::string ToString() {
        std::stringstream ss;
        ss << this->Method() << " " << this->Path() << " " << this->Protocol() << "/" << this->HttpVersion();
        for (auto item : *this) {
            ss << endl << item.first << " " << item.second;
        }
        return ss.str();
    }

    Request() {
        ParserState = HttpParserState::START;
    }

    void CloseFiles() {
        for (auto &field : fields)
            field.CloseFile();
    }


    std::map<std::string, std::string> Parameter;
    std::string Boundary;
    std::vector<MulPaField> fields;
    HttpParserState ParserState;
    bool ParseToDisk = false;
    std::string WriteFolder;

private:

    vector<unsigned char> prefBuffer;

//    HttpParserState parseRequest(const string &data, Request &request, HttpParserState state) {
//        using namespace FlowParser;
//        size_t pos = 0;
//        if (state == HttpParserState::START) {
//            string method = gotoNextNonAlpha(data, pos);
//            FlowString::toUpper(method);
//            request.Method(method);
//            if (pos >= data.size() || method.empty()) {
//                state = HttpParserState::BAD_REQUEST;
//                return state;
//            }
//            string test = gotoNextNonWhite(data, pos);
//            string path = goToOne(data, " ?", pos);
//            request.Path(path);
//            while (data.at(pos) == '?' || data.at(pos) == '&') {
//                string key = goTo(data, "=", ++pos);
//                string value = goToOne(data, " &", ++pos);
//                request.AddParameter(key, value);
//            }
//            gotoNextNonWhite(data, pos);
//            string protocol = goTo(data, "/", pos);
//            request.Protocol(protocol);
//            string httpVersion = goToNewLine(data, ++pos);
//            request.HttpVersion(httpVersion);
//
//            if (request.Method() == "PRI" && httpVersion == "2.0") {
//                state = parseHTML2(data, pos, request);
//            } else {
//                parseHeaderPart(data, pos, request);
//            }
//
//
////        LOG_INFO << request.Method();
//            if (request.Method() == "POST")
//                state = HttpParserState::CONTENT_START;
//        }
//
//        if (state == HttpParserState::CONTENT_START
//            && request.ContentType() == "application/x-www-form-urlencoded") {
//            gotoNextNonWhite(data, pos);
//            while (pos != std::string::npos) {
//                string key = goTo(data, "=", pos);
//                if (pos == string::npos) {
//                    state = HttpParserState::BAD_REQUEST;
//                    return state;
//                }
//                string value = goToOne(data, " &", ++pos);
//                request.AddParameter(key, value);
//                if (data[pos] == '&')
//                    ++pos;
//            }
//            state = HttpParserState::END;
//        }
//
//        if (state == HttpParserState::CONTENT_START
//            && request.ContentType().find("multipart/form-data;") == std::string::npos) {
//            gotoNextNonWhite(data, pos);
//            if (pos < data.size()) {
//                const std::string key = "body";
//                const std::string value = data.substr(pos);
//                request.AddParameter(key, value);
//            }
//            state = HttpParserState::END;
//        }
//
//        while (pos < data.size()
//               && (state == HttpParserState::CONTENT_DATA
//                   || state == HttpParserState::CONTENT_START
//                   || state == HttpParserState::CONTENT_HEADER
//                   || state == HttpParserState::CONTENT_HEADERFIELD)) {
//            if (state == HttpParserState::CONTENT_START) {
//                size_t ctPos = request.ContentType().find("multipart/form-data;");
//                if (ctPos == std::string::npos) {
//                    state = HttpParserState::END;
//                    return state;
//                }
//                FlowParser::goTo(request.ContentType(), "=", ctPos);
//                string boundary = FlowParser::goToEnd(request.ContentType(), ++ctPos);
//                request.Boundary = boundary;
//
//                goToNewLine(data, pos);
//                goToNextLine(data, pos);
//                state = HttpParserState::CONTENT_HEADER;
//            }
//
//            if (state == HttpParserState::CONTENT_HEADER) {
//                goTo(data, request.Boundary, pos);
//                if (pos == std::string::npos)
//                    return state;
//                state = HttpParserState::CONTENT_HEADERFIELD;
//            }
//            if (state == HttpParserState::CONTENT_HEADERFIELD && pos < data.size()) {
//                goToNewLine(data, pos);
//                goToNextLine(data, pos);
//
//                MulPaField &field = request.fields.emplace_back();
//                parseFieldHeaderPart(data, pos, field);
//                state = HttpParserState::CONTENT_DATA;
//                ++pos;
//            }
//
//            if (state == HttpParserState::CONTENT_DATA && pos < data.size()) {
//                auto start = pos;
//                goTo(data, request.Boundary, pos);
//                MulPaField &field = request.fields.back();
//                if (pos >= data.size()) {
//                    copy(data.begin() + start, data.end(), back_inserter(field.data));
//                    return state;
//                } else {
//                    auto dataEnding = findLastData(data, pos);
//                    copy(data.begin() + start, data.begin() + dataEnding, back_inserter(field.data));
//                    pos += request.Boundary.size();
//                    if (data.at(pos) == '-' && data.at(++pos) == '-' &&
//                        ((data.at(++pos) == '\r' && data.at(++pos) == '\n') || data.at(++pos) == '\n') &&
//                        ++pos >= data.size())
//
//                        state = HttpParserState::END;
//                    else
//                        state = HttpParserState::CONTENT_HEADERFIELD;
//
//                    field.CloseFile();
//                }
//            }
//        }
//        if (request.ParseToDisk)
//            request.CloseFiles();
////        state = HttpParserState::END;
//        return state;
//    }

    void parseFieldHeaderPart(const string &req, size_t &pos, MulPaField &field) {
        while (pos < req.size() && !FlowParser::isDoubleNewLine(req, pos)) {
            if (pos >= req.size()) break;
            string key = FlowParser::goTo(req, ":", pos);
            FlowParser::gotoNextNonWhite(req, ++pos);
            string value = FlowParser::goToNewLine(req, pos);
            field[key] = value;
        }
    }

    HttpParserState parseHTML2(const string &req, size_t &pos, Request &request) {
        const string HTML2_PRI_CHECK = "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n";
        if (req.substr(0, HTML2_PRI_CHECK.length()) != HTML2_PRI_CHECK) {
            return HttpParserState::BAD_REQUEST;
        }
        request.AddParameter("data", req.substr(HTML2_PRI_CHECK.length()));
        return HttpParserState::END;
    }

    void parseHeaderPart(const string &req, size_t &pos, Request &request) {
        while (pos < req.size() && !FlowParser::isDoubleNewLine(req, pos)) {
            if (pos >= req.size()) break;
            string key = FlowParser::goTo(req, ":", pos);
            FlowParser::gotoNextNonWhite(req, ++pos);
            string value = FlowParser::goToNewLine(req, pos);
            request[key] = value;
        }
    }

    HttpParserState
    parseRequest(vector<unsigned char> &data, Request &request, HttpParserState state) {
        using namespace FlowVParser;

        if (!prefBuffer.empty()) {
            data.insert(data.begin(), prefBuffer.begin(), prefBuffer.end());
            prefBuffer.clear();
        }

        vector<unsigned char>::iterator pos = data.begin();
        if (state == HttpParserState::START) {

            if(pos == data.end() || !isOneOf(pos, {"GET","POST", "PUT", "PATCH", "DELETE", "OPTION", "HEAD", "CONNECT", "TRACE"})){
                state = HttpParserState::BAD_REQUEST;
                return state;
            }

            string method = gotoNextNonAlpha(data, pos);
            FlowString::toUpper(method);
            request.Method(method);
            if (pos == data.end() || method.empty()) {
                state = HttpParserState::END;
                return state;
            }
            string test = gotoNextNonWhite(data, pos);
            string path = goToOne(data, " ?", pos);
            request.Path(path);
            while (*pos == '?' || *pos == '&') {
                string key = goTo(data, "=", ++pos);
                string value = goToOne(data, " &", ++pos);
                request.AddParameter(key, value);
            }
            gotoNextNonWhite(data, pos);
            string protocol = goTo(data, "/", pos);
            request.Protocol(protocol);
            string httpVersion = goToNewLine(data, ++pos);
            request.HttpVersion(httpVersion);
            parseHeaderPart(data, pos, request);

            if (request.Method() == "POST")
                state = HttpParserState::CONTENT_START;
        }

        if (state == HttpParserState::CONTENT_START
            && request.ContentType() == "application/x-www-form-urlencoded") {
            gotoNextNonWhite(data, pos);
            while (pos != data.end()) {
                string key = goTo(data, "=", pos);
                string value = goToOne(data, " &", ++pos);
                request.AddParameter(key, value);
                if(pos != data.end())
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
                    request.AddParameter("body", {pos, data.end()});
                    state = HttpParserState::END;
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
                    auto dataEnding = FlowVParser::findLastData(pos);
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

    void
    parseHeaderPart(vector<unsigned char> &data, vector<unsigned char>::iterator &pos,
                    Request &request) {
        while (pos != data.end() && !FlowVParser::isDoubleNewLine(pos)) {
            if (pos == data.end()) break;
            string key = FlowVParser::goTo(data, ":", pos);
            FlowVParser::gotoNextNonWhite(data, ++pos);
            string value = FlowVParser::goToNewLine(data, pos);
            request[key] = value;
        }
    }

    void
    parseFieldHeaderPart(vector<unsigned char> &data, vector<unsigned char>::iterator &pos,
                         MulPaField &field) {
        while (pos != data.end() && !FlowVParser::isDoubleNewLine(pos)) {
            if (pos == data.end()) break;
            string key = FlowVParser::goTo(data, ":", pos);
            FlowVParser::gotoNextNonWhite(data, ++pos);
            string value = FlowVParser::goToNewLine(data, pos);
            field[key] = value;
        }
    }

    bool
    hasNextDoubleNewLine(vector<unsigned char> &data, vector<unsigned char>::iterator pos) {

        const std::string wnl("\r\n\r\n");
        auto toCheck = std::search(pos, data.end(), wnl.begin(), wnl.end());
        if (toCheck == data.end()) {
            const std::string unl("\n\n");
            toCheck = std::search(pos, data.end(), unl.begin(), unl.end());
        }

        return toCheck != data.end();
    }

    void
    writeToDisk(const Request &request, MulPaField &field, vector<unsigned char>::iterator &start,
                vector<unsigned char>::iterator &end) {

        ofstream *file;
        if (field.FileIsOpen()) {
            file = field.File();
        } else {
            file = field.OpenIn(request.WriteFolder, FileNamingFunction);
            if (!field.data.empty()) {
                ostream_iterator<unsigned char> ofitr(*file);
                copy(field.data.begin(), field.data.end(), ofitr);
                field.data.clear();
            }
        }

        ostream_iterator<unsigned char> ofitr(*file);
        copy(start, end, ofitr);
    }

};