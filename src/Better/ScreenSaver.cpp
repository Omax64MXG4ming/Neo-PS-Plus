#include <Geode/Geode.hpp>
#include <Geode/modify/CCScene.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>

#ifndef GEODE_IS_IOS
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#endif

#include "../BlurAPI.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
#include <vector>

using namespace geode::prelude;
using namespace cocos2d;

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float FIXED_STEP = 1.f / 60.f;
constexpr float GRAVITY = 1176.f;
constexpr float RESTITUTION = 0.5f;
constexpr float FRICTION = 0.65f;

struct Vec {
    float x = 0.f;
    float y = 0.f;
};

static Vec add(Vec a, Vec b) {
    return {a.x + b.x, a.y + b.y};
}

static Vec sub(Vec a, Vec b) {
    return {a.x - b.x, a.y - b.y};
}

static Vec mul(Vec a, float s) {
    return {a.x * s, a.y * s};
}

static float dot(Vec a, Vec b) {
    return a.x * b.x + a.y * b.y;
}

static float length(Vec a) {
    return std::sqrt(dot(a, a));
}

static Vec normalize(Vec a) {
    float l = length(a);

    if (l <= 0.0001f)
        return {1.f, 0.f};

    return mul(a, 1.f / l);
}

static Vec rotate(Vec p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    return {
        p.x * c - p.y * s,
        p.x * s + p.y * c
    };
}

static Vec perpendicular(Vec p) {
    return {-p.y, p.x};
}

static float clampf(float v, float a, float b) {
    return std::max(a, std::min(v, b));
}

static float randomFloat(float min, float max) {
    return min + (max - min) *
        (static_cast<float>(std::rand()) / RAND_MAX);
}

} // namespace

// --------------------------------------------------
// Global state
// --------------------------------------------------

static float idleTimer = 0.f;
static bool ssActive = false;
static bool pauseOpen = false;

// --------------------------------------------------
// Physics item
// --------------------------------------------------

struct PhysItem {
    CCNode* vis = nullptr;

    Vec position;
    Vec velocity;

    float radius = 16.f;
    float angle = 0.f;
    float angularVelocity = 0.f;

    int itemType = -1;

    bool isPlayerCube = false;
    bool isBox = false;

    float invMass = 1.f;
};

// --------------------------------------------------
// Screensaver
// --------------------------------------------------

class ScreensaverLayer : public CCLayerColor {

    std::vector<PhysItem> items;

    float elapsed = 0.f;
    int itemsMade = 0;

    bool playerIn = false;
    bool draining = false;
    bool filled = false;
    bool floorEnabled = true;
    float physicsLag = 0.f;
    float pauseBeforeDrain = 0.f;
    float nextSpawn = 0.f;

    // Configuration

    int maxItems = 0;
    float physicsSpeed = 1.f;
    int playerChance = 0;

    bool allowDrop = false;
    int spawnTicks = 1;

    // Background

    std::string bgType;
    std::string bgFit;

    ccColor3B bgCol;
    int bgAlpha = 128;

    CCSprite* bgSprite = nullptr;

    // Animation

    float fadeIn = 0.f;

    bool doneFade = false;
    bool killed = false;

    CCSize winSize;

    static const int orbIds[11];
    static const float orbSizes[11];

    // --------------------------------------------------
    // Visual fallback
    // --------------------------------------------------

    CCNode* createFallbackVisual(
        float radius,
        bool player,
        int orbIdx
    ) {

        ccColor3B color;

        if (player) {
            color = ccc3(60, 170, 255);
        } else {
            static const ccColor3B colors[11] = {
                {255, 190, 50},
                {255, 90, 90},
                {80, 220, 140},
                {160, 100, 255},
                {255, 120, 220},
                {80, 220, 255},
                {255, 220, 80},
                {120, 160, 255},
                {255, 140, 70},
                {160, 255, 100},
                {220, 220, 220}
            };

            color = colors[
                std::max(0, std::min(orbIdx, 10))
            ];
        }

        float size = radius * 2.f;

        auto visual = CCLayerColor::create(
            ccc4(color.r, color.g, color.b, 255),
            size,
            size
        );

        if (!visual)
            return nullptr;

        visual->setAnchorPoint({0.5f, 0.5f});

        return visual;
    }

