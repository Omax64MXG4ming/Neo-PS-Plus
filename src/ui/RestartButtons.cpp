#include "RestartButtons.hpp"

#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/loader/SettingV3.hpp>

using namespace geode::prelude;

namespace neo::ui {

namespace {

bool isReload = false;

// --------------------------------------------------
// CHECK KEYBIND CONTEXT
// --------------------------------------------------

bool isKeybindAllowed() {
    auto scene = CCDirector::get()->getRunningScene();
    if (!scene)
        return false;

    const bool anywhere =
        Mod::get()->getSettingValue<bool>("keybind_anywhere");

    if (anywhere) {
        return LevelEditorLayer::get() == nullptr;
    }

    return scene->getChildByID("MenuLayer") != nullptr;
}

}

// --------------------------------------------------
// RESTART
// --------------------------------------------------

void RestartButtons::restart() {
    auto scene = CCDirector::get()->getRunningScene();
    if (!scene)
        return;

    if (
        scene->getChildByID("restart-popup") ||
        scene->getChildByID("reload-popup")
    ) {
        return;
    }

    auto popup = createQuickPopup(
        "Restart Game",
        "Are you sure you want to <cg>restart</c>?",
        "Cancel",
        "Yes",
        [](auto, bool confirmed) {
            if (confirmed) {
                utils::game::restart(true);
            }
        }
    );

    if (popup) {
        popup->setID("restart-popup");
    }
}

// --------------------------------------------------
// RELOAD TEXTURES
// --------------------------------------------------

void RestartButtons::reload() {
    auto scene = CCDirector::get()->getRunningScene();
    if (!scene)
        return;

    if (
        scene->getChildByID("restart-popup") ||
        scene->getChildByID("reload-popup")
    ) {
        return;
    }

    auto popup = createQuickPopup(
        "Reload Textures",
        "Are you sure you want to <cj>reload textures</c>?",
        "Cancel",
        "Yes",
        [](auto, bool confirmed) {
            if (confirmed) {
                isReload = true;
                GameManager::get()->reloadAll(
                    false,
                    false,
                    true
                );
            }
        }
    );

    if (popup) {
        popup->setID("reload-popup");
    }
}

// --------------------------------------------------
// BUTTON CREATION
// --------------------------------------------------

void RestartButtons::setup(
    CCMenu* menu,
    CCObject* target,
    SEL_MenuHandler restartCallback,
    SEL_MenuHandler reloadCallback
) {
    if (!menu || !target)
        return;

    // ----------------------------------------------
    // RESTART BUTTON
    // ----------------------------------------------

    if (Mod::get()->getSettingValue<bool>("restart")) {

        if (!menu->getChildByID("restart-button")) {
            auto sprite = CCSprite::createWithSpriteFrameName(
                "GJ_updateBtn_001.png"
            );

            if (sprite) {
                sprite->setScale(0.7f);

                auto button = CCMenuItemSpriteExtra::create(
                    sprite,
                    target,
                    restartCallback
                );

                if (button) {
                    button->setID("restart-button");
                    menu->addChild(button);
                }
            }
        }

    } else {
        menu->removeChildByID("restart-button");
    }

    // ----------------------------------------------
    // RELOAD TEXTURES BUTTON
    // ----------------------------------------------

    if (Mod::get()->getSettingValue<bool>("reload-textures")) {

        if (!menu->getChildByID("reload-button")) {
            auto sprite = CCSprite::createWithSpriteFrameName(
                "GJ_replayBtn_001.png"
            );

            if (sprite) {
                sprite->setScale(0.525f);

                auto button = CCMenuItemSpriteExtra::create(
                    sprite,
                    target,
                    reloadCallback
                );

                if (button) {
                    button->setID("reload-button");
                    menu->addChild(button);
                }
            }
        }

    } else {
        menu->removeChildByID("reload-button");
    }
}

// --------------------------------------------------
// RELOAD STATE
// --------------------------------------------------

bool RestartButtons::consumeReloadFlag() {
    const bool result = isReload;
    isReload = false;
    return result;
}

// --------------------------------------------------
// KEYBINDS
// --------------------------------------------------

void RestartButtons::registerKeybinds() {

    // ----------------------------------------------
    // RESTART KEYBIND
    // ----------------------------------------------

    listenForKeybindSettingPresses(
        "restart-keybind",
        [](Keybind const&, bool down, bool repeat, double) {

            if (!down || repeat)
                return;

            if (isKeybindAllowed()) {
                RestartButtons::restart();
            }
        }
    );

    // ----------------------------------------------
    // RELOAD TEXTURES KEYBIND
    // ----------------------------------------------

    listenForKeybindSettingPresses(
        "reload-keybind",
        [](Keybind const&, bool down, bool repeat, double) {

            if (!down || repeat)
                return;

            if (isKeybindAllowed()) {
                RestartButtons::reload();
            }
        }
    );
  }
}
