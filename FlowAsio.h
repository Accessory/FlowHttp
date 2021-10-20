#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "Response.h"
#include <vector>
#include "Socket.h"
#include <FlowUtils/FlowLog.h>
#include <FlowUtils/Semaphore.h>
#include "Request.h"

namespace FlowAsio {

    inline std::string readStringSSL(Socket &socket) {
        std::stringstream allDataBuffer;
        boost::system::error_code error;
        std::array<char, 8192> caBuf;
        do {
            size_t readLength = boost::asio::read(*socket.GetSSLSocket(),
                                                  boost::asio::buffer(caBuf),
                                                  boost::asio::transfer_at_least(1), error);

            allDataBuffer.write(caBuf.data(), readLength);
        } while (error != boost::asio::error::eof && socket.GetSSLSocket()->lowest_layer().available());
        return allDataBuffer.str();
    }

    inline std::string readString(Socket &socket) {
        std::stringstream allDataBuffer;
        boost::system::error_code error;
        std::array<char, 8192> caBuf;
        do {
            size_t readLength = boost::asio::read(*socket.GetSocket(),
                                                  boost::asio::buffer(caBuf),
                                                  boost::asio::transfer_at_least(1), error);

            allDataBuffer.write(caBuf.data(), readLength);
        } while (error != boost::asio::error::eof && socket.GetSocket()->lowest_layer().available());
        return allDataBuffer.str();
    }

    inline std::vector<unsigned char> readBytesSSL(Socket &socket) {
        std::vector<unsigned char> allDataBuffer;
        boost::system::error_code error;
        std::array<char, 8192> caBuf;
        do {
            size_t readLength = boost::asio::read(*socket.GetSSLSocket(),
                                                  boost::asio::buffer(caBuf),
                                                  boost::asio::transfer_at_least(1), error);

            allDataBuffer.insert(allDataBuffer.end(), caBuf.begin(), caBuf.begin() + readLength);
        } while (error != boost::asio::error::eof && socket.GetSSLSocket()->lowest_layer().available());
        return allDataBuffer;
    }

    inline std::vector<unsigned char> readBytesNoSSL(Socket &socket) {
        std::vector<unsigned char> allDataBuffer;
        boost::system::error_code error;
        std::array<char, 8192> caBuf;
        do {
            size_t readLength = boost::asio::read(*socket.GetSocket(),
                                                  boost::asio::buffer(caBuf),
                                                  boost::asio::transfer_at_least(1), error);

            allDataBuffer.insert(allDataBuffer.end(), caBuf.begin(), caBuf.begin() + readLength);
        } while (error != boost::asio::error::eof && socket.GetSocket()->lowest_layer().available());
        return allDataBuffer;
    }

    inline std::string read(Socket &socket) {
        if (socket.IsSSL())
            return readStringSSL(socket);
        else
            return readString(socket);
    }

    inline std::vector<unsigned char> readBytes(Socket &socket) {
        if (socket.IsSSL())
            return readBytesSSL(socket);
        else
            return readBytesNoSSL(socket);
    }

    inline bool writeStringSSL(Socket &socket, const std::string &data) {
        try {
            boost::asio::write(*socket.GetSSLSocket(), boost::asio::buffer(data));
        } catch (std::exception &ex) {
            LOG_WARNING << ex.what();
            return false;
        }
        return true;
    }

    inline bool writeString(Socket &socket, const std::string &data) {
        try {
            boost::asio::write(*socket.GetSocket(), boost::asio::buffer(data));
        } catch (std::exception &ex) {
            LOG_WARNING << ex.what();
            return false;
        }
        return true;
    }


    inline bool write(Socket &socket, const std::string &data) {
        if (socket.IsSSL())
            return writeStringSSL(socket, data);
        else
            return writeString(socket, data);
    }

    inline bool asioWrite(boost::asio::ip::tcp::socket &soc, const std::string &data) {
        try {
            boost::asio::write(soc, boost::asio::buffer(data));
        } catch (std::exception &ex) {
            LOG_WARNING << ex.what();
            return false;
        }
        return true;
    }

    inline bool write(Socket &socket, const unsigned char *data, const size_t &size) {
        try {
            if (socket.IsSSL())
                boost::asio::write(*socket.GetSSLSocket(), boost::asio::buffer(data, size));
            else
                boost::asio::write(*socket.GetSocket(), boost::asio::buffer(data, size));
        } catch (std::exception &ex) {
            LOG_WARNING << ex.what();
            return false;
        }
        return true;
    }

    inline bool write(Socket &socket, const std::vector<unsigned char> &data) {
        return write(socket, data.data(), data.size());
    }

    inline bool sendMultiStatus(Socket &socket, Response &res) {
        res.StatusCode = HttpStatusCode::MultiStatus;
        return write(socket, res.ToString());
    }

    inline bool sendOk(Socket &socket) {
        Response res;
        return write(socket, res.ToString());
    }

    inline bool sendOk(Socket &socket, Response &res) {
        res.StatusCode = HttpStatusCode::OK;
        return write(socket, res.ToString());
    }

    inline bool send(Socket &socket, const Response &res) {
        return write(socket, res.ToString());
    }

