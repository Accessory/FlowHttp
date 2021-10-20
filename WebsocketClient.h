#pragma once

#include <memory>
#include <FlowUtils/FlowLog.h>
#include "util/WebSocketUtil.h"
#include <iostream>
#include <string>
#include <regex>
#include <boost/asio.hpp>
#include "WebSocketFrame.h"
#include <atomic>
#include <thread>
#include <FlowUtils/Semaphore.h>


class WebsocketClient {
public:
    void connect(const std::string &url) {
        std::regex rgx("ws://([^:]+):(\\d+)(.*)");
        std::smatch m;
        if (std::regex_match(url, m, rgx)) {
            const auto &address = m[1];
            const auto &port = m[2];
            const auto &path = m[3];
            connect(address, port, path);
        } else {
            LOG_WARNING << "Connection Failed: " << "URL is mallformed";
        }
    }

    void setReadCallback(std::shared_ptr<std::function<void(WebSocketFrame)>> readCallback) {
        this->readCallback = readCallback;
    }

    void close() {
        io_context.stop();
        read_thread->detach();
        thread_mutex.unlock();
        read_in_thread = false;
        connected = false;
        socket->close();
    }

    void join() {
        std::lock_guard<std::mutex> lockGuard(thread_mutex);
    }

    void startReadingThread() {
        read_in_thread = true;
        read_thread = std::make_unique<std::thread>([&] {
            std::lock_guard<std::mutex> lockGuard(thread_mutex);
            while (read_in_thread && connected) {
                const auto data = readAsioRaw();
                size_t buffer_pos = 0;
                while(buffer_pos < data.size()) {
                    auto rtn = WebSocketFrame::readMessage(data, buffer_pos);
                    if(readCallback) {
                        readCallback->operator()(std::move(rtn));
                    }
                }
            }
        });
    }

    void connect(const std::string &address, const std::string &port, const std::string &path) {
        socket = std::make_unique<boost::asio::ip::tcp::socket>(io_context);

        boost::asio::ip::tcp::resolver::query query(address, port);

        boost::asio::ip::tcp::resolver resolver(io_context);
        const auto resolverResult = resolver.resolve(query);

        boost::asio::connect(*socket.get(), resolverResult);
        const auto key = WebSocketUtil::createSecWebSocketKey();

        const std::string request = "GET " + path + " HTTP/1.1\r\n"
                                                    "Host: " + address +
                                    ":" + port +
                                    "\r\n"
                                    "Upgrade: websocket\r\n"
                                    "Connection: Upgrade\r\n"
                                    "Sec-WebSocket-Key: " + key +
                                    "\r\n"
                                    "Sec-WebSocket-Version: 13\r\n\r\n";

        writeAsio(request);
        const auto result = readAsio();
        LOG_INFO << std::endl << result;
        connected = true;
    }

    bool isConnected() {
        return connected;
    }

    void send(const std::string &msg) {
        WebSocketFrame frame;
        frame.payload_data.assign(msg.begin(), msg.end());
        const auto data = frame.createMaskedData();
        writeAsio(data);
    }

    WebsocketClient() {
        if (socket != nullptr) {
            socket->close();
        }
    }

private:

    void writeAsio(boost::asio::ip::tcp::socket &socket, const std::vector<unsigned char> &data) {
        boost::system::error_code error;
        boost::asio::write(socket, boost::asio::buffer(data), error);
        if (error.failed()) {
            LOG_FATAL << error.message();
            read_in_thread = false;
        }
    }

    std::string readAsio() {
        boost::system::error_code error;
        std::vector<unsigned char> allDataBuffer;
        std::array<char, 8192> caBuf{};
        do {
            size_t readLength = boost::asio::read(*socket.get(),
                                                  boost::asio::buffer(caBuf),
                                                  boost::asio::transfer_at_least(1), error);

            allDataBuffer.insert(allDataBuffer.end(), caBuf.begin(), caBuf.begin() + readLength);
        } while (error != boost::asio::error::eof && socket->lowest_layer().available());
        if (error.failed()) {
            LOG_FATAL << error.message();
            connected = false;
        }
        return std::string(allDataBuffer.begin(), allDataBuffer.end());
    }

    std::vector<unsigned char> readAsioRaw() {
        boost::system::error_code error;
        std::vector<unsigned char> allDataBuffer;
        std::array<char, 8192> caBuf{};
        do {
            size_t readLength = boost::asio::read(*socket.get(),
                                                  boost::asio::buffer(caBuf),
                                                  boost::asio::transfer_at_least(1), error);
//            LOG_INFO << std::string(caBuf.begin(), caBuf.begin() + readLength);
            allDataBuffer.insert(allDataBuffer.end(), caBuf.begin(), caBuf.begin() + readLength);
        } while (error != boost::asio::error::eof && socket->lowest_layer().available());
        if (error.failed()) {
            LOG_FATAL << error.message();
            connected = false;
        }
        return allDataBuffer;
    }

    void writeAsio(const std::string &data) {
        boost::system::error_code error;
        boost::asio::write(*socket.get(), boost::asio::buffer(data), error);
        if (error) {
            LOG_FATAL << error.message();
            connected = false;
        }
    }

    void writeAsio(const std::vector<unsigned char> &data) {
        boost::system::error_code error;
        boost::asio::write(*socket.get(), boost::asio::buffer(data), error);
        if (error) {
            LOG_FATAL << error.message();
            connected = false;
        }
    }

    atomic_bool read_in_thread = false;
    atomic_bool connected = false;
    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ip::tcp::socket> socket;
    std::string _address;
    std::string _port;
    std::string _path;
    std::unique_ptr<std::thread> read_thread;
    std::mutex thread_mutex;
    std::shared_ptr<std::function<void(WebSocketFrame)>> readCallback;
};