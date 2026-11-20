#pragma once

#include <Geode/Geode.hpp>

namespace neo::ui {
class RestartButtons {
public:
    static void restart();
    static void reload();
    static void setup(
        cocos2d::CCMenu* menu,
        cocos2d::CCObject* target,
        cocos2d::SEL_MenuHandler restartCallback,
        cocos2d::SEL_MenuHandler reloadCallback
    );

    static bool consumeReloadFlag();
    static void registerKeybinds();
};

}
