#pragma once

#include "FlowAsio.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <utility>
#include <boost/asio/write.hpp>
#include <FlowUtils/ThreadPool.h>
#include "routes/Router.h"
#include <memory>
#include <FlowUtils/Semaphore.h>
#include <set>

class FlowHttp4 {
public:
    FlowHttp4(std::string address, std::string port, Router router,
              size_t threads = std::thread::hardware_concurrency(),
              const std::string &dh = "", const std::string &key = "", const std::string &cert = "") :
//            threadPool(threads),
            router(std::move(router)),
            address(std::move(address)),
            port(std::move(port)) {
//        acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(io_context, resolverResult.begin()->endpoint());
//        acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

        if (!dh.empty() && !key.empty() && !cert.empty()) {
            useSSL = true;
            ssl_context = std::make_unique<boost::asio::ssl::context>(
                    boost::asio::ssl::context::sslv23_server);

            ssl_context->set_options(
                    boost::asio::ssl::context::default_workarounds
                    | boost::asio::ssl::context::no_sslv2);

            ssl_context->use_certificate_chain_file(cert);
            ssl_context->use_private_key_file(key,
                                              boost::asio::ssl::context::pem);
            ssl_context->use_tmp_dh_file(dh);
        }

        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](boost::system::error_code a, auto b) {
            if(!a.failed()) {
                LOG_INFO << a.message();
                io_context.stop();
            }
        });
    }

    void Stop() {
        io_context.stop();
    }

    void Start() {
        boost::asio::co_spawn(io_context, listener(), boost::asio::detached);
    }

    void Run() {
        Start();
        Join();
    }

    void Join() {
        io_context.run();
    }

private:

    boost::asio::awaitable<void> listener() {
        boost::asio::ip::tcp::resolver::query query(address, port);
        boost::asio::ip::tcp::resolver resolver(io_context);
        const auto resolverResult = resolver.resolve(query);
        auto executor = co_await boost::asio::this_coro::executor;
        boost::asio::ip::tcp::acceptor acceptor(executor, resolverResult.begin()->endpoint());
        for (;;) {
            boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept(boost::asio::use_awaitable);
            std::thread t([&, s = std::move(socket)]() mutable {
                read(std::move(s));
            });
            t.detach();
        }
    }

    void read(boost::asio::ip::tcp::socket socket) {
        auto iSocket = useSSL ? FlowAsio::moveSocketToSSLSocket(socket, *ssl_context) : FlowAsio::moveSocketToSocket(
                socket);
        bool continue_connection = true;
        while (continue_connection) {
            continue_connection = false;
            Request request = FlowAsio::readRequest(iSocket);
            Response response(iSocket.IsSSL());
            router.execRoute(request, response, iSocket);

            if (response.StatusCode == HttpStatusCode::BadRequest) {
                break;
            }
            if (request.Header("Connection") == "keep-alive") {
                continue_connection = true;
            }
        }
    }


    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ssl::context> ssl_context;
//    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    Router router;
//    ThreadPool threadPool;
    bool useSSL = false;
    std::string address;
    std::string port;


};