#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include "Response.h"
#include <vector>
#include "Socket.h"
#include <FlowUtils/FlowLog.h>

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

    inline bool sendOk(Socket &socket) {
        Response res;
        return write(socket, res.ToString());
    }

    inline bool sendOk(Socket &socket, const Response& res) {
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

    inline void sendFound(Response &response, Socket &socket, const std::string location) {
        response.StatusCode = HttpStatusCode::Found;
        response.emplace("Location", location);
        write(socket, response.ToString());
    }

    inline void sendUnauthorized(Socket &socket) {
        Response response(socket.IsSSL());
        response.StatusCode = HttpStatusCode::Unauthorized;
        write(socket, response.ToString());
    }

    inline void sendNotFound(Socket &socket) {
        Response response(socket.IsSSL());
        response.StatusCode = HttpStatusCode::NotFound;
        write(socket, response.ToString());
    }

    inline void sendBadRequest(Socket &socket) {
        Response response(socket.IsSSL());
        response.StatusCode = HttpStatusCode::BadRequest;
        write(socket, response.ToString());
    }
}