#include <Geode/Geode.hpp>
#include <Geode/modify/LevelSearchLayer.hpp>
#include <Geode/utils/general.hpp>

#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#ifndef GEODE_IS_IOS
using namespace geode::prelude;

namespace neo::searchHistory {

    static constexpr size_t MAX_HISTORY = 20;
    static constexpr size_t MAX_VISIBLE = 8;
    static constexpr size_t MAX_LABEL_LENGTH = 32;

    // --------------------------------------------------
    // Storage
    // --------------------------------------------------

    static std::string getHistoryPath() {
        return (Mod::get()->getSaveDir() / "history.txt").string();
    }

    static std::vector<std::string> getHistory() {
        std::vector<std::string> history;

        std::ifstream file(getHistoryPath());
        if (!file.is_open())
            return history;

        std::string line;

        while (std::getline(file, line)) {
            if (!line.empty()) {
                history.push_back(line);
            }

            if (history.size() >= MAX_HISTORY)
                break;
        }

        return history;
    }

    static void saveHistory(
        std::vector<std::string> const& history
    ) {
        std::ofstream file(getHistoryPath(), std::ios::trunc);

        if (!file.is_open())
            return;

        for (auto const& entry : history) {
            file << entry << '\n';
        }
    }

    // --------------------------------------------------
    // History management
    // --------------------------------------------------

    static void pushHistory(std::string const& text) {
        if (text.empty())
            return;

        // Ignore whitespace-only searches.
        if (text.find_first_not_of(" \t\r\n") == std::string::npos)
            return;

        auto history = getHistory();

        // Remove previous occurrence.
        history.erase(
            std::remove(history.begin(), history.end(), text),
            history.end()
        );

        // Insert newest search first.
        history.insert(history.begin(), text);

        // Limit history size.
        if (history.size() > MAX_HISTORY)
            history.resize(MAX_HISTORY);

        saveHistory(history);
    }

    static void clearHistory() {
        saveHistory({});
    }

    // --------------------------------------------------
    // Popup
    // --------------------------------------------------

    class SearchHistoryPopup : public FLAlertLayer {
    protected:
        std::vector<std::string> m_history;
        CCMenu* m_listMenu = nullptr;

        bool init(std::vector<std::string> history) {
            if (!FLAlertLayer::init(150))
                return false;

            m_history = std::move(history);

            auto winSize = CCDirector::get()->getWinSize();
            auto center = winSize / 2.f;

            // Background
            auto bg = CCScale9Sprite::create("GJ_square01.png");

            if (!bg)
                return false;

            bg->setContentSize({300.f, 250.f});
            bg->setPosition(center);
            m_mainLayer->addChild(bg);

            // Title
            auto title = CCLabelBMFont::create(
                "Search History",
                "goldFont.fnt"
            );

            title->setPosition({
                center.width,
                center.height + 105.f
            });

            m_mainLayer->addChild(title);

            // List menu
            m_listMenu = CCMenu::create();
            m_listMenu->setPosition({
                center.width,
                center.height + 20.f
            });

            m_mainLayer->addChild(m_listMenu);

            // Close button
            auto closeSprite = CCSprite::createWithSpriteFrameName(
                "GJ_closeBtn_001.png"
            );

            if (closeSprite) {
                auto closeBtn = CCMenuItemSpriteExtra::create(
                    closeSprite,
                    this,
                    menu_selector(SearchHistoryPopup::onClose)
                );

                auto closeMenu = CCMenu::create();

                closeMenu->setPosition({
                    center.width - 140.f,
                    center.height + 115.f
                });

                closeMenu->addChild(closeBtn);
                m_mainLayer->addChild(closeMenu);
            }

            // Clear button
            auto clearSprite = ButtonSprite::create(
                "Clear All",
                "goldFont.fnt",
                "GJ_button_01.png",
                0.8f
            );

            if (clearSprite) {
                auto clearBtn = CCMenuItemSpriteExtra::create(
                    clearSprite,
                    this,
                    menu_selector(SearchHistoryPopup::onClear)
                );

                auto clearMenu = CCMenu::create();

                clearMenu->setPosition({
                    center.width,
                    center.height - 100.f
                });

                clearMenu->addChild(clearBtn);
                m_mainLayer->addChild(clearMenu);
            }

            refreshList();

            this->setTouchEnabled(true);
            this->setKeypadEnabled(true);

            CCDirector::get()->getTouchDispatcher()
                ->addTargetedDelegate(this, -500, true);

            return true;
        }

        // --------------------------------------------------
        // Refresh list
        // --------------------------------------------------

