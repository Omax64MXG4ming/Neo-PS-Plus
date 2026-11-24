#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/loader/SettingV3.hpp>

#include "ui/RestartButtons.hpp"
#include "BetterIncludes.hpp"
#include "updater/NeoUpdater.hpp"
#ifndef GEODE_IS_IOS

using namespace geode::prelude;

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
        
        return true;
    }
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

         Button sprite
       auto sprite = ButtonSprite::create(
          "Extra"
          "goldFont.fnt",
          "GJ_button_03.png",
          0.5f
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
    button->setPosition(
    ccp(
        winSize.width / 2.f,
        winSize.height / 2.f - 95.f
   ));
        menu->addChild(button);

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
        neo::ui::openNeoHub(this);
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
