#include "CobaltMenu.hpp"
#include "../features/CobaltState.hpp"

#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemToggler.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>

using namespace geode::prelude;

namespace {

float getUIScale(const CCSize& size) {
    constexpr float referenceWidth = 854.f;
    constexpr float referenceHeight = 480.f;

    float scaleX = size.width / referenceWidth;
    float scaleY = size.height / referenceHeight;

    return std::max(
        0.55f,
        std::min(1.f, std::min(scaleX, scaleY))
    );
}

enum ToggleID {
    Noclip = 1,
    NoCollision,
    AutoComplete,

    AutoLDM,
    CopyHack,
    EditHack,
    DisableQuit,

    IconBypass,
    LockIcons,
    CharacterBypass,
    SliderBypass,
    AutoVerify,

    NoHide,
    NoDeathEffect,
    NoShake,
    NoTransitions,

    SpeedSong,
    SpeedHack = 18
};

}

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

    buildMenu();

    return true;
}

// --------------------------------------------------
// BUILD MENU
// --------------------------------------------------

void CobaltMenu::buildMenu() {
    auto win = CCDirector::sharedDirector()->getWinSize();

    float s = getUIScale(win);

    float width = std::min(
        win.width - 24.f * s,
        1000.f
    );

    float height = std::min(
        win.height - 24.f * s,
        680.f
    );

    width = std::max(200.f, width);
    height = std::max(200.f, height);

    // Overlay
    auto overlay = CCLayerColor::create(
        {0, 0, 0, 145}
    );

    if (!overlay)
        return;

    overlay->setContentSize(win);
    overlay->setPosition(0.f, 0.f);

    addChild(overlay, -1);

    // Main panel
    auto panel = CCScale9Sprite::create(
        "GJ_square01.png"
    );

    if (!panel)
        return;

    panel->setContentSize({width, height});
    panel->setColor({25, 32, 45});
    panel->setOpacity(245);

    panel->setPosition(
        win.width / 2.f,
        win.height / 2.f
    );

    addChild(panel, 1);

    m_panel = panel;

    // Header
    auto title = CCLabelBMFont::create(
        "COBALT",
        "bigFont.fnt"
    );

    if (title) {
        title->setScale(0.75f * s);
        title->setPosition(
            width / 2.f,
            height - 25.f * s
        );

        panel->addChild(title);
    }

    // Subtitle
    auto subtitle = CCLabelBMFont::create(
        "MOD MENU",
        "goldFont.fnt"
    );

    if (subtitle) {
        subtitle->setScale(0.38f * s);
        subtitle->setColor({120, 220, 205});

        subtitle->setPosition(
            width / 2.f,
            height - 49.f * s
        );

        panel->addChild(subtitle);
    }

    // Close button
    auto closeMenu = CCMenu::create();

    if (closeMenu) {
        closeMenu->setPosition(0.f, 0.f);

        auto closeSprite =
            CCSprite::createWithSpriteFrameName(
                "GJ_closeBtn_001.png"
            );

        if (closeSprite) {
            auto closeButton =
                CCMenuItemSpriteExtra::create(
                    closeSprite,
                    this,
                    menu_selector(CobaltMenu::onClose)
                );

            if (closeButton) {
                closeButton->setPosition(
                    width - 19.f * s,
                    height - 19.f * s
                );

                closeMenu->addChild(closeButton);
            }
        }

        panel->addChild(closeMenu, 10);
    }

    // Separator
    auto separator = CCLayerColor::create(
        {80, 105, 125, 180},
        width - 30.f * s,
        1.f * s
    );

    if (separator) {
        separator->setPosition(
            15.f * s,
            height - 65.f * s
        );

        panel->addChild(separator);
    }

    // Scroll area
    float scrollWidth = width - 30.f * s;
    float scrollHeight = height - 115.f * s;

    m_scroll = ScrollLayer::create({
        scrollWidth,
        scrollHeight
    });

    if (!m_scroll)
        return;

    m_scroll->setPosition(
        15.f * s,
        22.f * s
    );

    panel->addChild(m_scroll, 2);

    buildOptions();

    // Opening animation
    panel->setScale(0.85f);
    panel->setOpacity(0);

    panel->runAction(
        CCEaseBackOut::create(
            CCScaleTo::create(0.22f, 1.f)
        )
    );

    panel->runAction(
        CCFadeTo::create(0.18f, 245)
    );
}

