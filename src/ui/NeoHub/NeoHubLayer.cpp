#include "NeoHubLayer.hpp"

#include <Geode/Geode.hpp>
#include <algorithm>

using namespace geode::prelude;

namespace neo::ui {

class NeoHubLayer : public CCLayerColor {
protected:
    CCNode* m_originalMenu = nullptr;

    CCLabelBMFont* m_pageTitle = nullptr;
    CCLabelBMFont* m_pageDescription = nullptr;

    // ------------------------------------------
    // INITIALIZATION
    // ------------------------------------------

    bool init(CCNode* originalMenu) {
        if (!CCLayerColor::initWithColor({10, 17, 35, 255}))
            return false;

        m_originalMenu = originalMenu;

        auto winSize = CCDirector::get()->getWinSize();

        auto center = ccp(
            winSize.width / 2.f,
            winSize.height / 2.f
        );

        this->setContentSize(winSize);
        this->setPosition(CCPointZero);
        this->setKeypadEnabled(true);

        // --------------------------------------
        // RESPONSIVE CONTAINER
        // --------------------------------------

        auto container = CCNode::create();

        if (!container)
            return false;

        container->setPosition(center);

        float scale = std::min({
            1.f,
            winSize.width / 440.f,
            winSize.height / 300.f
        });

        container->setScale(scale);

        this->addChild(container);

        // --------------------------------------
        // MAIN PANEL
        // --------------------------------------

        auto panel = CCScale9Sprite::create(
            "GJ_square01.png"
        );

        if (!panel)
            return false;

        panel->setContentSize({420.f, 270.f});
        panel->setPosition(CCPointZero);

        container->addChild(panel);

        // --------------------------------------
        // HEADER
        // --------------------------------------

        auto header = CCLabelBMFont::create(
            "NEO HUB",
            "goldFont.fnt"
        );

        if (header) {
            header->setPosition(ccp(0.f, 105.f));
            header->setScale(1.f);

            container->addChild(header);
        }

        auto subtitle = CCLabelBMFont::create(
            "Neo PS Plus - Custom Interface",
            "bigFont.fnt"
        );

        if (subtitle) {
            subtitle->setPosition(ccp(0.f, 78.f));
            subtitle->setScale(0.4f);
            subtitle->setColor({180, 210, 255});

            container->addChild(subtitle);
        }

        // --------------------------------------
        // PAGE CONTENT
        // --------------------------------------

        m_pageTitle = CCLabelBMFont::create(
            "Welcome to Neo Hub",
            "goldFont.fnt"
        );

        if (m_pageTitle) {
            m_pageTitle->setPosition(ccp(0.f, 30.f));
            m_pageTitle->setScale(0.65f);

            container->addChild(m_pageTitle);
        }

        m_pageDescription = CCLabelBMFont::create(
            "Your custom Geometry Dash experience.",
            "bigFont.fnt"
        );

        if (m_pageDescription) {
            m_pageDescription->setPosition(ccp(0.f, 0.f));
            m_pageDescription->setScale(0.42f);
            m_pageDescription->setColor({200, 220, 255});

            container->addChild(m_pageDescription);
        }

        // --------------------------------------
        // NAVIGATION MENU
        // --------------------------------------

        auto menu = CCMenu::create();

        if (!menu)
            return false;

        menu->setPosition(CCPointZero);

        container->addChild(menu, 5);

        // HOME
        auto home = createButton(
            "HOME",
            menu_selector(NeoHubLayer::onHome)
        );

        if (home) {
            home->setPosition(ccp(-150.f, -65.f));
            menu->addChild(home);
        }

        // FEATURES
        auto features = createButton(
            "FEATURES",
            menu_selector(NeoHubLayer::onFeatures)
        );

        if (features) {
            features->setPosition(ccp(-50.f, -65.f));
            menu->addChild(features);
        }

        // SETTINGS
        auto settings = createButton(
            "SETTINGS",
            menu_selector(NeoHubLayer::onSettings)
        );

        if (settings) {
            settings->setPosition(ccp(50.f, -65.f));
            menu->addChild(settings);
        }

        // ABOUT
        auto about = createButton(
            "ABOUT",
            menu_selector(NeoHubLayer::onAbout)
        );

        if (about) {
            about->setPosition(ccp(150.f, -65.f));
            menu->addChild(about);
        }

        // --------------------------------------
        // BACK BUTTON
        // --------------------------------------

        auto back = createButton(
            "BACK",
            menu_selector(NeoHubLayer::onBack)
        );

        if (back) {
            back->setPosition(ccp(0.f, -112.f));
            menu->addChild(back);
        }

        // --------------------------------------
        // TOUCH AND KEYBOARD
        // --------------------------------------

        this->setTouchEnabled(true);
        this->setTouchMode(kCCTouchesOneByOne);

        // Menus normally process their buttons first.
        this->setTouchPriority(0);

        return true;
    }