    // --------------------------------------------------
    // Background
    // --------------------------------------------------

    void loadBackgroundImage() {

        if (bgSprite) {
            bgSprite->removeFromParentAndCleanup(true);
            bgSprite = nullptr;
        }

        if (bgType != "image")
            return;

        auto imgPathStr =
            Mod::get()->getSettingValue<std::string>(
                "image-path"
            );

        if (imgPathStr.empty())
            return;

        std::filesystem::path imgPath(imgPathStr);

        std::error_code ec;

        if (!std::filesystem::exists(imgPath, ec) || ec)
            return;

        auto tex = CCTextureCache::get()->addImage(
            imgPath.string().c_str(),
            false
        );

        if (!tex)
            return;

        bgSprite = CCSprite::createWithTexture(tex);

        if (!bgSprite)
            return;

        bgSprite->setAnchorPoint({0.5f, 0.5f});

        bgSprite->setPosition({
            winSize.width / 2.f,
            winSize.height / 2.f
        });

        fitBackgroundImage();

        this->addChild(bgSprite, -1);
    }

    void fitBackgroundImage() {

        if (!bgSprite)
            return;

        auto sz = bgSprite->getTextureRect().size;

        if (sz.width <= 0.f || sz.height <= 0.f)
            return;

        float sx = winSize.width / sz.width;
        float sy = winSize.height / sz.height;

        if (bgFit == "stretch") {
            bgSprite->setScaleX(sx);
            bgSprite->setScaleY(sy);
        } else {
            bgSprite->setScale(std::max(sx, sy));
        }

        bgSprite->setOpacity(
            static_cast<GLubyte>(bgAlpha)
        );
    }

    // --------------------------------------------------
    // Item visual
    // --------------------------------------------------

    CCNode* createItemVisual(
        bool player,
        int orbIdx,
        float radius
    ) {

#ifndef GEODE_IS_IOS

        if (player) {

            auto gm = GameManager::get();

            auto sp = SimplePlayer::create(
                gm->getPlayerFrame()
            );

            if (sp) {

                sp->setColors(
                    gm->colorForIdx(gm->getPlayerColor()),
                    gm->colorForIdx(gm->getPlayerColor2())
                );

                if (gm->getPlayerGlow()) {
                    sp->setGlowOutline(
                        gm->colorForIdx(
                            gm->getPlayerGlowColor()
                        )
                    );
                }

                sp->setScale(1.f);

                return sp;
            }

        } else {

            auto obj = GameObject::createWithKey(
                orbIds[orbIdx]
            );

            if (obj) {
                obj->setScale(1.f);
                return obj;
            }
        }

#endif

        return createFallbackVisual(
            radius,
            player,
            orbIdx
        );
    }

    // --------------------------------------------------
    // Spawn
    // --------------------------------------------------

    void addItem(bool forcePlayer = false) {

        bool spawnPlayer = forcePlayer;

        int orbIdx = -1;

        float radius = 17.5f;

        bool isBox = true;

        if (!spawnPlayer) {

            orbIdx = std::rand() % 11;

            isBox = (orbIdx == 10);

            radius = orbSizes[orbIdx] / 2.f;
        }

        PhysItem item;

        item.radius = radius;
        item.isPlayerCube = spawnPlayer;
        item.isBox = isBox;
        item.itemType = orbIdx;

        item.position.x = spawnPlayer
            ? randomFloat(radius, winSize.width - radius)
            : randomFloat(
                winSize.width * 0.1f,
                winSize.width * 0.9f
            );

        item.position.y = winSize.height +
            (spawnPlayer
                ? randomFloat(200.f, 1000.f)
                : 250.f);

        item.angle = randomFloat(0.f, PI * 2.f);

        item.velocity = {
            randomFloat(-30.f, 30.f),
            0.f
        };

        item.angularVelocity = randomFloat(-1.5f, 1.5f);

        // Player cube is slightly heavier.

        item.invMass = spawnPlayer ? 0.8f : 1.f;

        item.vis = createItemVisual(
            spawnPlayer,
            orbIdx,
            radius
        );

        if (item.vis) {

            item.vis->setPosition({
                -9999.f,
                -9999.f
            });

            this->addChild(
                item.vis,
                spawnPlayer ? 3 : 2
            );
        }

        items.push_back(item);
    }

