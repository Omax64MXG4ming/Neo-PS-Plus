#include "NeoHubLayer.hpp"

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>

using namespace geode::prelude;

namespace neo::ui {

class NeoHubLayer : public CCLayerColor {
protected:
    CCNode* m_originalMenu = nullptr;
    CCMenu* m_menu = nullptr;

    CCLabelBMFont* m_pageTitle = nullptr;
    CCLabelBMFont* m_pageDescription = nullptr;

    int m_currentPage = 0;

    // ------------------------------------------
    // INITIALIZATION
    // ------------------------------------------

    bool init(CCNode* originalMenu) {
        if (!CCLayerColor::initWithColor({10, 17, 35, 255}))
            return false;

        m_originalMenu = originalMenu;

        auto winSize = CCDirector::get()->getWinSize();

        this->setContentSize(winSize);
        this->setPosition(CCPointZero);
        this->setKeypadEnabled(true);

        auto container = CCNode::create();
        if (!container)
            return false;

        container->setPosition(ccp(
            winSize.width / 2.f,
            winSize.height / 2.f
        ));

        float scale = std::min({
            1.f,
            winSize.width / 440.f,
            winSize.height / 300.f
        });

        container->setScale(scale);
        this->addChild(container);

        // --------------------------------------
        // PANEL
        // --------------------------------------

        auto panel = CCScale9Sprite::create("GJ_square01.png");
        if (!panel)
            return false;

        panel->setContentSize({420.f, 270.f});
        panel->setPosition(CCPointZero);
        container->addChild(panel);

        // --------------------------------------
        // HEADER
        // --------------------------------------

        auto header = CCLabelBMFont::create(
            "Extras",
            "goldFont.fnt"
        );

        if (header) {
            header->setPosition(ccp(0.f, 108.f));
            container->addChild(header);
        }

        auto subtitle = CCLabelBMFont::create(
            "Neo PS Plus - Extra Pag",
            "bigFont.fnt"
        );

        if (subtitle) {
            subtitle->setPosition(ccp(0.f, 80.f));
            subtitle->setScale(0.4f);
            subtitle->setColor({180, 210, 255});
            container->addChild(subtitle);
        }

        // --------------------------------------
        // PAGE CONTENT
        // --------------------------------------

        m_pageTitle = CCLabelBMFont::create(
            "Welcome to PS Neo Hub",
            "goldFont.fnt"
        );

        if (m_pageTitle) {
            m_pageTitle->setPosition(ccp(0.f, 43.f));
            m_pageTitle->setScale(0.65f);
            container->addChild(m_pageTitle);
        }

        m_pageDescription = CCLabelBMFont::create(
            "Your custom Geometry Dash experience.",
            "bigFont.fnt"
        );

        if (m_pageDescription) {
            m_pageDescription->setPosition(ccp(0.f, 18.f));
            m_pageDescription->setScale(0.38f);
            m_pageDescription->setColor({200, 220, 255});
            container->addChild(m_pageDescription);
        }

        // --------------------------------------
        // NAVIGATION MENU
        // --------------------------------------

        m_menu = CCMenu::create();
        if (!m_menu)
            return false;

        m_menu->setPosition(CCPointZero);
        container->addChild(m_menu, 5);

        showPage(0);

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

    void addButton(
        const char* text,
        float x,
        float y,
        SEL_MenuHandler callback
    ) {
        auto button = createButton(text, callback);
        if (!button)
            return;

        button->setPosition(ccp(x, y));
        m_menu->addChild(button);
    }

    // ------------------------------------------
    // PAGE TEXT
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
    // PAGE NAVIGATION
    // ------------------------------------------

    void clearButtons() {
        if (m_menu)
            m_menu->removeAllChildrenWithCleanup(true);
    }

    void showPage(int page) {
        if (!m_menu)
            return;

        m_currentPage = page;
        clearButtons();

        // --------------------------------------
        // PAGE 1
        // --------------------------------------

        if (page == 0) {
            setPage(
                "Welcome to Neo PS Hub",
                "Explore the Neo PS Plus experience. (._.)"
            );

            addButton(
                "COMMUNITY",
                -105.f, -42.f,
                menu_selector(NeoHubLayer::onCommunity)
            );

            addButton(
                "CREDITS",
                105.f, -42.f,
                menu_selector(NeoHubLayer::onCredits)
            );

            addButton(
                "DASHBOARD",
                -105.f, -88.f,
                menu_selector(NeoHubLayer::onDashboard)
            );

            addButton(
                "CHANGELOG",
                105.f, -88.f,
                menu_selector(NeoHubLayer::onChangelog)
            );

            addButton(
                "NEXT >",
                0.f, -122.f,
                menu_selector(NeoHubLayer::onNext)
            );
        }

        // --------------------------------------
        // PAGE 2
        // --------------------------------------

        else if (page == 1) {
            setPage(
                "Neo PS",
                "Discover more options and information."
            );


            addButton(
                "< PREVIOUS",
                105.f, -94.f,
                menu_selector(NeoHubLayer::onPrevious)
            );

            addButton(
                "BACK TO GAME",
                0.f, -122.f,
                menu_selector(NeoHubLayer::onBack)
            );
        }

        // --------------------------------------
        // INFORMATION PAGES
        // --------------------------------------

        else if (page == 2) {
            setPage(
                "Neo PS Credits",
                "Created with contributions from the Neo PS team and Better Utils Codes"
            );

            addButton(
                "BACK",
                0.f, -100.f,
                menu_selector(NeoHubLayer::onReturnHome)
            );
        }

        else if (page == 3) {
            setPage(
                "Neo PS Changelog",
                "Latest mod changes."
            );

            addButton(
                "BACK",
                0.f, -100.f,
                menu_selector(NeoHubLayer::onReturnHome)
            );
        }

            addButton(
                "BACK",
                0.f, -100.f,
                menu_selector(NeoHubLayer::onReturnHome)
            );
        }
            addButton(
                "BACK",
                0.f, -100.f,
                menu_selector(NeoHubLayer::onReturnHome)
            );
        }
}

