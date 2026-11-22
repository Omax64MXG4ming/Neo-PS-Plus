#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>

#include "menu/CobaltMenu.hpp"

using namespace geode::prelude;

class $modify(CobaltMenuButton, MenuLayer) {
    bool init() {
        if (!MenuLayer::init())
            return false;

        auto menu = this->getChildByID("bottom-menu");

        if (!menu)
            return true;

        auto button = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
            this,
            menu_selector(CobaltMenuButton::onCobaltMenu)
        );

        if (!button)
            return true;

        button->setID("cobalt-menu-button");

        menu->addChild(button);
        menu->updateLayout();

        return true;
    }

    void onCobaltMenu(CCObject*) {
        CobaltMenu::toggle();
    }
};