    // --------------------------------------------------
    // Shape projection
    // --------------------------------------------------

    float projectedRadius(
        const PhysItem& item,
        Vec axis
    ) const {

        if (!item.isBox)
            return item.radius;

        Vec right = rotate(
            {1.f, 0.f},
            item.angle
        );

        Vec up = rotate(
            {0.f, 1.f},
            item.angle
        );

        return item.radius * (
            std::abs(dot(right, axis)) +
            std::abs(dot(up, axis))
        );
    }

    // --------------------------------------------------
    // Collision: circle-circle
    // --------------------------------------------------

    bool collideCircles(
        const PhysItem& a,
        const PhysItem& b,
        Vec& normal,
        float& penetration
    ) {

        Vec delta = sub(b.position, a.position);

        float distance = length(delta);

        float total = a.radius + b.radius;

        if (distance >= total)
            return false;

        normal = distance > 0.0001f
            ? mul(delta, 1.f / distance)
            : Vec{1.f, 0.f};

        penetration = total - distance;

        return true;
    }

    // --------------------------------------------------
    // Collision: box-box
    // --------------------------------------------------

    bool collideBoxes(
        const PhysItem& a,
        const PhysItem& b,
        Vec& normal,
        float& penetration
    ) {

        Vec axes[4] = {
            rotate({1.f, 0.f}, a.angle),
            rotate({0.f, 1.f}, a.angle),
            rotate({1.f, 0.f}, b.angle),
            rotate({0.f, 1.f}, b.angle)
        };

        Vec delta = sub(b.position, a.position);

        float minimumOverlap = 1e30f;
        Vec minimumAxis;

        for (const auto& axis : axes) {

            float distance = std::abs(dot(delta, axis));

            float overlap =
                projectedRadius(a, axis) +
                projectedRadius(b, axis) -
                distance;

            if (overlap <= 0.f)
                return false;

            if (overlap < minimumOverlap) {

                minimumOverlap = overlap;
                minimumAxis = axis;
            }
        }

        if (dot(delta, minimumAxis) < 0.f)
            minimumAxis = mul(minimumAxis, -1.f);

        normal = minimumAxis;
        penetration = minimumOverlap;

        return true;
    }

    // --------------------------------------------------
    // Collision: circle-box
    // --------------------------------------------------

    bool collideCircleBox(
        const PhysItem& circle,
        const PhysItem& box,
        Vec& normal,
        float& penetration
    ) {

        Vec local = rotate(
            sub(circle.position, box.position),
            -box.angle
        );

        float closestX = clampf(
            local.x,
            -box.radius,
            box.radius
        );

        float closestY = clampf(
            local.y,
            -box.radius,
            box.radius
        );

        Vec closest = {closestX, closestY};

        Vec delta = sub(closest, local);

        float distance = length(delta);

        if (distance > circle.radius)
            return false;

        Vec localNormal;

        if (distance > 0.0001f) {

            localNormal = mul(
                delta,
                1.f / distance
            );

            penetration = circle.radius - distance;

        } else {

            float dx = box.radius - std::abs(local.x);
            float dy = box.radius - std::abs(local.y);

            if (dx < dy) {

                localNormal = {
                    local.x >= 0.f ? 1.f : -1.f,
                    0.f
                };

                penetration = circle.radius + dx;

            } else {

                localNormal = {
                    0.f,
                    local.y >= 0.f ? 1.f : -1.f
                };

                penetration = circle.radius + dy;
            }
        }

        // Circle-to-box normal.

        normal = rotate(
            localNormal,
            box.angle
        );

        return true;
    }