// --------------------------------------------------
// TOUCH
// --------------------------------------------------

void CobaltMenu::registerWithTouchDispatcher() {
    CCTouchDispatcher::get()->addTargetedDelegate(
        this,
        -128,
        true
    );
}

bool CobaltMenu::ccTouchBegan(
    CCTouch* touch,
    CCEvent* event
) {
    return true;
}

// --------------------------------------------------
// SECTION
// --------------------------------------------------

void CobaltMenu::addSection(
    const char* title,
    float& y
) {
    auto content = m_scroll->m_contentLayer;

    auto label = CCLabelBMFont::create(
        title,
        "goldFont.fnt"
    );

    if (!label)
        return;

    label->setScale(0.48f);
    label->setColor({65, 205, 180});

    label->setAnchorPoint({0.f, 0.5f});
    label->setPosition(10.f, y);

    content->addChild(label);

    y -= 27.f;
}

// --------------------------------------------------
// TOGGLE
// --------------------------------------------------

void CobaltMenu::addToggle(
    const char* title,
    int id,
    bool value,
    float& y
) {
    auto content = m_scroll->m_contentLayer;

    float width = content->getContentSize().width;

    // Row background
    auto row = CCLayerColor::create(
        {35, 43, 58, 210},
        width - 18.f,
        27.f
    );

    if (row) {
        row->setPosition(
            9.f,
            y - 13.5f
        );

        content->addChild(row);
    }

    // Label
    auto label = CCLabelBMFont::create(
        title,
        "bigFont.fnt"
    );

    if (label) {
        label->setScale(0.42f);
        label->setAnchorPoint({0.f, 0.5f});
        label->setPosition(12.f, y);

        content->addChild(label);
    }

    // Toggle sprites
    auto offSprite =
        CCSprite::createWithSpriteFrameName(
            "GJ_checkOff_001.png"
        );

    auto onSprite =
        CCSprite::createWithSpriteFrameName(
            "GJ_checkOn_001.png"
        );

    if (!offSprite || !onSprite) {
        log::warn("Cobalt: Could not load toggle sprites");
        y -= 32.f;
        return;
    }

    offSprite->setScale(0.65f);
    onSprite->setScale(0.65f);

    // Toggle item
    auto toggle = CCMenuItemToggler::create(
        offSprite,
        onSprite,
        this,
        menu_selector(CobaltMenu::onToggle)
    );

    if (!toggle) {
        y -= 32.f;
        return;
    }

    toggle->setTag(id);

    if (value)
        toggle->toggle(true);

    // Toggle menu
    auto menu = CCMenu::create();

    if (!menu) {
        y -= 32.f;
        return;
    }

    menu->setPosition(
        width - 29.f,
        y
    );

    menu->addChild(toggle);

    content->addChild(menu);

    y -= 32.f;
}

// --------------------------------------------------
// BUILD OPTIONS
// --------------------------------------------------

