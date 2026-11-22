#include "CobaltMenu.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>

CobaltMenu* CobaltMenu::create() {
    auto ret = new CobaltMenu();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool CobaltMenu::init() {
    if (!CCLayer::init())
        return false;

    auto winSize = CCDirector::sharedDirector()->getWinSize();

    this->setTouchEnabled(true);
    this->setKeypadEnabled(true);

    // Dark overlay
    auto overlay = CCLayerColor::create(
        {0, 0, 0, 130},
        winSize.width,
        winSize.height
    );

    this->addChild(overlay);

    // Main panel
    auto panel = CCScale9Sprite::create(
        "GJ_square01.png"
    );

    if (!panel)
        return false;

    panel->setContentSize({
        winSize.width * 0.78f,
        winSize.height * 0.78f
    });

    panel->setPosition(
        winSize.width / 2,
        winSize.height / 2
    );

    this->addChild(panel);

    m_panel = panel;

    createInterface();

    return true;
}

void CobaltMenu::createInterface() {
    auto size = m_panel->getContentSize();

    // Header
    auto title = CCLabelBMFont::create(
        "COBALT MENU",
        "bigFont.fnt"
    );

    title->setScale(0.65f);
    title->setPosition(
        size.width / 2,
        size.height - 30.f
    );

    m_panel->addChild(title);

    // Close button
    auto closeMenu = CCMenu::create();

    auto closeButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName(
            "GJ_closeBtn_001.png"
        ),
        this,
        menu_selector(CobaltMenu::onClose)
    );

    closeMenu->addChild(closeButton);

    closeMenu->setPosition(
        size.width - 22.f,
        size.height - 22.f
    );

    m_panel->addChild(closeMenu);

    // Navigation
    createNavigation();

    // Content container
    auto content = CCNode::create();

    content->setContentSize({
        size.width - 145.f,
        size.height - 80.f
    });

    content->setPosition(130.f, 20.f);

    m_panel->addChild(content);

    m_content = content;

    showPage(0);
}

void CobaltMenu::createNavigation() {
    auto menu = CCMenu::create();

    menu->setPosition(0, 0);

    m_panel->addChild(menu);

    m_navigation = menu;

    const char* categories[] = {
        "General",
        "Speedhack",
        "Level",
        "Cheats",
        "Bypass",
        "Visual",
        "Debug"
    };

    auto size = m_panel->getContentSize();

    float startY = size.height - 85.f;

    for (int i = 0; i < 7; i++) {
        auto sprite = ButtonSprite::create(
            categories[i]
        );

        sprite->setScale(0.65f);

        auto button = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(CobaltMenu::onNavigation)
        );

        button->setTag(i);

        button->setPosition(
            65.f,
            startY - (i * 43.f)
        );

        menu->addChild(button);
    }
}

void CobaltMenu::onNavigation(CCObject* sender) {
    auto button = static_cast<CCNode*>(sender);

    showPage(button->getTag());
}

void CobaltMenu::showPage(int page) {
    m_currentPage = page;

    m_content->removeAllChildren();

    const char* names[] = {
        "General Settings",
        "Speedhack Settings",
        "Level Settings",
        "Gameplay Settings",
        "Bypass Settings",
        "Visual Settings",
        "Debug Settings"
    };

    auto size = m_content->getContentSize();

    auto title = CCLabelBMFont::create(
        names[page],
        "bigFont.fnt"
    );

    title->setScale(0.55f);

    title->setAnchorPoint({0.f, 0.5f});

    title->setPosition(
        10.f,
        size.height - 25.f
    );

    m_content->addChild(title);
}

void CobaltMenu::onClose(CCObject*) {
    this->removeFromParentAndCleanup(true);
}

void CobaltMenu::toggle() {
    if (this->getParent()) {
        this->removeFromParentAndCleanup(true);
    }
}
