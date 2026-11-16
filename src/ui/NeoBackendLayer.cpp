#include "NeoBackendLayer.hpp"

#include <algorithm>
#include <sstream>
#include <string>

using namespace cocos2d;
using namespace neo::intercept;

namespace {

std::string getSourceName(RequestSource source) {
    switch (source) {
        case RequestSource::NeoBackend:
            return "NEO";

        case RequestSource::GeometryDash:
            return "GD";

        case RequestSource::Geode:
            return "GEODE";
    }

    return "UNKNOWN";
}

std::string getStateName(RequestState state) {
    switch (state) {
        case RequestState::Pending:
            return "PENDING";

        case RequestState::Completed:
            return "COMPLETED";

        case RequestState::Failed:
            return "FAILED";
    }

    return "UNKNOWN";
}

std::string shorten(
    const std::string& value,
    size_t maxLength
) {
    if (value.size() <= maxLength) {
        return value;
    }

    return value.substr(0, maxLength) + "...";
}

CCLabelTTF* makeLabel(
    const std::string& text,
    float size
) {
    return CCLabelTTF::create(
        text.c_str(),
        "Arial",
        size
    );
}

CCLabelTTF* makeWrappedLabel(
    const std::string& text,
    float size,
    float width,
    float height
) {
    return CCLabelTTF::create(
        text.c_str(),
        "Arial",
        size,
        CCSizeMake(width, height),
        kCCTextAlignmentLeft
    );
}

float getUIScale(const CCSize& size) {
    const float referenceWidth = 854.0f;
    const float referenceHeight = 480.0f;

    float scaleX = size.width / referenceWidth;
    float scaleY = size.height / referenceHeight;

    return std::max(
        0.55f,
        std::min(
            1.0f,
            std::min(scaleX, scaleY)
        )
    );
 }
 
 void layoutMenuItems(
    CCMenu* menu,
    const std::vector<CCMenuItemFont*>& items,
    float centerX,
    float centerY,
    float availableWidth,
    float baseScale,
    float gap
) {
    if (!menu || items.empty()) {
        return;
    }

    float totalWidth = 0.0f;

    for (auto item : items) {
        totalWidth += item->getContentSize().width * baseScale;
    }

    totalWidth += gap * (items.size() - 1);

    float fit = 1.0f;

    if (totalWidth > availableWidth) {
        fit = availableWidth / totalWidth;
    }

    float actualWidth = totalWidth * fit;
    float cursor = -actualWidth / 2.0f;

    for (auto item : items) {
        float scale = baseScale * fit;

        item->setScale(scale);

        float width =
            item->getContentSize().width * scale;

        item->setPosition(
            ccp(cursor + width / 2.0f, 0)
        );

        cursor += width + gap * fit;
    }

    menu->setPosition(
        ccp(centerX, centerY)
    );
  }

}

// ==========================================
// CREATE
// ==========================================

