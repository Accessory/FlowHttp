#include <string>
#include <FlowUtils/FlowArgParser.h>
#include "Request.h"
#include "Response.h"
#include <thread>
#include "routes/Router.h"
#include "routes/FileNotFound.h"
#include "routes/GetRoute.h"
#include "routes/ListFiles.h"
#include "routes/ValidateRoute.h"
#include "routes/RelationalUpload.h"
#include "routes/IfModifiedSince.h"
#include "routes/InfoRoute.h"
#include "routes/TestRoute.h"
#include <memory>
#include "routes/GetBrotliRoute.h"
#include "util/ArgParserUtil.h"
#include "webdav/Webdav.h"
#include "FlowHttp2.h"

int main(int argc, char *argv[]) {
    FlowArgParser fap = ArgParserUtil::defaultArgParser();
    fap.parse("localhost 1337 .");
    fap.parse(argc, argv);

    std::string address = fap.getString("address");
    std::string port = fap.getString("port");
    std::string path = fap.getString("path");
    FlowString::replaceAll(path, "\\", "/");

    LOG_INFO << "Listening on: " << address << ":" << port;
    LOG_INFO << "Path: " << path;

    const size_t threadCount = fap.hasOption("threads") ? std::stoul(fap.getString("threads"), nullptr, 10) :
                               std::thread::hardware_concurrency() * 2;

    const std::string dh = fap.getString("dh");
    const std::string key = fap.getString("key");
    const std::string cert = fap.getString("cert");

    Router router;
    router.addRoute(std::make_shared<InfoRoute>());
    router.addRoute(std::make_shared<ValidateRoute>());
    router.addRoute(std::make_shared<TestRoute>());
    router.addRoute(std::make_shared<Webdav>(path));
    router.addRoute(std::make_shared<RelationalUpload>("./", "/upload"));
    router.addRoute(std::make_shared<IfModifiedSince>(path));
    router.addRoute(std::make_shared<GetBrotliRoute>(path));
    router.addRoute(std::make_shared<GetRoute>(path));
    router.addRoute(std::make_shared<ListFiles>(path));
    router.addRoute(std::make_shared<FileNotFound>());

    FlowHttp2 flowHttp (address, port, router, threadCount, dh, key, cert);
    flowHttp.Run();
    flowHttp.Join();
    return EXIT_SUCCESS;
}