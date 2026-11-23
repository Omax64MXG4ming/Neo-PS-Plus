#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/ScrollLayer.hpp>

using namespace geode::prelude;

class CobaltMenu : public CCLayer {
protected:
    CCNode* m_panel = nullptr;
    ScrollLayer* m_scroll = nullptr;
    CCLabelBMFont* m_speedLabel = nullptr;

    bool init() override;

    void buildMenu();
    void buildOptions();

    void addSection(const char* title, float& y);
    void addToggle(const char* title, int id, bool value, float& y);

    void onToggle(CCObject* sender);
    void onSpeedDown(CCObject*);
    void onSpeedUp(CCObject*);
    void onClose(CCObject*);

public:
    static CobaltMenu* create();
    static void toggle();
};