void CobaltMenu::buildOptions() {
    auto content = m_scroll->m_contentLayer;

    float width = m_scroll->getContentSize().width;

    constexpr int sectionCount = 5;
    constexpr int optionCount = 18;

    constexpr float sectionHeight = 27.f;
    constexpr float optionHeight = 32.f;
    constexpr float speedControlHeight = 40.f;

    float contentHeight =
        40.f +
        sectionCount * sectionHeight +
        optionCount * optionHeight +
        speedControlHeight +
        40.f;

    content->setContentSize({
        width,
        contentHeight
    });

    float y = contentHeight - 20.f;

    // SPEEDHACK
    addSection("SPEEDHACK", y);

    addToggle(
        "Enable Speedhack",
        SpeedHack,
        cobalt::g_state.speedhack,
        y
    );

    addToggle(
        "Speedhack Song",
        SpeedSong,
        cobalt::g_state.speedhackSong,
        y
    );

    // Speed title
    auto speedTitle = CCLabelBMFont::create(
        "Speed",
        "bigFont.fnt"
    );

    if (speedTitle) {
        speedTitle->setScale(0.43f);
        speedTitle->setAnchorPoint({0.f, 0.5f});
        speedTitle->setPosition(12.f, y);

        content->addChild(speedTitle);
    }

    // Speed value
    m_speedLabel = CCLabelBMFont::create(
        fmt::format(
            "{:.2f}x",
            cobalt::g_state.speed
        ).c_str(),
        "bigFont.fnt"
    );

    if (m_speedLabel) {
        m_speedLabel->setScale(0.43f);

        m_speedLabel->setPosition(
            width - 60.f,
            y
        );

        content->addChild(m_speedLabel);
    }

    // Speed buttons
    auto speedMenu = CCMenu::create();

    if (speedMenu) {
        auto minusLabel = CCLabelBMFont::create(
            "-",
            "bigFont.fnt"
        );

        auto plusLabel = CCLabelBMFont::create(
            "+",
            "bigFont.fnt"
        );

        if (minusLabel && plusLabel) {
            auto minus = CCMenuItemSpriteExtra::create(
                minusLabel,
                this,
                menu_selector(CobaltMenu::onSpeedDown)
            );

            auto plus = CCMenuItemSpriteExtra::create(
                plusLabel,
                this,
                menu_selector(CobaltMenu::onSpeedUp)
            );

            if (minus && plus) {
                minus->setPosition(
                    width - 105.f,
                    y
                );

                plus->setPosition(
                    width - 15.f,
                    y
                );

                speedMenu->addChild(minus);
                speedMenu->addChild(plus);
            }
        }

        speedMenu->setPosition(0.f, 0.f);

        content->addChild(speedMenu);
    }

    y -= speedControlHeight;

    // GAMEPLAY
    addSection("GAMEPLAY", y);

    addToggle("Noclip", Noclip, cobalt::g_state.noclip, y);
    addToggle("No Collision", NoCollision, cobalt::g_state.noCollision, y);
    addToggle("Auto Complete", AutoComplete, cobalt::g_state.autoComplete, y);

    // LEVEL
    addSection("LEVEL", y);

    addToggle("Auto LDM", AutoLDM, cobalt::g_state.autoLDM, y);
    addToggle("Copy Hack", CopyHack, cobalt::g_state.copyHack, y);
    addToggle("Edit Hack", EditHack, cobalt::g_state.editHack, y);
    addToggle("No Quit", DisableQuit, cobalt::g_state.disableQuit, y);

    // BYPASS
    addSection("BYPASS", y);

    addToggle("Icon Bypass", IconBypass, cobalt::g_state.iconBypass, y);
    addToggle("Lock All Icons", LockIcons, cobalt::g_state.lockAllIcons, y);
    addToggle("Character Filter", CharacterBypass, cobalt::g_state.characterFilterBypass, y);
    addToggle("Slider Bypass", SliderBypass, cobalt::g_state.sliderHack, y);
    addToggle("Verify Hack", AutoVerify, cobalt::g_state.autoVerify, y);

    // VISUAL
    addSection("VISUAL", y);

    addToggle("No Hide Player", NoHide, cobalt::g_state.noHide, y);
    addToggle("No Death Effect", NoDeathEffect, cobalt::g_state.noDeathEffect, y);
    addToggle("No Camera Shake", NoShake, cobalt::g_state.noShake, y);
    addToggle("No Transitions", NoTransitions, cobalt::g_state.noTransitions, y);

    // Footer
    auto footer = CCLabelBMFont::create(
        "COBALT MOD MENU",
        "goldFont.fnt"
    );

    if (footer) {
        footer->setScale(0.35f);
        footer->setOpacity(130);

        footer->setPosition(
            width / 2.f,
            10.f
        );

        m_panel->addChild(footer, 5);
    }
}

