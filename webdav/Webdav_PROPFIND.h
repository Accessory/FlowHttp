#pragma once

#include "../routes/Route.h"
#include <FlowUtils/FlowLog.h>
#include <string>
#include <boost/asio.hpp>
#include "../Socket.h"
#include <tinyxml2.h>
#include <sstream>
#include <utility>
#include <FlowUtils/FlowFile.h>
#include "../MimeTypes.h"
#include <FlowUtils/UrlEscape.h>

class Webdav_PROPFIND : public Route {
public:
    explicit Webdav_PROPFIND(std::string basePath) : basePath(std::move(basePath)), Route(".*", "PROPFIND") {
    }

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace tinyxml2;

        XMLDocument document;
        if (request.ParserState == HttpParserState::BAD_REQUEST ||
            document.Parse(request.GetBody().c_str()) != XML_SUCCESS) {
            return FlowAsio::sendBadRequest(socket);
        }

        const auto path = request.Path();
        const auto depth = request.Header("Depth");

        XMLDocument rtn;
        createMultistatusXml(rtn, document, path, depth);

        XMLPrinter printer;
        rtn.Print(&printer);
        LOG_TRACE << printer.CStr();
        std::stringstream dataStream;
        dataStream << HEADER << printer.CStr();
        response.Data(dataStream.str());
//        LOG_INFO << dataStream.str();

        return FlowAsio::sendMultiStatus(socket, response);
    }

private:
    tinyxml2::XMLElement *
    createProps(tinyxml2::XMLDocument &response, const tinyxml2::XMLElement *props, std::string path) {
        using namespace tinyxml2;
        XMLElement *prop = response.NewElement("D:prop");
        for (const XMLElement *child = props->FirstChildElement(); child != NULL; child = child->NextSiblingElement()) {
            const auto split = FlowString::splitOnFirst(child->Name(), ":");
            const auto child_name = split.size() == 2 ? split.at(1).c_str() : split.at(0).c_str();
            const auto is_directory = FlowFile::isDirectory(path);
            const auto is_File = !is_directory;
            std::string child_full_name("D:");
            child_full_name += child_name;

            if (!FlowFile::fileExist(path)) {
                UrlEscape::UrlDecode(path);
            }

            if (is_File && std::strcmp(child_name, "displayname") == 0) {
                XMLElement *new_prop = response.NewElement(child_full_name.c_str());
                const auto file_name = FlowFile::getFilename(path);
                new_prop->SetText(file_name.c_str());
                prop->InsertEndChild(new_prop);
            } else if (is_File && std::strcmp(child_name, "getcontenttype") == 0) {
                XMLElement *new_prop = response.NewElement(child_full_name.c_str());
                const auto extension = FlowFile::getFileExtension(path);
                const auto mime = MimeTypes::extension_to_type(extension);
                new_prop->SetText(mime.c_str());
                prop->InsertEndChild(new_prop);
            } else if (is_File && std::strcmp(child_name, "getcontentlength") == 0) {
                XMLElement *new_prop = response.NewElement(child_full_name.c_str());
                const auto size = FlowFile::getFileSize(path);
                new_prop->SetText(size);
                prop->InsertEndChild(new_prop);
            } else if (std::strcmp(child_name, "getlastmodified") == 0) {
                XMLElement *new_prop = response.NewElement(child_full_name.c_str());
                const auto last_modified = FlowFile::getLastModified(path);
                std::stringstream lastModified;
                lastModified << std::put_time(std::gmtime(&last_modified), "%a, %d %b %Y %T GMT");
                new_prop->SetText(lastModified.str().c_str());
//                new_prop->SetText("Mon, 12 Jan 1998 09:25:56 GMT");
                prop->InsertEndChild(new_prop);
            } else if (std::strcmp(child_name, "resourcetype") == 0) {
                XMLElement *new_prop = response.NewElement(child_full_name.c_str());
                prop->InsertEndChild(new_prop);
                if (is_directory) {
                    XMLElement *collection = response.NewElement("D:collection");
                    new_prop->InsertEndChild(collection);
                }
            } else if (std::strcmp(child_name, "lockdiscovery") == 0) {
                XMLElement *new_prop = response.NewElement(child_full_name.c_str());
                prop->InsertEndChild(new_prop);
            } else if (std::strcmp(child_name, "supportedlock") == 0) {
                XMLElement *new_prop = response.NewElement(child_full_name.c_str());
                prop->InsertEndChild(new_prop);
            } else {
                XMLElement *new_prop = response.NewElement(child_full_name.c_str());
                prop->InsertEndChild(new_prop);
            }
        }
        return prop;
    }

    tinyxml2::XMLElement *
    createResponse(tinyxml2::XMLDocument &response, const tinyxml2::XMLElement *request, const std::string &path) {
        using namespace tinyxml2;
        XMLElement *xmlResponse = response.NewElement("D:response");
        XMLElement *href = response.NewElement("D:href");
        auto href_path = path.substr(basePath.size());
        if (FlowFile::isDirectory(path) && path.at(path.size() - 1) != '/')
            href_path += '/';
        href->SetText(href_path.c_str());
        XMLElement *propstat = response.NewElement("D:propstat");

        XMLElement *prop = createProps(response, request->FirstChildElement(), path);

        XMLElement *status = response.NewElement("D:status");
        status->SetText("HTTP/1.1 200 OK");
        propstat->InsertEndChild(prop);
        propstat->InsertEndChild(status);
        xmlResponse->InsertEndChild(href);
        xmlResponse->InsertEndChild(propstat);
        return xmlResponse;
    }

    void createMultistatusXml(tinyxml2::XMLDocument &response, const tinyxml2::XMLDocument &request,
                              const std::string &path, const std::string depth = "1") {
        using namespace tinyxml2;
//        const auto response_namespace = request.NextSiblingElement("xmlns");
        XMLElement *root = response.NewElement("D:multistatus");
        root->SetAttribute("xmlns:D", "DAV:");
        response.InsertFirstChild(root);
        const auto request_path = FlowFile::combinePath(basePath, path);
        if (depth == "1") {
            const auto content = FlowFile::getContentOfDirectory(request_path);
            for (const auto &item : content) {
                XMLElement *xmlResponse = createResponse(response, request.FirstChildElement(),
                                                         item);
                root->InsertFirstChild(xmlResponse);
            }
        } else if (depth == "0") {
            XMLElement *xmlResponse = createResponse(response, request.FirstChildElement(),
                                                     request_path);
            root->InsertFirstChild(xmlResponse);
        }
    }

    const static inline char *HEADER = "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n";
    std::string basePath;
};