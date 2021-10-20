cmake_minimum_required(VERSION 3.4)
project(FlowHttp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 20)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
link_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCE_FILES
        routes/Route.h routes/Router.h RequestMethod.h
        routes/FileNotFound.h routes/GetRoute.h
        routes/StaticUploadRoute.h routes/BasicAuth.h Socket.h
        routes/FileBrowser.h routes/PathCheck.h CookieUtil.h Cookie.h
        MimeTypes.h
        Request.h
        Response.h routes/SessionRoute.h
        MulPaField.h
        manager/SessionManager.h manager/Session.h
        routes/ListFiles.h HttpParserState.h routes/Http2.h util/ArgParserUtil.h util/WebSocketUtil.h
        FlowHttp.h request_object/RequestObject.h request_object/RequestEntry.h request_object/RequestString.h)

include_directories(../Flow)

add_executable(FlowHttp main.cpp ${SOURCE_FILES})
#add_executable(pwm pwm/pwm.cpp)
#add_executable(test hpack/hmain.cpp hpack/HPackTable.h hpack/HPack.h hpack/Huffman.h)
#target_link_libraries(pwm argon2)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.72.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(FlowHttp ${Boost_LIBRARIES})
#target_link_libraries(pwm ${Boost_LIBRARIES})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(FlowHttp ${OPENSSL_LIBRARIES})

find_package(Brotli)
target_link_libraries(FlowHttp Brotli::brotlienc Brotli::brotlicommon)


find_package(Threads)
target_link_libraries(FlowHttp ${CMAKE_THREAD_LIBS_INIT})

find_package(tinyxml2 CONFIG REQUIRED)
include_directories(${TinyXML2_INCLUDE_DIRS})
target_link_libraries(FlowHttp tinyxml2::tinyxml2)

if (WIN32)
    target_link_libraries(FlowHttp ws2_32)
    target_link_libraries(FlowHttp wsock32)
    target_link_libraries(FlowHttp bcrypt)
endif ()


SET(HEADER
        FlowAsio.h
        Cookie.h
        CookieUtil.h
        MimeTypes.h
        MulPaField.h
        RequestMethod.h
        Request.h
        Response.h
        Socket.h
        HttpStatus.h
        HttpParserState.h
        )

set_target_properties(FlowHttp PROPERTIES PUBLIC_HEADER
        "${HEADER}"
        )

if (UNIX)
    include(GNUInstallDirs)

    install(TARGETS FlowHttp
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${CMAKE_PROJECT_NAME}
            )

#    install(TARGETS pwm
#            RUNTIME DESTINATION bin
#            )

    install(DIRECTORY routes/ DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}/FlowHttp/routes)

    install(DIRECTORY manager/ DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}/FlowHttp/manager)

    install(DIRECTORY util/ DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}/FlowHttp/util)

    #    install(TARGETS FlowHttp_Core
    #            RUNTIME DESTINATION bin
    #            LIBRARY DESTINATION lib
    #            ARCHIVE DESTINATION lib
    #            )
endif ()

target_precompile_headers(FlowHttp PRIVATE
        <boost/asio.hpp>
        <boost/asio/ssl.hpp>
        <FlowUtils/Semaphore.h>
        <FlowUtils/FlowLog.h>
        FlowAsio.h
        HttpStatus.h HttpParserState.h
        MulPaField.h Response.h
        Request.h Socket.h
        routes/InfoRoute.h
        routes/ValidateSession.h
        webdav/Webdav.h
        routes/RelationalUpload.h
        routes/IfModifiedSince.h
        routes/GetBrotliRoute.h
        routes/GetRoute.h
        routes/ListFiles.h
#        routes/FileNotFound.h
        FlowHttp.h
        )