    // --------------------------------------------------
    // General collision
    // --------------------------------------------------

    bool collide(
        const PhysItem& a,
        const PhysItem& b,
        Vec& normal,
        float& penetration
    ) {

        if (!a.isBox && !b.isBox) {
            return collideCircles(
                a,
                b,
                normal,
                penetration
            );
        }

        if (a.isBox && b.isBox) {
            return collideBoxes(
                a,
                b,
                normal,
                penetration
            );
        }

        if (!a.isBox && b.isBox) {
            return collideCircleBox(
                a,
                b,
                normal,
                penetration
            );
        }

        bool result = collideCircleBox(
            b,
            a,
            normal,
            penetration
        );

        normal = mul(normal, -1.f);

        return result;
    }

    // --------------------------------------------------
    // Collision resolution
    // --------------------------------------------------

    void resolveCollision(
        PhysItem& a,
        PhysItem& b,
        Vec normal,
        float penetration
    ) {

        float invA = a.invMass;
        float invB = b.invMass;

        float totalInv = invA + invB;

        if (totalInv <= 0.f)
            return;

        // Position correction.

        float correction = std::max(
            penetration - 0.01f,
            0.f
        ) * 0.65f / totalInv;

        a.position = sub(
            a.position,
            mul(normal, correction * invA)
        );

        b.position = add(
            b.position,
            mul(normal, correction * invB)
        );

        // Relative velocity.

        Vec relative = sub(
            b.velocity,
            a.velocity
        );

        float velocityAlongNormal = dot(
            relative,
            normal
        );

        if (velocityAlongNormal > 0.f)
            return;

        float impulseMagnitude =
            -(1.f + RESTITUTION) *
            velocityAlongNormal /
            totalInv;

        Vec impulse = mul(
            normal,
            impulseMagnitude
        );

        a.velocity = sub(
            a.velocity,
            mul(impulse, invA)
        );

        b.velocity = add(
            b.velocity,
            mul(impulse, invB)
        );

        // Friction.

        relative = sub(
            b.velocity,
            a.velocity
        );

        Vec tangent = sub(
            relative,
            mul(normal, dot(relative, normal))
        );

        float tangentLength = length(tangent);

        if (tangentLength > 0.0001f) {

            tangent = mul(
                tangent,
                1.f / tangentLength
            );

            float jt = -dot(relative, tangent) /
                totalInv;

            float frictionImpulse = clampf(
                jt,
                -impulseMagnitude * FRICTION,
                impulseMagnitude * FRICTION
            );

            Vec friction = mul(
                tangent,
                frictionImpulse
            );

            a.velocity = sub(
                a.velocity,
                mul(friction, invA)
            );

            b.velocity = add(
                b.velocity,
                mul(friction, invB)
            );

            a.angularVelocity -=
                frictionImpulse * 0.0008f;

            b.angularVelocity +=
                frictionImpulse * 0.0008f;
        }
    }

    // --------------------------------------------------
    // World boundaries
    // --------------------------------------------------

    void solveBoundaries(PhysItem& item) {

        float extentX = projectedRadius(
            item,
            {1.f, 0.f}
        );

        float extentY = projectedRadius(
            item,
            {0.f, 1.f}
        );

        if (item.position.x - extentX < 0.f) {

            item.position.x = extentX;

            if (item.velocity.x < 0.f)
                item.velocity.x *= -RESTITUTION;
        }

        if (item.position.x + extentX > winSize.width) {

            item.position.x = winSize.width - extentX;

            if (item.velocity.x > 0.f)
                item.velocity.x *= -RESTITUTION;
        }

        if (!allowDrop && item.position.y - extentY < 0.f) {

            item.position.y = extentY;

            if (item.velocity.y < 0.f)
                item.velocity.y *= -RESTITUTION;

            item.velocity.x *= 0.985f;
            item.angularVelocity *= 0.97f;
        }
    }

