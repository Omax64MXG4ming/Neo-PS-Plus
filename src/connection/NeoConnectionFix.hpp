#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/CCHttpClient.hpp>

namespace neo::connection {

class $modify(NeoConnectionFix, cocos2d::extension::CCHttpClient) {
    void send(cocos2d::extension::CCHttpRequest* request);
};

}