NeoBackendLayer* NeoBackendLayer::create() {
    auto ret = new NeoBackendLayer();

    if (ret && ret->init()) {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
}

// ==========================================
// OPEN FLOATING WINDOW
// ==========================================

void NeoBackendLayer::open(CCNode* parent) {
    if (!parent) {
        return;
    }

    if (parent->getChildByID("neo-inspector-window")) {
        return;
    }

    auto layer = NeoBackendLayer::create();

    if (!layer) {
        return;
    }

    layer->setID("neo-inspector-window");

    parent->addChild(layer, 10000);
}

// ==========================================
// INIT
// ==========================================

bool NeoBackendLayer::init() {
    if (!CCLayer::init()) {
        return false;
    }

setTouchEnabled(true);

    auto winSize =
        CCDirector::sharedDirector()->getWinSize();

    m_uiScale = getUIScale(winSize);

    const float s = m_uiScale;

    // Modal dimensions.

    const float margin = 12.0f * s;

    m_panelWidth = std::max(
    0.0f,
    winSize.width - margin * 2.0f
    );

    m_panelHeight = std::max(
    0.0f,
    winSize.height - margin * 2.0f
    );

    m_panelWidth = std::min(
    m_panelWidth,
    1000.0f
    );

    m_panelHeight = std::min(
    m_panelHeight,
    680.0f
    );
    
    // Dark overlay.

    auto background = CCLayerColor::create(
        ccc4(0, 0, 0, 175)
    );

    if (!background) {
        return false;
    }

    addChild(background, 0);

    // Main panel.

    m_panel = CCLayerColor::create(
        ccc4(13, 18, 32, 250),
        m_panelWidth,
        m_panelHeight
    );

    if (!m_panel) {
        return false;
    }

m_panel->setAnchorPoint(ccp(0, 0));

m_panel->setPosition(
    ccp(
        (winSize.width - m_panelWidth) / 2.0f,
        (winSize.height - m_panelHeight) / 2.0f
    )
);

    addChild(m_panel, 1);

    const float W = m_panelWidth;
    const float H = m_panelHeight;

    // Title.

    auto title = makeLabel(
        "NEO BACKEND INSPECTOR",
        22.0f * s
    );

    title->setPosition(
        ccp(W / 2, H - 30.0f * s)
    );

    m_panel->addChild(title);

    // Status.

    m_statusLabel = makeLabel(
        "LIVE MONITOR | READY",
        10.0f * s
    );

    m_statusLabel->setPosition(
        ccp(W / 2, H - 53.0f * s)
    );

    m_panel->addChild(m_statusLabel);

    // Close button.

    auto closeItem = CCMenuItemFont::create(
        "X",
        this,
        menu_selector(NeoBackendLayer::onClose)
    );

    closeItem->setScale(1.2f * s);

auto closeMenu = CCMenu::create(
    closeItem,
    nullptr
);



    closeMenu->setPosition(
        ccp(W - 28.0f * s, H - 28.0f * s)
    );

    m_panel->addChild(closeMenu);
m_internalMenus.push_back(closeMenu);


// ==========================================
// FILTER BUTTONS
// ==========================================

struct FilterButton {
    const char* name;
    SEL_MenuHandler callback;
};

FilterButton filters[] = {
    {"ALL", menu_selector(NeoBackendLayer::onFilterAll)},
    {"NEO", menu_selector(NeoBackendLayer::onFilterNeo)},
    {"GD", menu_selector(NeoBackendLayer::onFilterGD)},
    {"GEODE", menu_selector(NeoBackendLayer::onFilterGeode)}
};

auto filterMenu = CCMenu::create();

std::vector<CCMenuItemFont*> filterItems;

for (const auto& filter : filters) {
    auto item = CCMenuItemFont::create(
        filter.name,
        this,
        filter.callback
    );

    if (!item) {
        continue;
    }

    filterItems.push_back(item);
    filterMenu->addChild(item);
}

const float separatorX = W * 0.40f;

layoutMenuItems(
    filterMenu,
    filterItems,
    separatorX / 2.0f,
    H - 88.0f * s,
    separatorX - 30.0f * s,
    0.65f * s,
    15.0f * s
);

m_panel->addChild(filterMenu);
m_internalMenus.push_back(filterMenu);

    // Column separator.

    auto separator = CCLayerColor::create(
        ccc4(65, 85, 115, 180),
        1.0f * s,
        H - 125.0f * s
    );

    separator->setPosition(
        ccp(W * 0.40f, 15.0f * s)
    );

    m_panel->addChild(separator);

// ==========================================
// DETAIL TABS
// ==========================================

struct DetailTab {
    const char* name;
    SEL_MenuHandler callback;
};

DetailTab tabs[] = {
    {"OVERVIEW", menu_selector(NeoBackendLayer::onTabOverview)},
    {"HEADERS", menu_selector(NeoBackendLayer::onTabHeaders)},
    {"BODY", menu_selector(NeoBackendLayer::onTabBody)},
    {"RESPONSE", menu_selector(NeoBackendLayer::onTabResponse)}
};

auto tabMenu = CCMenu::create();

std::vector<CCMenuItemFont*> tabItems;

for (const auto& tab : tabs) {
    auto item = CCMenuItemFont::create(
        tab.name,
        this,
        tab.callback
    );

    if (!item) {
        continue;
    }

    tabItems.push_back(item);
    tabMenu->addChild(item);
}

const float detailLeft = separatorX + 12.0f * s;
const float detailRight = W - 12.0f * s;

layoutMenuItems(
    tabMenu,
    tabItems,
    (detailLeft + detailRight) / 2.0f,
    H - 88.0f * s,
    detailRight - detailLeft,
    0.60f * s,
    10.0f * s
);

m_panel->addChild(tabMenu);
m_internalMenus.push_back(tabMenu);

    // Details label with automatic line wrapping.
    
const float detailX = separatorX + 14.0f * s;

const float detailWidth =
    W - detailX - 15.0f * s;

const float detailHeight =
    H - 145.0f * s;

m_detailsLabel = makeWrappedLabel(
    "Select a request to inspect its details.",
    10.0f * s,
    detailWidth,
    detailHeight
);

if (!m_detailsLabel) {
    return false;
}

m_detailsLabel->setAnchorPoint(ccp(0, 1));

m_detailsLabel->setPosition(
    ccp(
        detailX,
        H - 115.0f * s
    )
);

m_panel->addChild(m_detailsLabel);

    // Request list container.

    m_listContainer = CCNode::create();

    m_listContainer->setTag(1000);

    m_panel->addChild(m_listContainer);

    // Live listener.

    std::weak_ptr<std::atomic<bool>> weakDirty =
        m_dirtySignal;

    m_listenerID =
        NeoActivityManager::get().addListener(
            [weakDirty]() {
                if (auto dirty = weakDirty.lock()) {
                    dirty->store(true);
                }
            }
        );

    m_dirtySignal->store(true);

    scheduleUpdate();

    return true;

}

// ==========================================
// UPDATE
// ==========================================

void NeoBackendLayer::update(float dt) {
    CCLayer::update(dt);

    if (m_dirtySignal && m_dirtySignal->exchange(false)) {
        refreshList();
        refreshDetails();
    }
}

// ==========================================
// REFRESH REQUEST LIST
// ==========================================

void NeoBackendLayer::refreshList() {
    if (!m_listContainer) {
        return;
    }

    m_listContainer->removeAllChildrenWithCleanup(true);

    auto history =
        NeoActivityManager::get().getHistory();

    std::reverse(
        history.begin(),
        history.end()
    );

    m_visibleIDs.clear();

    const float s = m_uiScale;

    const float W = m_panelWidth;
    const float H = m_panelHeight;

const float listWidth = W * 0.40f - 25.0f * s;

    float y = H - 120.0f * s;

    const int maxRows = std::max(
        4,
        std::min(
            12,
            static_cast<int>(
                (H - 150.0f * s) / (29.0f * s)
            )
        )
    );

    int displayed = 0;

    for (const auto& request : history) {
        bool matches = false;

        switch (m_filter) {
            case 0:
                matches = true;
                break;

            case 1:
                matches =
                    request.source == RequestSource::NeoBackend;
                break;

            case 2:
                matches =
                    request.source == RequestSource::GeometryDash;
                break;

            case 3:
                matches =
                    request.source == RequestSource::Geode;
                break;
        }

        if (!matches) {
            continue;
        }

        if (displayed >= maxRows) {
            break;
        }

        m_visibleIDs.push_back(request.id);

        std::stringstream row;

        row << "#"
            << request.id
            << " | "
            << getSourceName(request.source)
            << " | "
            << request.method
            << " | "
            << shorten(request.url, 24);

auto item = CCMenuItemFont::create(
    row.str().c_str(),
    this,
    menu_selector(NeoBackendLayer::onSelectRequest)
);

if (!item) {
    continue;
}

item->setTag(displayed);
item->setAnchorPoint(ccp(0, 0.5f));

// Tamaño máximo permitido para la fila.
const float availableRowWidth = listWidth - 15.0f * s;

const float baseRowScale = 0.55f * s;

float rowScale = baseRowScale;

const float naturalWidth = item->getContentSize().width;

if (naturalWidth * rowScale > availableRowWidth) {
    rowScale = availableRowWidth / naturalWidth;
}

item->setScale(rowScale);

auto menu = CCMenu::create(
    item,
    nullptr
);


menu->setPosition(
    ccp(
        15.0f * s,
        y
    )
);

m_listContainer->addChild(menu);

// El menú ya está añadido y registrado.
menu->setHandlerPriority(-130);

        y -= 29.0f * s;

        displayed++;
    }

    if (m_selectedID == 0 && !m_visibleIDs.empty()) {
        m_selectedID = m_visibleIDs.front();
    }

    if (m_visibleIDs.empty()) {
        auto empty = makeLabel(
            "No requests found.",
            10.0f * s
        );

        empty->setPosition(
            ccp(
                listWidth / 2,
                H / 2
            )
        );

        m_listContainer->addChild(empty);
    }

    std::stringstream status;

    status << "LIVE MONITOR | "
           << history.size()
           << " REQUESTS";

    if (m_statusLabel) {
        m_statusLabel->setString(
            status.str().c_str()
        );
    }
}

// ==========================================
// REQUEST DETAILS
// ==========================================

void NeoBackendLayer::refreshDetails() {
    if (!m_detailsLabel) {
        return;
    }

    if (m_selectedID == 0) {
        m_detailsLabel->setString(
            "Select a request to inspect its details."
        );

        return;
    }

    auto history =
        NeoActivityManager::get().getHistory();

    auto it = std::find_if(
        history.begin(),
        history.end(),
        [this](const NeoRequest& request) {
            return request.id == m_selectedID;
        }
    );

    if (it == history.end()) {
        m_selectedID = 0;

        m_detailsLabel->setString(
            "Selected request is no longer available."
        );

        return;
    }

    const auto& request = *it;

    std::stringstream details;

    switch (m_detailTab) {

        // ==================================
        // OVERVIEW
        // ==================================

        case 0: {
            details
                << "REQUEST #" << request.id << "\n\n"
                << "SOURCE: "
                << getSourceName(request.source)
                << "\n\n"

                << "METHOD: "
                << request.method
                << "\n\n"

                << "FULL URL:\n"
                << request.url
                << "\n\n"

                << "STATE: "
                << getStateName(request.state)
                << "\n\n"

                << "HTTP STATUS: "
                << (
                    request.responseCode > 0
                    ? std::to_string(request.responseCode)
                    : "Not captured"
                )
                << "\n\n"

                << "DURATION: "
                << request.durationMs
                << " ms";

            if (!request.error.empty()) {
                details
                    << "\n\nERROR:\n"
                    << request.error;
            }

            break;
        }

        // ==================================
        // REQUEST HEADERS
        // ==================================

        case 1: {
            details << "REQUEST HEADERS\n\n";

            if (request.requestHeaders.empty()) {
                details << "No request headers captured.";
            }
            else {
                for (const auto& [key, value] : request.requestHeaders) {
                    details
                        << key
                        << ": "
                        << value
                        << "\n\n";
                }
            }

            break;
        }

        // ==================================
        // REQUEST BODY
        // ==================================

        case 2: {
            details << "REQUEST BODY\n\n";

            if (request.requestBody.empty()) {
                details << "No request body captured.";
            }
            else {
                details << request.requestBody;
            }

            break;
        }

        // ==================================
        // RESPONSE
        // ==================================

        case 3: {
            details
                << "RESPONSE INFORMATION\n\n"

                << "HTTP STATUS: "
                << (
                    request.responseCode > 0
                    ? std::to_string(request.responseCode)
                    : "Not captured"
                )
                << "\n\n"

                << "RESPONSE HEADERS:\n\n";

            if (request.responseHeaders.empty()) {
                details << "No response headers captured.\n\n";
            }
            else {
                for (const auto& [key, value] : request.responseHeaders) {
                    details
                        << key
                        << ": "
                        << value
                        << "\n\n";
                }
            }

            details << "RESPONSE BODY:\n\n";

            if (request.responseBody.empty()) {
                details << "No response body captured.";
            }
            else {
                details << request.responseBody;
            }

            if (!request.error.empty()) {
                details
                    << "\n\nERROR:\n"
                    << request.error;
            }

            break;
        }
    }

    m_detailsLabel->setString(
        details.str().c_str()
    );
}

// ==========================================
// SELECT REQUEST
// ==========================================

void NeoBackendLayer::onSelectRequest(
    CCObject* sender
) {
    auto item =
        static_cast<CCMenuItemFont*>(sender);

    const int index = item->getTag();

    if (
        index < 0 ||
        static_cast<size_t>(index) >= m_visibleIDs.size()
    ) {
        return;
    }

    m_selectedID = m_visibleIDs[index];

    refreshDetails();
}

// ==========================================
// FILTERS
// ==========================================

void NeoBackendLayer::onFilterAll(CCObject*) {
    m_filter = 0;
    m_selectedID = 0;
    m_dirtySignal->store(true);
}

void NeoBackendLayer::onFilterNeo(CCObject*) {
    m_filter = 1;
    m_selectedID = 0;
    m_dirtySignal->store(true);
}

void NeoBackendLayer::onFilterGD(CCObject*) {
    m_filter = 2;
    m_selectedID = 0;
    m_dirtySignal->store(true);
}

void NeoBackendLayer::onFilterGeode(CCObject*) {
    m_filter = 3;
    m_selectedID = 0;
    m_dirtySignal->store(true);
}

// ==========================================
// DETAIL TABS
// ==========================================

void NeoBackendLayer::onTabOverview(CCObject*) {
    m_detailTab = 0;
    refreshDetails();
}

void NeoBackendLayer::onTabHeaders(CCObject*) {
    m_detailTab = 1;
    refreshDetails();
}

void NeoBackendLayer::onTabBody(CCObject*) {
    m_detailTab = 2;
    refreshDetails();
}

void NeoBackendLayer::onTabResponse(CCObject*) {
    m_detailTab = 3;
    refreshDetails();
}

// ==========================================
// CLOSE
// ==========================================
void NeoBackendLayer::onClose(CCObject*) {
    removeFromParentAndCleanup(true);
}
// ==========================================
// CLEANUP
// ==========================================

void NeoBackendLayer::onExit() {
    if (m_listenerID != 0) {
        NeoActivityManager::get().removeListener(
            m_listenerID
        );

        m_listenerID = 0;
    }

    m_dirtySignal.reset();

    CCLayer::onExit();
}

NeoBackendLayer::~NeoBackendLayer() = default;

// ==========================================
// MODAL TOUCH BLOCKING
// ==========================================

void NeoBackendLayer::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()
        ->getTouchDispatcher()
        ->addTargetedDelegate(
            this,
            -129,
            true
        );
}

bool NeoBackendLayer::ccTouchBegan(
    CCTouch* touch,
    CCEvent* event
) {
    return true;
}

void NeoBackendLayer::ccTouchMoved(
    CCTouch* touch,
    CCEvent* event
) {}

void NeoBackendLayer::ccTouchEnded(
    CCTouch* touch,
    CCEvent* event
) {}

void NeoBackendLayer::ccTouchCancelled(
    CCTouch* touch,
    CCEvent* event
) {}