    inline std::size_t readToVector(Socket &socket, std::vector<unsigned char> &buffer,
                                    boost::system::error_code &error,
                                    std::size_t bufferSize) {
        try {
            buffer.clear();
            buffer.resize(bufferSize);
            if (socket.IsSSL())
                return boost::asio::read(*socket.GetSSLSocket(),
                                         boost::asio::buffer(buffer),
                                         boost::asio::transfer_at_least(1),
                                         error);
            else
                return boost::asio::read(*socket.GetSocket(),
                                         boost::asio::buffer(buffer),
                                         boost::asio::transfer_at_least(1),
                                         error);
        } catch (std::exception ex) {
            LOG_WARNING << ex.what();
        }
        return 0;
    }

    inline void redirect(const std::string &string, Socket &socket) {
        Response response(socket.IsSSL());
        response.StatusCode = HttpStatusCode::MovedPermanently;
        response.emplace("Location", string);
        write(socket, response.ToString());
    }

    inline void sendFound(Response &response, Socket &socket, const std::string &location) {
        response.StatusCode = HttpStatusCode::Found;
        response.emplace("Location", location);
        write(socket, response.ToString());
    }

    inline void sendUnauthorized(Socket &socket) {
        Response response(socket.IsSSL());
        response.StatusCode = HttpStatusCode::Unauthorized;
        write(socket, response.ToString());
    }

    inline bool sendNotFound(Socket &socket) {
        Response response(socket.IsSSL());
        response.StatusCode = HttpStatusCode::NotFound;
        return write(socket, response.ToString());
    }

    inline bool sendBadRequest(Socket &socket) {
        Response response;
        response.StatusCode = HttpStatusCode::BadRequest;
        return write(socket, response.ToString());
    }

    inline bool sendBadRequest(Socket &socket, Response &response) {
        response.StatusCode = HttpStatusCode::BadRequest;
        return write(socket, response.ToString());
    }

    inline bool sendMethodNotAllowed(Socket &socket) {
        Response response(socket.IsSSL());
        response.StatusCode = HttpStatusCode::MethodNotAllowed;
        return write(socket, response.ToString());
    }

    inline bool
    sendFile(const std::string &fileName,
             const std::string &path,
             const std::string &mime,
             Request &request, Response &response, Socket &socket,
             bool isDownload = true) {
        const static size_t BUFFER_SIZE = 128 * 1024;

        if (isDownload)
            response["Content-Disposition"] = "attachment; filename=" + fileName;


        response.ContentType = mime;
        response.SetFile(path);

        //Range
        response["Accept-Ranges"] = "bytes";
        if (request.count("Range")) {
            if (!response.Range(request.at("Range"))) {
                request.CloseConnection();
                FlowAsio::sendBadRequest(socket);
                return true;
            }
        }
        unsigned char data1[BUFFER_SIZE];
        unsigned char data2[BUFFER_SIZE];
        unsigned char *data = data1;
        bool dataSwitch = false;

        Semaphore m;

        const std::string header = response.Header();
        m.lock();
        std::thread([&] {
            FlowAsio::write(socket, header);
            m.unlock();
        }).detach();
        size_t readBytes = 0;
        bool hasError = false;
        size_t lock_count = 0;
        while ((readBytes = response.ReadData(data, BUFFER_SIZE)) && !hasError) {
            m.lock();
            std::thread([&, data, readBytes] {
                if (!FlowAsio::write(socket, data, readBytes)) {
                    hasError = true;
                    request.CloseConnection();
                    m.unlock();
                    return false;
                }
                m.unlock();
                return true;
            }).detach();
            data = dataSwitch ? data1 : data2;
            dataSwitch = !dataSwitch;
        }
        m.lock();
        request.CloseConnection();
        return true;
    }

    inline Request readRequest(Socket &socket) {
        Request request;
        while (request.ParserState <= HttpParserState::HEADER_END && request.ParserState != HttpParserState::END &&
               request.ParserState != HttpParserState::BAD_REQUEST) {
            auto req = FlowAsio::readBytes(socket);
//            LOG_DEBUG << "Request:" << std::endl << std::string(req.begin(), req.end());
            request << req;
        }
        return request;
    }

    inline Socket moveSocketToSSLSocket(boost::asio::ip::tcp::socket& socket, boost::asio::ssl::context &ssl_context) {
        Socket rtn(std::move(socket));
        boost::system::error_code error;
        rtn.SetSSL(ssl_context);
        rtn.GetSSLSocket()->handshake(boost::asio::ssl::stream_base::server, error);
        if (error) {
            LOG_WARNING << "Bad Request";
        }
        return rtn;
    }

    inline Socket moveSocketToSocket(boost::asio::ip::tcp::socket& socket) {
        Socket rtn(std::move(socket));
        return rtn;
    }

    inline std::string getBody(Request& request, Socket& socket){
        while (request.ParserState >= HttpParserState::HEADER_END && request.ParserState != HttpParserState::END &&
               request.ParserState != HttpParserState::BAD_REQUEST) {
            auto req = FlowAsio::readBytes(socket);
//            LOG_DEBUG << "Request:" << std::endl << std::string(req.begin(), req.end());
            request << req;
        }

        if(request.GetBody().length() != std::stoull(request.ContentLength())){
            LOG_WARNING << "Bad Request after reading all!";
            request.ParserState = HttpParserState::BAD_REQUEST;
        }

        return request.GetBody();
    }
}