    // --------------------------------------------------
    // Physics step
    // --------------------------------------------------

    void physicsStep(float dt) {

        float gravity = GRAVITY * physicsSpeed;

        // Integrate movement.

        for (auto& item : items) {

            item.velocity.y -= gravity * dt;

            item.position = add(
                item.position,
                mul(item.velocity, dt)
            );

            item.angle += item.angularVelocity * dt;

            // Angular damping.

            item.angularVelocity *= 0.999f;

            solveBoundaries(item);
        }

        // Pair collisions.

        constexpr int ITERATIONS = 4;

        for (int iteration = 0;
             iteration < ITERATIONS;
             iteration++) {

            for (size_t i = 0; i < items.size(); i++) {

                for (size_t j = i + 1;
                     j < items.size();
                     j++) {

                    auto& a = items[i];
                    auto& b = items[j];

                    Vec normal;
                    float penetration = 0.f;

                    if (collide(
                        a,
                        b,
                        normal,
                        penetration
                    )) {

                        resolveCollision(
                            a,
                            b,
                            normal,
                            penetration
                        );
                    }
                }
            }

            for (auto& item : items)
                solveBoundaries(item);
        }
    }

    // --------------------------------------------------
    // Visual update
    // --------------------------------------------------

    void updateVisuals() {

        for (auto& item : items) {

            if (!item.vis)
                continue;

            item.vis->setPosition({
                item.position.x,
                item.position.y
            });

            item.vis->setRotation(
                item.angle * 180.f / PI
            );
        }
    }

    // --------------------------------------------------
    // Reset
    // --------------------------------------------------

    void fullReset() {

        maxItems = static_cast<int>(
            Mod::get()->getSettingValue<int64_t>(
                "item-count"
            )
        );

        playerChance = static_cast<int>(
            Mod::get()->getSettingValue<int64_t>(
                "player-chance"
            )
        );

        allowDrop =
            Mod::get()->getSettingValue<bool>(
                "infinite-fall"
            );

        physicsSpeed = static_cast<float>(
            Mod::get()->getSettingValue<int64_t>(
                "physics-speed"
            )
        ) / 10.f;

        if (physicsSpeed <= 0.f)
            physicsSpeed = 1.f;

        spawnTicks = std::max(
            1,
            static_cast<int>(20.f / physicsSpeed)
        );

        maxItems = std::max(1, maxItems);

        elapsed = 0.f;
        itemsMade = 0;

        playerIn = false;
        filled = false;
        draining = false;

        physicsLag = 0.f;

        pauseBeforeDrain =
            5.f + randomFloat(0.f, 1.f);

        nextSpawn = 0.f;

        for (auto& item : items) {

            if (item.vis)
                item.vis->removeFromParentAndCleanup(true);
        }

        items.clear();
    }

    // --------------------------------------------------
    // Init
    // --------------------------------------------------

    bool init() override {

        bgType =
            Mod::get()->getSettingValue<std::string>(
                "background-mode"
            );

        if (bgType != "blur" && bgType != "image")
            bgType = "color";

        bgCol =
            Mod::get()->getSettingValue<ccColor3B>(
                "overlay-color"
            );

        bgAlpha = static_cast<int>(
            Mod::get()->getSettingValue<int64_t>(
                "overlay-opacity"
            )
        );

        bgFit =
            Mod::get()->getSettingValue<std::string>(
                "image-fit"
            );

        if (!CCLayerColor::initWithColor(
            ccc4(bgCol.r, bgCol.g, bgCol.b, 0)
        )) {
            return false;
        }

        winSize = CCDirector::get()->getWinSize();

        this->setContentSize(winSize);
        this->setPosition(CCPointZero);

        if (bgType == "blur" && BlurAPI::isBlurAPIEnabled())
            BlurAPI::addBlur(this);

        loadBackgroundImage();

        this->setTouchEnabled(true);
        this->setTouchMode(kCCTouchesOneByOne);
        this->setTouchPriority(-9999);

        this->setKeypadEnabled(true);
        this->setKeyboardEnabled(true);
        this->setMouseEnabled(true);

        this->scheduleUpdate();

        std::srand(
            static_cast<unsigned>(std::time(nullptr))
        );

        fadeIn = 0.f;
        doneFade = false;
        killed = false;

        fullReset();

        return true;
    }

