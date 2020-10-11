cmake_minimum_required(VERSION 3.4)
project(FlowHttp)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 17)


include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SOURCE_FILES
        routes/Route.h routes/Router.h RequestMethod.h
        routes/FileNotFound.h routes/GetRoute.h
        routes/StaticUploadRoute.h routes/BasicAuth.h Socket.h
        routes/FileBrowser.h routes/PathCheck.h CookieGod.h Cookie.h
        MimeTypes.h
        Request.h
        Response.h routes/SessionRoute.h
        MulPaField.h
        manager/SessionManager.h manager/Session.h
        routes/ListFiles.h HttpParserState.h routes/Http2.h util/ArgParserUtil.h)

include_directories(../Flow)

add_executable(FlowHttp main2.cpp ${SOURCE_FILES})
#add_executable(test hpack/hmain.cpp hpack/HPackTable.h hpack/HPack.h hpack/Huffman.h)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.66.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(FlowHttp ${Boost_LIBRARIES})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(FlowHttp ${OPENSSL_LIBRARIES})

find_package(Brotli)
target_link_libraries(FlowHttp Brotli::brotlienc Brotli::brotlicommon)


find_package(Threads)
target_link_libraries(FlowHttp ${CMAKE_THREAD_LIBS_INIT})

if (WIN32)
    target_link_libraries(FlowHttp ws2_32)
    target_link_libraries(FlowHttp wsock32)
    target_link_libraries(FlowHttp bcrypt)
endif ()


SET(HEADER
        FlowAsio.h
        Cookie.h
        CookieGod.h
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