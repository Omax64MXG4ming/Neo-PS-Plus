#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

class CobaltMenu : public CCLayer {
protected:
    CCNode* m_panel = nullptr;
    CCNode* m_content = nullptr;
    CCMenu* m_navigation = nullptr;

    int m_currentPage = 0;

    bool init() override;

    void createInterface();
    void createNavigation();
    void showPage(int page);

    void onClose(CCObject*);
    void onNavigation(CCObject*);

public:
    static CobaltMenu* create();
    void toggle();
};