    // ------------------------------------------
    // BUTTON FACTORY
    // ------------------------------------------

    CCMenuItemSpriteExtra* createButton(
        const char* text,
        SEL_MenuHandler callback
    ) {
        auto sprite = ButtonSprite::create(
            text,
            "goldFont.fnt",
            "GJ_button_01.png",
            0.65f
        );

        if (!sprite)
            return nullptr;

        return CCMenuItemSpriteExtra::create(
            sprite,
            this,
            callback
        );
    }

    // ------------------------------------------
    // PAGE HANDLER
    // ------------------------------------------

    void setPage(
        const char* title,
        const char* description
    ) {
        if (m_pageTitle)
            m_pageTitle->setString(title);

        if (m_pageDescription)
            m_pageDescription->setString(description);
    }

    // ------------------------------------------
    // HOME
    // ------------------------------------------

    void onHome(CCObject*) {
        setPage(
            "Welcome to Neo Hub",
            "Your custom Geometry Dash experience."
        );

        log::info("[NeoHub] Home opened.");
    }

    // ------------------------------------------
    // FEATURES
    // ------------------------------------------

    void onFeatures(CCObject*) {
        setPage(
            "Neo Features",
            "Your Neo PS Plus features will appear here."
        );

        log::info("[NeoHub] Features opened.");
    }

    // ------------------------------------------
    // SETTINGS
    // ------------------------------------------

    void onSettings(CCObject*) {
        setPage(
            "Neo Settings",
            "Customize your Neo PS Plus experience."
        );

        log::info("[NeoHub] Settings opened.");
    }

    // ------------------------------------------
    // ABOUT
    // ------------------------------------------

    void onAbout(CCObject*) {
        setPage(
            "About Neo PS Plus",
            "Created for the Neo PS community."
        );

        log::info("[NeoHub] About opened.");
    }

    // ------------------------------------------
    // BACK
    // ------------------------------------------

    void onBack(CCObject*) {
        this->removeFromParentAndCleanup(true);
    }

    void keyBackClicked() override {
        onBack(nullptr);
    }

    void onExit() override {
        // Restore original MenuLayer when leaving the Hub.
        if (m_originalMenu) {
            m_originalMenu->setVisible(true);
        }

        CCLayerColor::onExit();
    }

public:
    static NeoHubLayer* create(CCNode* originalMenu) {
        auto ret = new NeoHubLayer();

        if (ret && ret->init(originalMenu)) {
            ret->autorelease();
            return ret;
        }

        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool ccTouchBegan(CCTouch*, CCEvent*) override {
        // Consume touches outside the buttons.
        return true;
    }
};

// ----------------------------------------------
// OPEN NEO HUB
// ----------------------------------------------

void openNeoHub(CCNode* originalMenu) {
    auto scene = CCDirector::get()->getRunningScene();

    if (!scene || !originalMenu)
        return;

    // Prevent duplicate instances.
    if (scene->getChildByID("neo-hub"))
        return;

    auto hub = NeoHubLayer::create(originalMenu);

    if (!hub)
        return;

    hub->setID("neo-hub");

    // Hide original menu.
    originalMenu->setVisible(false);

    scene->addChild(
        hub,
        scene->getHighestChildZ() + 100
    );

    log::info("[NeoHub] Opened successfully.");
}

}