            addButton(
                "BACK",
                0.f, -100.f,
                menu_selector(NeoHubLayer::onReturnHome)
            );
        }
    }

    // ------------------------------------------
    // COMMUNITY
    // ------------------------------------------

    void onCommunity(CCObject*) {
        utils::web::openLinkInBrowser(
            "https://discord.gg/TdB33cAyMn"
        );
    }

    // ------------------------------------------
    // DASHBOARD
    // ------------------------------------------

    void onDashboard(CCObject*) {

        utils::web::openLinkInBrowser(
            "https://neops.x10.mx/dashboard"
        );
    }

    // ------------------------------------------
    // CREDITS
    // ------------------------------------------

    void onCredits(CCObject*) {
        showPage(2);
    }

    // ------------------------------------------
    // CHANGELOG
    // ------------------------------------------

    void onChangelog(CCObject*) {
        utils::web::openLinkInBrowser(
            "https://github.com/Omax64MXG4ming/Neo-PS-Plus/releases"
        );
    }
    // ------------------------------------------
    // NEXT / PREVIOUS
    // ------------------------------------------

    void onNext(CCObject*) {
        showPage(1);
    }

    void onPrevious(CCObject*) {
        showPage(0);
    }

    void onReturnHome(CCObject*) {
        showPage(0);
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
        if (m_originalMenu)
            m_originalMenu->setVisible(true);

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
};

// ----------------------------------------------
// OPEN NEO HUB
// ----------------------------------------------

void openNeoHub(CCNode* originalMenu) {
    auto scene = CCDirector::get()->getRunningScene();

    if (!scene || !originalMenu)
        return;

    if (scene->getChildByID("neo-hub"))
        return;

    auto hub = NeoHubLayer::create(originalMenu);
    if (!hub)
        return;

    hub->setID("neo-hub");

    originalMenu->setVisible(false);

    scene->addChild(
        hub,
        scene->getHighestChildZ() + 100
    );
}
