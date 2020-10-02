#pragma once

#include <boost/uuid/uuid.hpp>
#include <set>
#include "Session.h"
#include <FlowUtils/FlowUUID.h>
#include <memory>

namespace SessionManager {
    std::map<boost::uuids::uuid, std::shared_ptr<Session>> sessions;

    bool tryGetSession(const boost::uuids::uuid &uuid, std::shared_ptr<Session> &session) {
        if (uuid == boost::uuids::nil_uuid())
            return false;

        auto pair = sessions.find(uuid);
        if (pair != sessions.end()) {
            session = pair->second;
            return true;
        }
        return false;
    }

    std::shared_ptr<Session> newSession() {
        boost::uuids::uuid id = FlowUUID::UUID();
        auto itr = sessions.emplace(id, std::make_shared<Session>(id));
        return itr.first->second;
    }

};
