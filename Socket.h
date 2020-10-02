#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <memory>

class Socket {
public:
    Socket(boost::asio::io_context& io_context) {
        socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
        isSSL = false;
    }

    void SetIsSSL(const bool& useSSL){
        this->isSSL = useSSL;
    }

    void SetSSL(boost::asio::ssl::context&
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

    bool IsSSL(){
        return isSSL;
    };

private:
    std::shared_ptr<boost::asio::ip::tcp::socket> socket;
    std::shared_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> sslSocket;
    bool isSSL;
};