// --------------------------------------------------
// TOGGLE CALLBACK
// --------------------------------------------------

void CobaltMenu::onToggle(CCObject* sender) {
    auto item = static_cast<CCMenuItemToggler*>(sender);

    bool value = item->isToggled();

    switch (item->getTag()) {
        case Noclip:
            cobalt::g_state.noclip = value;
            break;

        case NoCollision:
            cobalt::g_state.noCollision = value;
            break;

        case AutoComplete:
            cobalt::g_state.autoComplete = value;
            break;

        case AutoLDM:
            cobalt::g_state.autoLDM = value;
            break;

        case CopyHack:
            cobalt::g_state.copyHack = value;
            break;

        case EditHack:
            cobalt::g_state.editHack = value;
            cobalt::applyPatches();
            break;

        case DisableQuit:
            cobalt::g_state.disableQuit = value;
            break;

        case IconBypass:
            cobalt::g_state.iconBypass = value;
            break;

        case LockIcons:
            cobalt::g_state.lockAllIcons = value;
            break;

        case CharacterBypass:
            cobalt::g_state.characterFilterBypass = value;
            cobalt::applyPatches();
            break;

        case SliderBypass:
            cobalt::g_state.sliderHack = value;
            cobalt::applyPatches();
            break;

        case AutoVerify:
            cobalt::g_state.autoVerify = value;
            break;

        case NoHide:
            cobalt::g_state.noHide = value;
            break;

        case NoDeathEffect:
            cobalt::g_state.noDeathEffect = value;
            break;

        case NoShake:
            cobalt::g_state.noShake = value;
            break;

        case NoTransitions:
            cobalt::g_state.noTransitions = value;
            cobalt::applyPatches();
            break;

        case SpeedHack:
            cobalt::g_state.speedhack = value;
            cobalt::applySpeedhack();
            break;

        case SpeedSong:
            cobalt::g_state.speedhackSong = value;
            cobalt::applySpeedhack();
            break;
    }

    cobalt::save();
}

// --------------------------------------------------
// SPEED CALLBACKS
// --------------------------------------------------

void CobaltMenu::onSpeedDown(CCObject*) {
    cobalt::g_state.speed = std::max(
        0.5f,
        cobalt::g_state.speed - 0.1f
    );

    if (m_speedLabel) {
        m_speedLabel->setString(
            fmt::format(
                "{:.2f}x",
                cobalt::g_state.speed
            ).c_str()
        );
    }

    cobalt::applySpeedhack();
    cobalt::save();
}

void CobaltMenu::onSpeedUp(CCObject*) {
    cobalt::g_state.speed = std::min(
        5.0f,
        cobalt::g_state.speed + 0.1f
    );

    if (m_speedLabel) {
        m_speedLabel->setString(
            fmt::format(
                "{:.2f}x",
                cobalt::g_state.speed
            ).c_str()
        );
    }

    cobalt::applySpeedhack();
    cobalt::save();
}

// --------------------------------------------------
// CLOSE
// --------------------------------------------------

void CobaltMenu::onClose(CCObject*) {
    this->removeFromParentAndCleanup(true);
}

// --------------------------------------------------
// TOGGLE MENU
// --------------------------------------------------

void CobaltMenu::toggle() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();

    if (!scene)
        return;

    if (auto existing = scene->getChildByID("cobalt-menu")) {
        existing->removeFromParentAndCleanup(true);
        return;
    }

    auto menu = CobaltMenu::create();

    if (!menu)
        return;

    menu->setID("cobalt-menu");

    scene->addChild(menu, 999);
}
