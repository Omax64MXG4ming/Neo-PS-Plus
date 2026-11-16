#pragma once

#include <Geode/Geode.hpp>
#include <string>

namespace neo::config {

    inline std::string getBackendURL() {
        return geode::Mod::get()->getSettingValue<std::string>(
            "backend-url"
        );
    }

    inline bool isInterceptEnabled() {
        return geode::Mod::get()->getSettingValue<bool>(
            "intercept-enabled"
        );
    }

    inline bool interceptAllServers() {
        return geode::Mod::get()->getSettingValue<bool>(
            "intercept-all-servers"
        );
    }

}
