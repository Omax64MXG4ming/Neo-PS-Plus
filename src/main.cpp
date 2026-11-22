#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/loader/SettingV3.hpp>

#include "ui/NeoBackendLayer.hpp"
#include "ui/RestartButtons.hpp"
#include "BetterIncludes.hpp"
#include "updater/NeoUpdater.hpp"
#include "ui/NeoCustomMenu.hpp"
#include "ui/NeoHub/NeoHubLayer.hpp"
#ifndef GEODE_IS_IOS

using namespace geode::prelude;
// Neo Buttons

class $modify(NeoCustomMenuButton, MenuLayer) {

    bool init() {
        if (!MenuLayer::init())
            return false;

        auto winSize = CCDirector::get()->getWinSize();

        // Button menu
        auto menu = CCMenu::create();

        if (!menu)
            return true;

        menu->setID("neo-custom-menu-button-menu");
        menu->setPosition(CCPointZero);

        this->addChild(menu, 1000);

        // Button sprite
        auto sprite = ButtonSprite::create(
            "Credits",
            "goldFont.fnt",
            "GJ_button_03.png",
            0.7f
        );

        if (!sprite)
            return true;

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(NeoCustomMenuButton::onOpenNeoMenu)
        );

        if (!button)
            return true;

        button->setPosition({
            winSize.width - 42.f,
            winSize.height - 45.f
        });

        menu->addChild(button);

        return true;
    }

    void onOpenNeoMenu(CCObject*) {
        neo::ui::openCustomMenu();
    }
};

// --------------------------------------------------
// MENU LAYER
// --------------------------------------------------
class $modify(AltMenuLayer, MenuLayer) {

    // ----------------------------------------------
    // SCENE
    // ----------------------------------------------

    static CCScene* scene(bool p0) {
        if (!neo::ui::RestartButtons::consumeReloadFlag()) {
            return MenuLayer::scene(p0);
        }

        return MenuLayer::scene(false);
    }

    // ----------------------------------------------
    // INIT
    // ----------------------------------------------

    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto winSize = CCDirector::get()->getWinSize();

        // ------------------------------------------
        // BUTTON CONTAINER
        // ------------------------------------------

auto menu = typeinfo_cast<CCMenu*>(
    this->getChildByID("close-menu")
);

if (!menu) {
    menu = CCMenu::create();

    menu->setID("close-menu");
    menu->setContentSize({33.25f, 200.f});
    menu->ignoreAnchorPointForPosition(false);

    this->addChild(menu);
}

        menu->setAnchorPoint({0.f, 1.f});

        menu->setPosition(
            winSize.width / 300.f,
            winSize.height - winSize.width / 300.f
        );

        auto layout = ColumnLayout::create()
            ->setAxisReverse(true)
            ->setAxisAlignment(AxisAlignment::End)
            ->setGap(4.f)
            ->setCrossAxisOverflow(false);

        menu->setLayout(layout);

        // ------------------------------------------
        // EXIT BUTTON
        // ------------------------------------------

        if (Mod::get()->getSettingValue<bool>("exit")) {

            if (!menu->getChildByID("close-button")) {
                auto sprite = CCSprite::createWithSpriteFrameName(
                    "GJ_closeBtn_001.png"
                );

                if (sprite) {
                    sprite->setScale(0.7f);

                    auto button = CCMenuItemSpriteExtra::create(
                        sprite,
                        this,
                        menu_selector(MenuLayer::onQuit)
                    );

                    if (button) {
                        button->setID("close-button");
                        menu->addChild(button);
                    }
                }
            }

        } else {
            menu->removeChildByID("close-button");
        }

        // ------------------------------------------
        // RESTART / RELOAD BUTTONS
        // ------------------------------------------

        neo::ui::RestartButtons::setup(
            menu,
            this,
            menu_selector(AltMenuLayer::onRestart),
            menu_selector(AltMenuLayer::onReload)
        );

        // ------------------------------------------
        // SIZE
        // ------------------------------------------

        const auto scale =
            Mod::get()->getSettingValue<double>("size");

        menu->setScale(static_cast<float>(scale));
        menu->updateLayout();

        // ------------------------------------------
        // NEO INSPECTOR BUTTON
        // ------------------------------------------

        if (!this->getChildByID("neo-inspector-menu")) {

            auto inspectorSprite = ButtonSprite::create(
                "Interceptor",
                "goldFont.fnt",
                "GJ_button_01.png",
                0.65f
            );

            if (inspectorSprite) {

                auto inspectorButton = CCMenuItemSpriteExtra::create(
                    inspectorSprite,
                    this,
                    menu_selector(AltMenuLayer::onNeoInspector)
                );

                if (inspectorButton) {

                    auto inspectorMenu = CCMenu::create(
                        inspectorButton,
                        nullptr
                    );

                    if (inspectorMenu) {
                        inspectorMenu->setID("neo-inspector-menu");

                        inspectorMenu->setPosition(
                            ccp(
                                winSize.width - 65.0f,
                                winSize.height - 35.0f
                            )
                        );

                        this->addChild(inspectorMenu, 100);
                    }
                }
            }
        }

        // ------------------------------------------
        // NEO HUB BUTTON
        // ------------------------------------------

        if (!this->getChildByID("neo-hub-menu")) {

            auto hubSprite = ButtonSprite::create(
                "Neo------++++HUB",
                "goldFont.fnt",
                "GJ_button_01.png",
                0.65f
            );

            if (hubSprite) {

                auto hubButton = CCMenuItemSpriteExtra::create(
                    hubSprite,
                    this,
                    menu_selector(AltMenuLayer::onOpenNeoHub)
                );

                if (hubButton) {

                    auto hubMenu = CCMenu::create();

                    if (hubMenu) {
                        hubMenu->setID("neo-hub-menu");
                        hubMenu->setPosition(CCPointZero);

                        hubButton->setPosition(
                            ccp(
                                winSize.width - 65.f,
                                winSize.height - 80.f
                            )
                        );

                        hubMenu->addChild(hubButton);

                        this->addChild(hubMenu, 100);
                    }
                }
            }
        }
        
        return true;
    }

    // ----------------------------------------------
    // BUTTON CALLBACKS
    // ----------------------------------------------

    void onRestart(CCObject*) {
        neo::ui::RestartButtons::restart();
    }

    void onReload(CCObject*) {
        neo::ui::RestartButtons::reload();
    }

    void onOpenNeoHub(CCObject*) {
        log::info("[NeoHub] Opening Neo Hub");

        neo::ui::openNeoHub(this);
    }

    void onNeoInspector(CCObject*) {
        log::info("[NeoInspector] Opening Inspector");
        NeoBackendLayer::open(this);
    }
};

// --------------------------------------------------
// GAME LOADED
// --------------------------------------------------
$on_game(Loaded) {
    // ----------------------------------------------
    // NEO PS PLUS UPDATER
    // ----------------------------------------------
neo::updater::NeoUpdater::checkForUpdates();
    // ----------------------------------------------
    // RESTART / RELOAD KEYBINDS
    // ----------------------------------------------

    neo::ui::RestartButtons::registerKeybinds();
}

#endif