    // --------------------------------------------------
    // Update
    // --------------------------------------------------

    void update(float dt) override {

        if (killed)
            return;

        dt = clampf(dt, 0.f, 0.25f);

        if (!doneFade) {

            fadeIn += dt;

            float t = std::min(
                fadeIn / 0.5f,
                1.f
            );

            if (t >= 1.f)
                doneFade = true;

            if (bgType == "blur") {

                setOpacity(
                    static_cast<GLubyte>(180.f * t)
                );

            } else if (bgType == "color") {

                setOpacity(
                    static_cast<GLubyte>(bgAlpha * t)
                );

            } else {

                setOpacity(0);

                if (bgSprite) {

                    bgSprite->setOpacity(
                        static_cast<GLubyte>(bgAlpha * t)
                    );
                }
            }

            return;
        }

        elapsed += dt;

        float spawnDelay =
            static_cast<float>(spawnTicks) / 60.f;

        while (
            itemsMade < maxItems &&
            elapsed >= nextSpawn
        ) {

            addItem(false);

            itemsMade++;

            nextSpawn = elapsed + spawnDelay;
        }

        if (!playerIn && itemsMade >= maxItems / 2) {

            playerIn = true;

            if ((std::rand() % 100) < playerChance)
                addItem(true);
        }

        if (
            !allowDrop &&
            !filled &&
            itemsMade >= maxItems
        ) {

            float spawnTime = maxItems * spawnDelay;

            if (elapsed >= spawnTime + pauseBeforeDrain) {

                filled = true;
                draining = true;
            }
        }

        if (!allowDrop && draining) {

            bool allGone = true;

            for (const auto& item : items) {

                if (item.position.y > -300.f) {
                    allGone = false;
                    break;
                }
            }

            if (allGone) {

                fullReset();

                return;
            }
        }

        if (
            allowDrop &&
            elapsed > maxItems * spawnDelay + 8.f
        ) {

            fullReset();

            return;
        }

        // Fixed timestep.

        physicsLag += dt;

        int steps = 0;

        while (
            physicsLag >= FIXED_STEP &&
            steps < 15
        ) {

            physicsStep(FIXED_STEP);

            physicsLag -= FIXED_STEP;

            steps++;
        }

        updateVisuals();
    }

public:

    // --------------------------------------------------
    // Create
    // --------------------------------------------------

    static ScreensaverLayer* create() {

        auto ptr = new ScreensaverLayer();

        if (ptr && ptr->init()) {

            ptr->autorelease();

            ptr->setTag(2047);

            return ptr;
        }

        CC_SAFE_DELETE(ptr);

        return nullptr;
    }

    // --------------------------------------------------
    // Kill
    // --------------------------------------------------

    void kill() {

        if (killed)
            return;

        killed = true;

        this->stopAllActions();
        this->unscheduleAllSelectors();

        this->removeFromParentAndCleanup(true);

        ssActive = false;
        idleTimer = 0.f;
    }

    // --------------------------------------------------
    // Input
    // --------------------------------------------------

    bool ccTouchBegan(CCTouch*, CCEvent*) override {

        kill();

        return true;
    }

    void keyDown(enumKeyCodes, double) override {
        kill();
    }

    void keyBackClicked() override {
        kill();
    }

    bool scrollWheel(float, float) override {

        kill();

        return true;
    }

    // --------------------------------------------------
    // Destructor
    // --------------------------------------------------

    ~ScreensaverLayer() override {

        if (bgType == "blur")
            BlurAPI::removeBlur(this);
    }
};