        void refreshList() {
            if (!m_listMenu)
                return;

            m_listMenu->removeAllChildren();

            if (m_history.empty()) {
                auto emptyLabel = CCLabelBMFont::create(
                    "No search history",
                    "bigFont.fnt"
                );

                if (emptyLabel) {
                    emptyLabel->setScale(0.55f);
                    emptyLabel->setColor({180, 180, 180});
                    emptyLabel->setPosition({0.f, 0.f});

                    m_listMenu->addChild(emptyLabel);
                }

                return;
            }

            float y = 65.f;

            size_t count = 0;

            for (auto const& entry : m_history) {
                if (count >= MAX_VISIBLE)
                    break;

                // Keep the original query in the button ID.
                std::string display = entry;

                if (display.size() > MAX_LABEL_LENGTH) {
                    display.resize(MAX_LABEL_LENGTH - 3);
                    display += "...";
                }

                auto label = CCLabelBMFont::create(
                    display.c_str(),
                    "bigFont.fnt"
                );

                if (!label) {
                    y -= 22.f;
                    count++;
                    continue;
                }

                label->setScale(0.5f);

                auto btn = CCMenuItemSpriteExtra::create(
                    label,
                    this,
                    menu_selector(SearchHistoryPopup::onSelect)
                );

                btn->setID(entry);
                btn->setPosition({0.f, y});

                m_listMenu->addChild(btn);

                y -= 22.f;
                count++;
            }
        }

        // --------------------------------------------------
        // Select search
        // --------------------------------------------------

        void onSelect(CCObject* sender) {
            if (!sender)
                return;

            auto node = static_cast<CCNode*>(sender);
            auto query = node->getID();

            if (query.empty())
                return;

            auto scene = CCDirector::get()->getRunningScene();

            if (!scene)
                return;

            auto searchLayer = typeinfo_cast<LevelSearchLayer*>(
                scene->getChildByIDRecursive("LevelSearchLayer")
            );

            if (!searchLayer)
                return;

            auto input = typeinfo_cast<CCTextInputNode*>(
                searchLayer->getChildByID("search-bar")
            );

            if (!input)
                return;

            input->setString(query);

            // Close popup.
            onClose(nullptr);

            // Execute search.
            searchLayer->onSearch(nullptr);
        }

        // --------------------------------------------------
        // Clear history
        // --------------------------------------------------

        void onClear(CCObject*) {
            m_history.clear();

            clearHistory();
            refreshList();
        }

        // --------------------------------------------------
        // Close popup
        // --------------------------------------------------

        void onClose(CCObject*) {
            CCDirector::get()->getTouchDispatcher()
                ->removeDelegate(this);

            this->removeFromParentAndCleanup(true);
        }

        void keyBackClicked() override {
            onClose(nullptr);
        }

    public:
        static SearchHistoryPopup* create(
            std::vector<std::string> history
        ) {
            auto ret = new SearchHistoryPopup();

            if (ret && ret->init(std::move(history))) {
                ret->autorelease();
                return ret;
            }

            CC_SAFE_DELETE(ret);
            return nullptr;
        }
    };

    // --------------------------------------------------
    // LevelSearchLayer Hook
    // --------------------------------------------------

    class $modify(HistorySearchLayer, LevelSearchLayer) {

        bool init(int p0) {
            if (!LevelSearchLayer::init(p0))
                return false;

            auto winSize = CCDirector::get()->getWinSize();

            // History menu
            auto menu = CCMenu::create();

            menu->setID("history-menu"_spr);

            menu->setPosition({
                winSize.width / 2.f - 195.f,
                winSize.height / 2.f + 115.f
            });

this->addChild(menu, 100);

            // History icon
            auto sprite = CCSprite::createWithSpriteFrameName(
                "GJ_timeIcon_001.png"
            );

            if (!sprite)
                return true;

            auto btn = CCMenuItemSpriteExtra::create(
                sprite,
                this,
                menu_selector(HistorySearchLayer::onHistory)
            );

            btn->setScale(0.7f);

            menu->addChild(btn);

            return true;
        }

        // Open history popup
        void onHistory(CCObject*) {
            auto scene = CCDirector::get()->getRunningScene();

            if (!scene)
                return;

            auto popup = SearchHistoryPopup::create(getHistory());

            if (popup) {
                scene->addChild(
                    popup,
                    scene->getHighestChildZ() + 1
                );
            }
        }

        // Save searches
        void onSearch(CCObject* sender) {
            auto input = typeinfo_cast<CCTextInputNode*>(
                this->getChildByID("search-bar")
            );

            if (input) {
                pushHistory(input->getString());
            }

            LevelSearchLayer::onSearch(sender);
        }
    };

}
#endif
