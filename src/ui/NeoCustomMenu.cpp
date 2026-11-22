#include "NeoCustomMenu.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace neo::ui {

class NeoCustomMenu : public CCLayerColor {
protected:
    CCLabelBMFont* m_statusLabel = nullptr;
    int m_testCount = 0;

    bool init() override {
        if (!CCLayerColor::initWithColor({0, 0, 0, 170}))
            return false;

        auto winSize = CCDirector::get()->getWinSize();

        // Screen center
        auto center = ccp(
            winSize.width / 2.f,
            winSize.height / 2.f
        );

        this->setContentSize(winSize);
        this->setPosition(CCPointZero);
        this->setKeypadEnabled(true);

        // ------------------------------------------
        // MAIN PANEL
        // ------------------------------------------

        auto panel = CCScale9Sprite::create("GJ_square01.png");

        if (!panel)
            return false;

        panel->setContentSize({340.f, 240.f});
        panel->setPosition(center);

        this->addChild(panel);

        // ------------------------------------------
        // TITLE
        // ------------------------------------------

        auto title = CCLabelBMFont::create(
            "Neo PS Plus",
            "goldFont.fnt"
        );

        if (title) {
            title->setPosition(ccp(0.f, 85.f));
            title->setScale(0.8f);
            panel->addChild(title);
        }

        // ------------------------------------------
        // DESCRIPTION
        // ------------------------------------------

        auto description = CCLabelBMFont::create(
            "Custom Menu - Test Version",
            "bigFont.fnt"
        );

        if (description) {
            description->setPosition(ccp(0.f, 40.f));
            description->setScale(0.45f);
            description->setColor({180, 210, 255});

            panel->addChild(description);
        }

        // ------------------------------------------
        // STATUS
        // ------------------------------------------

        m_statusLabel = CCLabelBMFont::create(
            "Menu loaded successfully!",
            "bigFont.fnt"
        );

        if (m_statusLabel) {
            m_statusLabel->setPosition(
                ccp(center.x, center.y + 15.f)
            );

            m_statusLabel->setScale(0.5f);
            m_statusLabel->setColor({180, 255, 180});

            this->addChild(m_statusLabel, 2);
        }

        // ------------------------------------------
        // BUTTON MENU
        // ------------------------------------------

        auto menu = CCMenu::create();

        if (!menu)
            return false;

        menu->setPosition(CCPointZero);
        this->addChild(menu, 3);

        // ------------------------------------------
        // TEST BUTTON
        // ------------------------------------------

        auto testSprite = ButtonSprite::create(
            "Test Button",
            "goldFont.fnt",
            "GJ_button_01.png",
            0.8f
        );

        if (testSprite) {
            auto testButton = CCMenuItemSpriteExtra::create(
                testSprite,
                this,
                menu_selector(NeoCustomMenu::onTest)
            );

            if (testButton) {
                testButton->setPosition(
                    ccp(center.x, center.y - 35.f)
                );

                menu->addChild(testButton);
            }
        }

        // ------------------------------------------
        // CLOSE BUTTON
        // ------------------------------------------

        auto closeSprite = CCSprite::createWithSpriteFrameName(
            "GJ_closeBtn_001.png"
        );

        if (closeSprite) {
            auto closeButton = CCMenuItemSpriteExtra::create(
                closeSprite,
                this,
                menu_selector(NeoCustomMenu::onClose)
            );

            if (closeButton) {
                closeButton->setPosition(
                    ccp(center.x + 150.f, center.y + 100.f)
                );

                menu->addChild(closeButton);
            }
        }

        return true;
    }

    // ----------------------------------------------
    // TEST BUTTON
    // ----------------------------------------------

    void onTest(CCObject*) {
        m_testCount++;

        if (m_statusLabel) {
            m_statusLabel->setString(
                fmt::format(
                    "Button clicked: {} times!",
                    m_testCount
                ).c_str()
            );
        }

        log::info(
            "[NeoCustomMenu] Test button clicked: {}",
            m_testCount
        );
    }

    // ----------------------------------------------
    // CLOSE MENU
    // ----------------------------------------------

    void onClose(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }

    void keyBackClicked() override {
        onClose(nullptr);
    }

public:
    static NeoCustomMenu* create() {
        auto ret = new NeoCustomMenu();

        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// ----------------------------------------------
// OPEN MENU
// ----------------------------------------------

void openCustomMenu() {
    auto scene = CCDirector::get()->getRunningScene();

    if (!scene)
        return;

    // Prevent duplicate menus.
    if (scene->getChildByID("neo-custom-menu"))
        return;

    auto menu = NeoCustomMenu::create();

    if (!menu)
        return;

    menu->setID("neo-custom-menu");

    scene->addChild(
        menu,
        scene->getHighestChildZ() + 100
    );

    log::info("[NeoCustomMenu] Menu opened.");
}

}
