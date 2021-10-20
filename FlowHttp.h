#pragma once

#include "FlowAsio.h"
#include <FlowUtils/WorkerPool.h>
#include "routes/Router.h"
#include <memory>
#include <FlowUtils/Semaphore.h>
#include <set>

class FlowHttp {
public:
    FlowHttp(const std::string &address, const std::string &port, Router router,
             size_t threads = std::thread::hardware_concurrency(),
             const std::string &dh = "", const std::string &key = "", const std::string &cert = "") :
            workerPool(threads),
            router(std::move(router))
    {
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
        keepRunning = false;
    }

    void Run() {
        Semaphore semaphore;
        size_t openConnections = 0;
        while (keepRunning) {
            semaphore.lock();
            workerPool.addTask(
                    std::make_shared<std::function<void()>>([&] { // Threadfunction
                        boost::system::error_code error;

                        Socket socket(io_context);
                        acceptor->accept(*socket.GetSocket(), error);
                        ++openConnections;
                        LOG_INFO << "Open connections: " << openConnections;
                        if (error)
                            return;

                        if (useSSL) {
                            socket.SetSSL(*ssl_context);
                            socket.GetSSLSocket()->handshake(boost::asio::ssl::stream_base::server, error);
                            if (error) {
                                LOG_WARNING << "Bad Request";
                                semaphore.unlock();
                                return;
                            }
                        }
                        semaphore.unlock();
                        bool continue_connection = true;
                        while (continue_connection) {
                            continue_connection = false;
                            Request request = FlowAsio::readRequest(socket);
                            Response response(socket.IsSSL());
                            router.execRoute(request, response, socket);
                            if (response.StatusCode == HttpStatusCode::BadRequest) {
                                break;
                            }
                            if (request.Header("Connection") == "keep-alive") {
                                continue_connection = true;
                            }
                        }
                        LOG_INFO << "Connection closed.";
                        --openConnections;
                    })); // Threadfunction
            workerPool.start();
        }
    }

    void Join() {
        workerPool.join();
    }

private:
    boost::asio::io_context io_context;
    std::unique_ptr<boost::asio::ssl::context> ssl_context;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    Router router;
    WorkerPool workerPool;
    bool keepRunning = true;
    bool useSSL = false;
};