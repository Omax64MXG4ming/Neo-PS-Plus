#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/utils/general.hpp>
#include <vector>
#include <algorithm>
#include <fstream>

using namespace geode::prelude;

std::vector<std::string> getHistory() {
    auto path = (Mod::get()->getSaveDir() / "history.txt").string();

    std::vector<std::string> history;
    std::ifstream file(path);

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty())
            history.push_back(line);
    }

    return history;
}

void saveHistory(std::vector<std::string> const& history) {
    auto path = (Mod::get()->getSaveDir() / "history.txt").string();

    std::ofstream file(path);

    if (!file.is_open())
        return;

    for (auto const& str : history) {
        file << str << "\n";
    }
}

void pushHistory(std::string const& text) {
    if (text.empty()) return;
    auto history = getHistory();
    history.erase(std::remove(history.begin(), history.end(), text), history.end());
    history.insert(history.begin(), text);
    if (history.size() > 20) history.pop_back();
    saveHistory(history);
}

class SearchHistoryPopup : public FLAlertLayer {
protected:
    std::vector<std::string> m_history;
    CCMenu* m_listMenu;

    bool init(std::vector<std::string> history) {
        if (!FLAlertLayer::init(150)) return false;

        m_history = history;
        auto winSize = CCDirector::get()->getWinSize();

        auto bg = CCScale9Sprite::create("GJ_square01.png", { 0, 0, 80, 80 });
        bg->setContentSize({ 300.f, 250.f });
        bg->setPosition(winSize / 2);
        m_mainLayer->addChild(bg);

        auto title = CCLabelBMFont::create("Search History", "goldFont.fnt");
        title->setPosition({ winSize.width / 2, winSize.height / 2 + 105.f });
        m_mainLayer->addChild(title);

        m_listMenu = CCMenu::create();
        m_listMenu->setPosition({ winSize.width / 2.f, winSize.height / 2.f + 20.f });
        m_mainLayer->addChild(m_listMenu);

        auto closeBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
            this,
            menu_selector(SearchHistoryPopup::onClose)
        );
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition({ winSize.width / 2 - 140.f, winSize.height / 2 + 115.f });
        closeMenu->addChild(closeBtn);
        m_mainLayer->addChild(closeMenu);

        this->refreshList();

        auto clearBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Clear All", "goldFont.fnt", "GJ_button_01.png", .8f),
            this,
            menu_selector(SearchHistoryPopup::onClear)
        );
        auto clearMenu = CCMenu::create();
        clearMenu->setPosition({ winSize.width / 2, winSize.height / 2 - 100.f });
        clearMenu->addChild(clearBtn);
        m_mainLayer->addChild(clearMenu);

        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);
        CCDirector::get()->getTouchDispatcher()->addTargetedDelegate(this, -500, true);

        return true;
    }

    void refreshList() {
        m_listMenu->removeAllChildren();
        float y = 60.f;
        int count = 0;
        for (auto const& str : m_history) {
            if (count > 7) break;
            auto lbl = CCLabelBMFont::create(str.c_str(), "bigFont.fnt");
            lbl->setScale(0.5f);
            auto btn = CCMenuItemSpriteExtra::create(
                lbl,
                this,
                menu_selector(SearchHistoryPopup::onSelect)
            );
            btn->setID(str);
            btn->setPosition({ 0.f, y });
            m_listMenu->addChild(btn);
            y -= 22.f;
            count++;
        }
    }
 // reason i made this mod is since i keep trying to search denouement or something and missspell it lmao
    void onSelect(CCObject* sender) {
        auto str = static_cast<CCNode*>(sender)->getID();
        auto scene = CCDirector::get()->getRunningScene();
        LevelSearchLayer* searchLayer = typeinfo_cast<LevelSearchLayer*>(scene->getChildByIDRecursive("LevelSearchLayer"));
        if (!searchLayer) return;

        CCTextInputNode* input = typeinfo_cast<CCTextInputNode*>(searchLayer->getChildByID("search-bar"));
        if (input) input->setString(str);

        this->onClose(nullptr);

        pushHistory(str);
        searchLayer->onSearch(nullptr);
    }

    void onClear(CCObject*) {
        m_history.clear();
        saveHistory(m_history);
        this->refreshList();
    }

    void onClose(CCObject*) {
        CCDirector::get()->getTouchDispatcher()->removeDelegate(this);
        this->removeFromParentAndCleanup(true);
    }

    void keyBackClicked() override {
        this->onClose(nullptr);
    }

public:
    static SearchHistoryPopup* create(std::vector<std::string> history) {
        auto ret = new SearchHistoryPopup();
        if (ret && ret->init(history)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class $modify(HistorySearchLayer, LevelSearchLayer) {
    bool init(int p0) {
        if (!LevelSearchLayer::init(p0)) return false;

        auto winSize = CCDirector::get()->getWinSize();

        auto menu = CCMenu::create();
        menu->setID("history-menu"_spr);
        menu->setPosition({ winSize.width / 2.f - 195.f, winSize.height / 2.f + 115.f });

        auto scene = CCDirector::get()->getRunningScene();
        this->addChild(menu, scene->getHighestChildZ() + 1);

        auto btn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png"), // i mean? do i need to use a custom icon for this?
            this,
            menu_selector(HistorySearchLayer::onHistory)
        );
        btn->setScale(0.7f);
        menu->addChild(btn);

        return true;
    }

    void onHistory(CCObject*) {
        auto history = getHistory();
        auto p = SearchHistoryPopup::create(history);
        CCDirector::get()->getRunningScene()->addChild(p, CCDirector::get()->getRunningScene()->getHighestChildZ() + 1);
    }

    void onSearch(CCObject* sender) {
        CCTextInputNode* input = typeinfo_cast<CCTextInputNode*>(this->getChildByID("search-bar"));
        if (input) {
            std::string text = input->getString();
            pushHistory(text);
        }
        LevelSearchLayer::onSearch(sender);
    }
};
