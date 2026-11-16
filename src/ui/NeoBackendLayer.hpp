#pragma once

#include <Geode/Geode.hpp>

#include <memory>
#include <atomic>
#include <vector>
#include <cstdint>

#include "../activity/NeoActivityManager.hpp"

class NeoBackendLayer : public cocos2d::CCLayer {
protected:
    std::shared_ptr<std::atomic<bool>> m_dirtySignal =
        std::make_shared<std::atomic<bool>>(true);

    uint64_t m_listenerID = 0;
    uint64_t m_selectedID = 0;

    int m_filter = 0;
    int m_detailTab = 0;

    float m_uiScale = 1.0f;
    float m_panelWidth = 0.0f;
    float m_panelHeight = 0.0f;

    std::vector<uint64_t> m_visibleIDs;

    cocos2d::CCLayerColor* m_panel = nullptr;
    cocos2d::CCNode* m_listContainer = nullptr;

    cocos2d::CCLabelTTF* m_statusLabel = nullptr;
    cocos2d::CCLabelTTF* m_detailsLabel = nullptr;

    bool init() override;
    void update(float dt) override;
void onEnter() override;
std::vector<cocos2d::CCMenu*> m_internalMenus;
void onEnterTransitionDidFinish() override;

bool ccTouchBegan(
    cocos2d::CCTouch* touch,
    cocos2d::CCEvent* event
) override;

void ccTouchMoved(
    cocos2d::CCTouch* touch,
    cocos2d::CCEvent* event
) override;

void ccTouchEnded(
    cocos2d::CCTouch* touch,
    cocos2d::CCEvent* event
) override;

void ccTouchCancelled(
    cocos2d::CCTouch* touch,
    cocos2d::CCEvent* event
) override;

    void onExit() override;

    void refreshList();
    void refreshDetails();

    void onClose(cocos2d::CCObject*);
    void onSelectRequest(cocos2d::CCObject*);

    void onFilterAll(cocos2d::CCObject*);
    void onFilterNeo(cocos2d::CCObject*);
    void onFilterGD(cocos2d::CCObject*);
    void onFilterGeode(cocos2d::CCObject*);

    void onTabOverview(cocos2d::CCObject*);
    void onTabHeaders(cocos2d::CCObject*);
    void onTabBody(cocos2d::CCObject*);
    void onTabResponse(cocos2d::CCObject*);

public:
    ~NeoBackendLayer() override;

    static NeoBackendLayer* create();
    static void open(cocos2d::CCNode* parent);
};
