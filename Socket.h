#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>
#include <utility>

class Socket {
public:
    explicit Socket(boost::asio::ip::tcp::socket socket) {
        this->socket = std::make_shared<boost::asio::ip::tcp::socket>(std::move(*&socket));
        //        this->socket = std::make_shared<boost::asio::ip::tcp::socket>(*socket)
        isSSL = false;
    }

    explicit Socket(boost::asio::io_context &io_context) {
        socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
        isSSL = false;
    }

    void SetIsSSL(const bool &useSSL) {
        this->isSSL = useSSL;
    }

    void SetSSL(boost::asio::ssl::context &
    ssl_context) {
        isSSL = true;
        sslSocket
        = std::make_shared<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(std::move(*socket),
                ssl_context);
    }

    std::shared_ptr<boost::asio::ip::tcp::socket> GetSocket() {
        return socket;
    }

    std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> GetSSLSocket() {
        return sslSocket;
    }

    void Close() {
        if (isSSL) {
            sslSocket->shutdown();
        } else {
            socket->shutdown(boost::asio::socket_base::shutdown_receive);
        }
    }

    [[nodiscard]] bool IsSSL() const {
        return isSSL;
    };

    [[nodiscard]] bool IsAvailable() const {
        if (isSSL)
            return sslSocket->lowest_layer().available();
        return socket->lowest_layer().available();
    }

private:
    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
    std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> sslSocket;
    bool isSSL;
};