// --------------------------------------------------
// Orb data
// --------------------------------------------------

const int ScreensaverLayer::orbIds[11] = {
    36, 84, 141, 1022, 1330, 1333,
    1704, 1751, 3004, 3027, 1594
};

const float ScreensaverLayer::orbSizes[11] = {
    32.3f, 33.2f, 33.6f, 31.96f, 36.5f,
    34.9f, 41.f, 41.f, 41.f, 39.4f, 30.8f
};

// --------------------------------------------------
// Idle logic
// --------------------------------------------------

class $modify(OrbitScrsvrScene, CCScene) {

    bool init() {

        if (!CCScene::init())
            return false;

        this->schedule(
            schedule_selector(
                OrbitScrsvrScene::checkIdle
            ),
            0.f
        );

        return true;
    }

    void checkIdle(float dt) {

        auto scene =
            CCDirector::get()->getRunningScene();

        if (!scene)
            return;

        bool inEditor =
            scene->getChildByType<LevelEditorLayer>(0)
            != nullptr;

        bool inPlay =
            scene->getChildByType<PlayLayer>(0)
            != nullptr;

        if (inEditor || (inPlay && !pauseOpen) || ssActive) {

            idleTimer = 0.f;
            return;
        }

        idleTimer += dt;

        float timeout = static_cast<float>(
            Mod::get()->getSettingValue<int64_t>(
                "wait-time"
            )
        );

        if (idleTimer < timeout)
            return;

        idleTimer = 0.f;
        ssActive = true;

        auto ss = ScreensaverLayer::create();

        if (ss) {

            ss->setZOrder(9999);
            this->addChild(ss);

        } else {

            ssActive = false;
        }
    }
};

// --------------------------------------------------
// Touch dispatcher
// --------------------------------------------------

class $modify(CCTouchDispatcher) {

    void touches(
        CCSet* t,
        CCEvent* e,
        unsigned int type
    ) {

        CCTouchDispatcher::touches(t, e, type);

        if (type == CCTOUCHBEGAN)
            idleTimer = 0.f;
    }
};

// --------------------------------------------------
// Keyboard dispatcher
// --------------------------------------------------

#ifndef GEODE_IS_IOS

class $modify(CCKeyboardDispatcher) {

    bool dispatchKeyboardMSG(
        enumKeyCodes key,
        bool down,
        bool repeat,
        double ts
    ) {

        bool result =
            CCKeyboardDispatcher::dispatchKeyboardMSG(
                key,
                down,
                repeat,
                ts
            );

        if (down && !repeat) {

            idleTimer = 0.f;

            auto scene =
                CCDirector::get()->getRunningScene();

            if (scene) {

                auto ss = scene->getChildByTag(2047);

                if (ss) {
                    static_cast<ScreensaverLayer*>(ss)->kill();
                }
            }
        }

        return result;
    }
};

#endif

// --------------------------------------------------
// Pause layer
// --------------------------------------------------

class $modify(PauseLayer) {

    void customSetup() {

        PauseLayer::customSetup();

        pauseOpen = true;
        idleTimer = 0.f;
    }

#ifndef GEODE_IS_IOS

    void onQuit(CCObject* sender) {

        pauseOpen = false;
        idleTimer = 0.f;

        PauseLayer::onQuit(sender);
    }

    void onResume(CCObject* sender) {

        pauseOpen = false;
        idleTimer = 0.f;

        auto scene =
            CCDirector::get()->getRunningScene();

        if (scene) {

            auto ss = scene->getChildByTag(2047);

            if (ss) {
                static_cast<ScreensaverLayer*>(ss)->kill();
            }
        }

        PauseLayer::onResume(sender);
    }

#endif
};

// --------------------------------------------------
// Level editor
// --------------------------------------------------

class $modify(LevelEditorLayer) {

    bool init(GJGameLevel* lvl, bool unk) {

        idleTimer = 0.f;

        return LevelEditorLayer::init(lvl, unk);
    }
};
