#pragma once

#include "FlowAsio.h"
//#include <FlowUtils/ThreadPool.h>
#include <FlowUtils/WorkerPool.h>
#include "routes/Router.h"
#include <memory>
#include <FlowUtils/Semaphore.h>
#include <set>

class FlowHttp2 {
public:
    FlowHttp2(const std::string &address, const std::string &port, Router router,
              size_t threads = std::thread::hardware_concurrency(),
              const std::string &dh = "", const std::string &key = "", const std::string &cert = "") :
              workerPool(threads),
            router(std::move(router)) {
        boost::asio::ip::tcp::resolver::query query(address, port);
        boost::asio::ip::tcp::resolver resolver(io_context);
        const auto resolverResult = resolver.resolve(query);
        acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(io_context, resolverResult.begin()->endpoint());
        acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

        if (!dh.empty() && !key.empty() && !cert.empty()) {
            useSSL = true;
            ssl_context = std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23_server);

            ssl_context->set_options(
                    boost::asio::ssl::context::default_workarounds
                    | boost::asio::ssl::context::no_sslv2);

            ssl_context->use_certificate_chain_file(cert);
            ssl_context->use_private_key_file(key,
                                              boost::asio::ssl::context::pem);
            ssl_context->use_tmp_dh_file(dh);
        }
    }

    void Stop() {
        io_context.stop();
    }

    void Start() {
        acceptor->async_accept([&](
                const boost::system::error_code &error, boost::asio::ip::tcp::socket socket) {
            if (error) {
                return;
            }
            Start();
            Socket iSocket(std::move(socket));

            workerPool.addTask(std::make_shared<std::function<void()>>(
                    [&, iSocket]() mutable { // Threadfunction
                        if (useSSL) {
                            boost::system::error_code error;
                            iSocket.SetSSL(*ssl_context);
                            iSocket.GetSSLSocket()->handshake(boost::asio::ssl::stream_base::server, error);
                            if (error) {
                                LOG_WARNING << "Bad Request";
                                return;
                            }
                        }
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
                    })); // Threadfunction
                    workerPool.start();
        });
    }

    void Run() {
        Start();
        Join();
    }

    void Join() {
        io_context.run();
    }

private:
    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ssl::context> ssl_context;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    Router router;
    WorkerPool workerPool;
    bool useSSL = false;
};