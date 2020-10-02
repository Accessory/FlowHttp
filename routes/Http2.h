#pragma once


#include "../Request.h"
#include "../Socket.h"
#include "../Response.h"
#include "Route.h"
#include <FlowUtils/FlowLog.h>
#include "../FlowAsio.h"
#include "../hpack/HPack.h"
#include "../http2/Frame.h"
#include "../http2/Setting.h"
#include <FlowUtils/base64.h>


class Http2 : public Route {
public:
    Http2() : Route(".*", ".*") {}

private:
    Socket *sessionSocket;

    virtual bool Run(Request &request, Response &response, Socket &socket) {
        using namespace Flow_Http2;
        if (request.HttpVersion() == "1.1") {
            if (request.Header("Upgrade") == "h2c") {
                auto settings = request.Header("HTTP2-Settings");
                auto dec_settings = Base64::base64_decode(settings);
                Settings_Frame frame;
                frame.Length = dec_settings.size();
                frame.Parse(dec_settings, 0, dec_settings.size());
                LOG_INFO << dec_settings;

                response.StatusCode = HttpStatusCode::SwitchingProtocols;
                response.emplace("Upgrade", "h2c");
                response.emplace("Connection", "Upgrade");
                FlowAsio::write(socket, response.Header());
                auto data = FlowAsio::read(socket);
                LOG_INFO << data;
                Settings_Frame settingsFrame;

                FlowAsio::write(socket, settingsFrame.createData());
                const std::string send = ":status: 200\n\r"
                                         "content-type: text/html\n\r"
                                         "content-length: 5";
                const std::string payload = "Hallo";

                auto hello = HPack::encode(send, true);

                Frame frame4;
                frame4.Type = FrameType::HEADERS;
                frame4.Flags = 4;
                frame4.Stream_Identifier = 1;
                frame4.Payload.assign(hello.begin(), hello.end());
                FlowAsio::write(socket, frame4.createData());

                Frame frame5;
                frame5.Type = FrameType::DATA;
                frame5.Flags = 1;
                frame5.Stream_Identifier = 1;
                std::copy(payload.begin(), payload.end(), std::back_inserter(frame5.Payload));
                FlowAsio::write(socket, frame5.createData());
                data = FlowAsio::read(socket);
                LOG_INFO << data;
                return true;
            }
        }
        if (request.HttpVersion() == "2.0") {
            auto data = request.GetParameter("data");
            size_t dataPos = 0;
            Settings_Frame frame;
            dataPos = frame.Parse(data, dataPos);

            if (frame.error) {
                FlowAsio::sendBadRequest(socket);
                return true;
            }
            LOG_INFO << "DataSize: " << data.size();
            LOG_INFO << "PayloadSize: " << frame.Length;
            LOG_INFO << "Stream Identifier: " << frame.Stream_Identifier;
            for (auto &setting : frame) {
                LOG_INFO << "Identifier: " << setting.Identifier << " Value: " << setting.Value;
            }


            Frame frame2;
            dataPos = frame2.Parse(data, dataPos);
            for (size_t i = 0; i < frame2.Length; ++i) {
                frame2.Payload.emplace_back(data.at(dataPos++));
            }

            if (data.size() <= dataPos) {
                data += FlowAsio::read(socket);
            }

            Frame frame3;
            dataPos = frame3.Parse(data, dataPos);
            for (size_t i = 0; i < frame3.Length; ++i) {
                frame3.Payload.emplace_back(data.at(dataPos++));
            }

            auto headerString = HPack::decode(frame3.Payload);
            LOG_INFO << std::endl << headerString;


            Frame answer;
            answer.Flags = 0;
            answer.Stream_Identifier = 0;
            answer.Type = FrameType::SETTINGS;

            FlowAsio::write(socket, answer.createData());

            const std::string send = ":status: 200\n\r"
                                     "content-type: text/html\n\r"
                                     "content-length: 5";
            const std::string payload = "Hallo";

            auto hello = HPack::encode(send, true);
//            std::copy(payload.begin(), payload.end(), std::back_inserter(hello));
            Frame frame4;
            frame4.Type = FrameType::HEADERS;
            frame4.Flags = 4;
            frame4.Stream_Identifier = 1;
            frame4.Payload.assign(hello.begin(), hello.end());
            FlowAsio::write(socket, frame4.createData());

            Frame frame5;
            frame5.Type = FrameType::DATA;
            frame5.Flags = 1;
            frame5.Stream_Identifier = 1;
            std::copy(payload.begin(), payload.end(), std::back_inserter(frame5.Payload));
            FlowAsio::write(socket, frame5.createData());
            data = FlowAsio::read(socket);
            LOG_INFO << data;
            Settings_Frame last;
            last.Parse(data, 0);
            return true;
        }
        return false;
    }
};