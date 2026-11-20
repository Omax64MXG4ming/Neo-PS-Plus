#pragma once

#include <Geode/Geode.hpp>

namespace neo::ui {

class RestartButtons {
public:
    // Restart game confirmation.
    static void restart();

    // Reload textures confirmation.
    static void reload();

    // Creates/removes Restart and Reload buttons.
    static void setup(
        CCMenu* menu,
        CCObject* target,
        SEL_MenuHandler restartCallback,
        SEL_MenuHandler reloadCallback
    );

    // Returns true if the next MenuLayer scene must skip texture reload.
    static bool consumeReloadFlag();

    // Registers both keybinds.
    static void registerKeybinds();
};

}
