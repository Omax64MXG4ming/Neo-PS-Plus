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
    // LOAD SPRITE ATLAS
    // ------------------------------------------
bool loadAtlas() {
    auto fileUtils = CCFileUtils::sharedFileUtils();

    if (!fileUtils)
        return false;

    float scale = CCDirector::get()->getContentScaleFactor();

    const char* candidates[3];

    if (scale >= 3.0f) {
        candidates[0] = "neo-hub-uhd.plist";
        candidates[1] = "neo-hub-hd.plist";
        candidates[2] = "neo-hub.plist";
    }
    else if (scale >= 1.5f) {
        candidates[0] = "neo-hub-hd.plist";
        candidates[1] = "neo-hub-uhd.plist";
        candidates[2] = "neo-hub.plist";
    }
    else {
        candidates[0] = "neo-hub.plist";
        candidates[1] = "neo-hub-hd.plist";
        candidates[2] = "neo-hub-uhd.plist";
    }

    auto resourceDir = Mod::get()->getResourcesDir();

    for (const char* filename : candidates) {
        auto plistPath = (
            resourceDir / "neo-hub" / filename
        ).string();

        if (!fileUtils->isFileExist(plistPath))
            continue;

        CCSpriteFrameCache::get()->addSpriteFramesWithFile(
            plistPath.c_str()
        );

        log::info("[NeoHub] Loaded atlas: {}", plistPath);

        return true;
    }

    log::error(
        "[NeoHub] No atlas found in mod resources: {}",
        resourceDir.string()
    );

    return false;
}
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

        // Load atlas before creating buttons
        loadAtlas();

        // --------------------------------------
        // RESPONSIVE CONTAINER
        // --------------------------------------

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
            winSize.height / 380.f
        });

        container->setScale(scale);
        this->addChild(container);

        // --------------------------------------
        // PANEL
        // --------------------------------------

        auto panel = CCScale9Sprite::create("GJ_square01.png");

        if (!panel)
            return false;

        panel->setContentSize({420.f, 350.f});
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
            header->setPosition(ccp(0.f, 145.f));
            container->addChild(header);
        }

        auto subtitle = CCLabelBMFont::create(
            "Neo PS Plus - Extra Page",
            "bigFont.fnt"
        );

        if (subtitle) {
            subtitle->setPosition(ccp(0.f, 118.f));
            subtitle->setScale(0.4f);
            subtitle->setColor({180, 210, 255});

            container->addChild(subtitle);
        }

        // --------------------------------------
        // PAGE CONTENT
        // --------------------------------------

        m_pageTitle = CCLabelBMFont::create(
            "Welcome to Neo PS Hub",
            "goldFont.fnt"
        );

        if (m_pageTitle) {
            m_pageTitle->setPosition(ccp(0.f, 80.f));
            m_pageTitle->setScale(0.65f);

            container->addChild(m_pageTitle);
        }

        m_pageDescription = CCLabelBMFont::create(
            "Explore the Neo PS Plus experience.",
            "bigFont.fnt"
        );

        if (m_pageDescription) {
            m_pageDescription->setPosition(ccp(0.f, 55.f));
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
    // ICON BUTTON FACTORY
    // ------------------------------------------

    CCMenuItemSpriteExtra* createIconButton(
    const char* frame,
    float scale,
    SEL_MenuHandler callback
) {
    auto icon = CCSprite::createWithSpriteFrameName(frame);

    if (!icon) {
        log::warn("[NeoHub] Missing sprite: {}", frame);
        return nullptr;
    }

    icon->setScale(scale);

    auto button = CCMenuItemSpriteExtra::create(
        icon,
        this,
        callback
    );

    return button;
}

    // ------------------------------------------
    // ADD ICON BUTTON
    // ------------------------------------------

    void addIconButton(
    const char* frame,
    float x,
    float y,
    SEL_MenuHandler callback
) {
    auto button = createIconButton(
        frame,
        0.30f,
        callback
    );

    if (!button)
        return;

    button->setPosition(ccp(x, y));
    m_menu->addChild(button);
}

    // ------------------------------------------
    // NORMAL BUTTON FACTORY
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
    // CLEAR BUTTONS
    // ------------------------------------------

    void clearButtons() {
        if (m_menu)
            m_menu->removeAllChildrenWithCleanup(true);
    }

    // ------------------------------------------
    // PAGE NAVIGATION
    // ------------------------------------------

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
        "Explore the Neo PS Plus experience."
    );

    addIconButton(
        "community.png",
        -105.f, -15.f,
        menu_selector(NeoHubLayer::onCommunity)
    );

    addIconButton(
        "credits.png",
        105.f, -15.f,
        menu_selector(NeoHubLayer::onCredits)
    );

    addIconButton(
        "dashboard.png",
        -105.f, -90.f,
        menu_selector(NeoHubLayer::onDashboard)
    );

    addIconButton(
        "changelog.png",
        105.f, -90.f,
        menu_selector(NeoHubLayer::onChangelog)
    );

    addButton(
        "NEXT >",
        0.f, -150.f,
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
                -90.f, -100.f,
                menu_selector(NeoHubLayer::onPrevious)
            );

            addButton(
                "BACK TO GAME",
                90.f, -100.f,
                menu_selector(NeoHubLayer::onBack)
            );
        }

        // --------------------------------------
        // CREDITS
        // --------------------------------------

        else if (page == 2) {
            setPage(
                "Neo PS Credits",
                "Created by the Neo PS team and Better Utils Codes."
            );

            addButton(
                "BACK",
                0.f, -110.f,
                menu_selector(NeoHubLayer::onReturnHome)
            );
        }
    }

    // ------------------------------------------
    // COMMUNITY
    // ------------------------------------------

    void onCommunity(CCObject*) {
        log::info("[NeoHub] Opening Community");

        utils::web::openLinkInBrowser(
            "https://discord.gg/TdB33cAyMn"
        );
    }

    // ------------------------------------------
    // DASHBOARD
    // ------------------------------------------

    void onDashboard(CCObject*) {
        log::info("[NeoHub] Opening Dashboard");

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
        log::info("[NeoHub] Opening Changelog");

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

    log::info("[NeoHub] Opened successfully.");
}

}
