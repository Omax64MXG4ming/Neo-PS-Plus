#include <Geode/Geode.hpp>
#include <Geode/modify/CCScene.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>

#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS)
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/binding/GameObject.hpp>
#include <Geode/binding/SimplePlayer.hpp>
#endif

#include "../include/BlurAPI.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

using namespace geode::prelude;
using namespace cocos2d;

// --------------------------------------------------
// Constants
// --------------------------------------------------

namespace {

constexpr float PI = 3.14159265358979323846f;
constexpr float FIXED_STEP = 1.f / 60.f;

constexpr float GRAVITY = 9.8f * 3.f * 40.f;
constexpr float RESTITUTION = 0.5f;
constexpr float FRICTION = 0.7f;

constexpr int SOLVER_ITERATIONS = 5;

// --------------------------------------------------
// Vector
// --------------------------------------------------

struct Vec2 {
    float x = 0.f;
    float y = 0.f;
};

static Vec2 vecAdd(Vec2 a, Vec2 b) {
    return {a.x + b.x, a.y + b.y};
}

static Vec2 vecSub(Vec2 a, Vec2 b) {
    return {a.x - b.x, a.y - b.y};
}

static Vec2 vecMul(Vec2 a, float value) {
    return {a.x * value, a.y * value};
}

static float vecDot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

static float vecLength(Vec2 a) {
    return std::sqrt(vecDot(a, a));
}

static Vec2 vecNormalize(Vec2 a) {
    float len = vecLength(a);

    if (len <= 0.00001f)
        return {1.f, 0.f};

    return vecMul(a, 1.f / len);
}

static Vec2 vecRotate(Vec2 p, float angle) {
    float c = std::cos(angle);
    float s = std::sin(angle);

    return {
        p.x * c - p.y * s,
        p.x * s + p.y * c
    };
}

static float cross(Vec2 a, Vec2 b) {
    return a.x * b.y - a.y * b.x;
}

static float clampValue(float value, float min, float max) {
    return std::max(min, std::min(value, max));
}

static float randomFloat(float min, float max) {
    return min + (max - min) *
        (static_cast<float>(std::rand()) /
         static_cast<float>(RAND_MAX));
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

    Vec2 position;
    Vec2 velocity;

    float radius = 16.f;

    float angle = 0.f;
    float angularVelocity = 0.f;

    float invMass = 1.f;
    float invInertia = 0.f;

    int itemType = -1;

    bool isPlayerCube = false;
    bool isBox = false;
};

// --------------------------------------------------
// Screensaver layer
// --------------------------------------------------

class ScreensaverLayer : public CCLayerColor {

    std::vector<PhysItem> items;

    // Timing

    float elapsed = 0.f;
    int itemsMade = 0;

    bool playerIn = false;
    bool draining = false;
    bool filled = false;

    float physicsLag = 0.f;
    float pauseBeforeDrain = 0.f;
    float nextSpawn = 0.f;

    // Configuration

    int maxItems = 0;
    float physicsSpeed = 1.f;
    int playerChance = 0;

    bool allowDrop = false;
    bool floorEnabled = true;

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
    // Shape projection
    // --------------------------------------------------

    float projectedRadius(
        const PhysItem& item,
        Vec2 axis
    ) const {

        if (!item.isBox)
            return item.radius;

        Vec2 right = vecRotate(
            {1.f, 0.f},
            item.angle
        );

        Vec2 up = vecRotate(
            {0.f, 1.f},
            item.angle
        );

        return item.radius * (
            std::abs(vecDot(right, axis)) +
            std::abs(vecDot(up, axis))
        );
    }

    // --------------------------------------------------
    // Inertia
    // --------------------------------------------------

    void calculateMass(PhysItem& item) {

        // Same approximate density for every object.

        float mass = PI * item.radius * item.radius;

        if (item.isBox)
            mass = 4.f * item.radius * item.radius;

        mass = std::max(mass, 1.f);

        item.invMass = 1.f / mass;

        float inertia = 0.f;

        if (item.isBox) {

            float side = item.radius * 2.f;

            inertia = mass * (
                side * side + side * side
            ) / 12.f;

        } else {

            inertia = 0.5f *
                mass *
                item.radius *
                item.radius;
        }

        item.invInertia = inertia > 0.0001f
            ? 1.f / inertia
            : 0.f;
    }

    // --------------------------------------------------
    // Background image
    // --------------------------------------------------

void loadBackgroundImage() {

    if (bgSprite) {
        bgSprite->removeFromParentAndCleanup(true);
        bgSprite = nullptr;
    }

    if (bgType != "image")
        return;

    auto imgPath = Mod::get()->getSettingValue<std::string>(
        "image-path"
    );

    std::filesystem::path imagePath(imgPath);

    std::error_code ec;

    if (
        imagePath.empty() ||
        !std::filesystem::exists(imagePath, ec) ||
        ec
    ) {
        return;
    }

    auto tex = CCTextureCache::get()->addImage(
        imagePath.string().c_str(),
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

        auto size = bgSprite->getTextureRect().size;

        if (size.width <= 0.f || size.height <= 0.f)
            return;

        float sx = winSize.width / size.width;
        float sy = winSize.height / size.height;

        if (bgFit == "stretch") {

            bgSprite->setScaleX(sx);
            bgSprite->setScaleY(sy);

        } else {

            bgSprite->setScale(std::max(sx, sy));
        }

        bgSprite->setOpacity(
            static_cast<GLubyte>(
                clampValue(static_cast<float>(bgAlpha), 0.f, 255.f)
            )
        );
    }

    // --------------------------------------------------
    // Fallback visual
    // --------------------------------------------------

    CCNode* createFallbackVisual(
        float radius,
        bool player,
        int orbIdx
    ) {

        static const ccColor3B orbColors[11] = {
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

        ccColor3B color = player
            ? ccc3(70, 170, 255)
            : orbColors[
                std::max(0, std::min(orbIdx, 10))
            ];

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
    // Create visual
    // --------------------------------------------------

    CCNode* createItemVisual(
        bool spawnPlayer,
        int orbIdx,
        float radius
    ) {

#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS)

        if (spawnPlayer) {

            auto gm = GameManager::get();

            auto sp = SimplePlayer::create(
                gm->getPlayerFrame()
            );

            if (sp) {
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
            spawnPlayer,
            orbIdx
        );
    }

    // --------------------------------------------------
    // Spawn item
    // --------------------------------------------------

    void addItem(bool forcePlayer = false) {

        bool spawnPlayer = forcePlayer;

        int orbIdx = -1;

        float radius = 17.5f;

        bool isBox = true;

        if (spawnPlayer) {

            radius = 17.5f;

        } else {

            orbIdx = std::rand() % 11;

            isBox = (orbIdx == 10);

            radius = orbSizes[orbIdx] / 2.f;
        }

        PhysItem item;

        item.radius = radius;
        item.isBox = isBox;
        item.isPlayerCube = spawnPlayer;
        item.itemType = orbIdx;

        // Spawn above the screen.

        float minX = radius;
        float maxX = std::max(
            radius,
            winSize.width - radius
        );

        item.position.x = randomFloat(minX, maxX);

        item.position.y = winSize.height +
            (spawnPlayer
                ? randomFloat(200.f, 1000.f)
                : 250.f);

        item.velocity = {
            spawnPlayer ? randomFloat(-10.f, 10.f)
                        : randomFloat(-10.f, 10.f),
            0.f
        };

        item.angle = randomFloat(0.f, PI * 2.f);

        item.angularVelocity = randomFloat(-1.5f, 1.5f);

        calculateMass(item);

        // Push ordinary orbs.

        if (!spawnPlayer) {
            item.velocity.x +=
                (10 - std::rand() % 21) * 0.05f * 40.f;
        }

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
    // Circle vs circle
    //
    // Normal points from A to B.
    // --------------------------------------------------

    bool collideCircles(
        const PhysItem& a,
        const PhysItem& b,
        Vec2& normal,
        float& penetration
    ) const {

        Vec2 delta = vecSub(
            b.position,
            a.position
        );

        float distance = vecLength(delta);

        float totalRadius = a.radius + b.radius;

        if (distance >= totalRadius)
            return false;

        normal = distance > 0.0001f
            ? vecMul(delta, 1.f / distance)
            : Vec2{1.f, 0.f};

        penetration = totalRadius - distance;

        return true;
    }

    // --------------------------------------------------
    // Box vs box (SAT)
    // --------------------------------------------------

    bool collideBoxes(
        const PhysItem& a,
        const PhysItem& b,
        Vec2& normal,
        float& penetration
    ) const {

        Vec2 axes[4] = {
            vecRotate({1.f, 0.f}, a.angle),
            vecRotate({0.f, 1.f}, a.angle),
            vecRotate({1.f, 0.f}, b.angle),
            vecRotate({0.f, 1.f}, b.angle)
        };

        Vec2 delta = vecSub(
            b.position,
            a.position
        );

        float minOverlap = 1e30f;
        Vec2 minAxis = {1.f, 0.f};

        for (const auto& axis : axes) {

            float distance = std::abs(
                vecDot(delta, axis)
            );

            float overlap =
                projectedRadius(a, axis) +
                projectedRadius(b, axis) -
                distance;

            if (overlap <= 0.f)
                return false;

            if (overlap < minOverlap) {
                minOverlap = overlap;
                minAxis = axis;
            }
        }

        if (vecDot(delta, minAxis) < 0.f)
            minAxis = vecMul(minAxis, -1.f);

        normal = minAxis;
        penetration = minOverlap;

        return true;
    }

    // --------------------------------------------------
    // Circle vs box
    //
    // Normal points from circle to box.
    // --------------------------------------------------

    bool collideCircleBox(
        const PhysItem& circle,
        const PhysItem& box,
        Vec2& normal,
        float& penetration
    ) const {

        Vec2 local = vecRotate(
            vecSub(circle.position, box.position),
            -box.angle
        );

        Vec2 closest = {
            clampValue(local.x, -box.radius, box.radius),
            clampValue(local.y, -box.radius, box.radius)
        };

        Vec2 delta = vecSub(closest, local);

        float distance = vecLength(delta);

        if (distance > circle.radius)
            return false;

        Vec2 localNormal;

        if (distance > 0.0001f) {

            localNormal = vecMul(
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

        normal = vecRotate(
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
        Vec2& normal,
        float& penetration
    ) const {

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

        normal = vecMul(normal, -1.f);

        return result;
    }

    // --------------------------------------------------
    // Collision resolution
    // --------------------------------------------------

    void resolveCollision(
        PhysItem& a,
        PhysItem& b,
        Vec2 normal,
        float penetration
    ) {

        float invA = a.invMass;
        float invB = b.invMass;

        float totalInvMass = invA + invB;

        if (totalInvMass <= 0.f)
            return;

        // Position correction.

        float correction = std::max(
            penetration - 0.01f,
            0.f
        ) * 0.75f / totalInvMass;

        a.position = vecSub(
            a.position,
            vecMul(normal, correction * invA)
        );

        b.position = vecAdd(
            b.position,
            vecMul(normal, correction * invB)
        );

        // Relative velocity.

        Vec2 relativeVelocity = vecSub(
            b.velocity,
            a.velocity
        );

        float velocityNormal = vecDot(
            relativeVelocity,
            normal
        );

        // Already separating.

        if (velocityNormal > 0.f)
            return;

        float impulseMagnitude =
            -(1.f + RESTITUTION) *
            velocityNormal /
            totalInvMass;

        Vec2 impulse = vecMul(
            normal,
            impulseMagnitude
        );

        a.velocity = vecSub(
            a.velocity,
            vecMul(impulse, invA)
        );

        b.velocity = vecAdd(
            b.velocity,
            vecMul(impulse, invB)
        );

        // Tangential friction.

        relativeVelocity = vecSub(
            b.velocity,
            a.velocity
        );

        Vec2 tangent = vecSub(
            relativeVelocity,
            vecMul(
                normal,
                vecDot(relativeVelocity, normal)
            )
        );

        float tangentLength = vecLength(tangent);

        if (tangentLength <= 0.0001f)
            return;

        tangent = vecMul(
            tangent,
            1.f / tangentLength
        );

        float jt =
            -vecDot(relativeVelocity, tangent) /
            totalInvMass;

        float frictionLimit =
            impulseMagnitude * FRICTION;

        jt = clampValue(
            jt,
            -frictionLimit,
            frictionLimit
        );

        Vec2 frictionImpulse = vecMul(
            tangent,
            jt
        );

        a.velocity = vecSub(
            a.velocity,
            vecMul(frictionImpulse, invA)
        );

        b.velocity = vecAdd(
            b.velocity,
            vecMul(frictionImpulse, invB)
        );

        // Approximate angular response.

        a.angularVelocity -=
            cross(normal, frictionImpulse) *
            a.invInertia *
            a.radius *
            0.15f;

        b.angularVelocity +=
            cross(normal, frictionImpulse) *
            b.invInertia *
            b.radius *
            0.15f;
    }

    // --------------------------------------------------
    // Screen boundaries
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

        // Left wall.

        if (item.position.x - extentX < 0.f) {

            item.position.x = extentX;

            if (item.velocity.x < 0.f)
                item.velocity.x *= -RESTITUTION;

            item.angularVelocity *= 0.98f;
        }

        // Right wall.

        if (item.position.x + extentX > winSize.width) {

            item.position.x = winSize.width - extentX;

            if (item.velocity.x > 0.f)
                item.velocity.x *= -RESTITUTION;

            item.angularVelocity *= 0.98f;
        }

        // Floor.

        if (
            floorEnabled &&
            item.position.y - extentY < 0.f
        ) {

            item.position.y = extentY;

            if (item.velocity.y < 0.f)
                item.velocity.y *= -RESTITUTION;

            item.velocity.x *= 0.985f;
            item.angularVelocity *= 0.97f;

            if (std::abs(item.velocity.y) < 8.f)
                item.velocity.y = 0.f;
        }
    }

    // --------------------------------------------------
    // Physics integration
    // --------------------------------------------------

    void physicsStep(float dt) {

        float gravity = GRAVITY * physicsSpeed;

        // Integrate each body.

        for (auto& item : items) {

            item.velocity.y -= gravity * dt;

            item.position = vecAdd(
                item.position,
                vecMul(item.velocity, dt)
            );

            item.angle +=
                item.angularVelocity * dt;

            // Angular damping.

            item.angularVelocity *= 0.999f;

            solveBoundaries(item);
        }

        // Collision solver.

        for (
            int iteration = 0;
            iteration < SOLVER_ITERATIONS;
            iteration++
        ) {

            for (size_t i = 0; i < items.size(); i++) {

                for (
                    size_t j = i + 1;
                    j < items.size();
                    j++
                ) {

                    auto& a = items[i];
                    auto& b = items[j];

                    Vec2 normal;
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

            // Keep objects inside the screen.

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

        floorEnabled = !allowDrop;

        pauseBeforeDrain =
            5.f + randomFloat(0.f, 1.f);

        nextSpawn = 0.f;

        // Remove previous visuals.

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

        if (!CCLayerColor::initWithColor({
            bgCol.r,
            bgCol.g,
            bgCol.b,
            0
        })) {
            return false;
        }

        winSize = CCDirector::get()->getWinSize();

        this->setContentSize(winSize);
        this->setPosition(CCPointZero);

        // Blur.

        if (
            bgType == "blur" &&
            BlurAPI::isBlurAPIEnabled()
        ) {
            BlurAPI::addBlur(this);
        }

        loadBackgroundImage();

        // Input.

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

        // Prevent huge physics jumps after a lag spike.

        dt = clampValue(dt, 0.f, 0.25f);

        // Fade in.

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

        float H = winSize.height;

        elapsed += dt;

        float spawnDelay =
            static_cast<float>(spawnTicks) / 60.f;

        // Spawn objects.

        while (
            itemsMade < maxItems &&
            elapsed >= nextSpawn
        ) {

            addItem(false);

            itemsMade++;

            nextSpawn = elapsed + spawnDelay;
        }

        // Player chance.

        if (
            !playerIn &&
            itemsMade >= maxItems / 2
        ) {

            playerIn = true;

            if ((std::rand() % 100) < playerChance)
                addItem(true);
        }

        // Start draining.

        if (
            !allowDrop &&
            !filled &&
            itemsMade >= maxItems
        ) {

            float spawnTime =
                maxItems * spawnDelay;

            if (
                elapsed >=
                spawnTime + pauseBeforeDrain
            ) {

                filled = true;
                draining = true;

                // Remove the floor.

                floorEnabled = false;
            }
        }

        // Check if everything has fallen.

        if (!allowDrop && draining) {

            bool allGone = true;

            for (const auto& item : items) {

                if (item.position.y >= -300.f) {
                    allGone = false;
                    break;
                }
            }

            if (allGone) {

                fullReset();

                return;
            }
        }

        // Infinite fall mode.

        if (
            allowDrop &&
            elapsed >
                static_cast<float>(maxItems) *
                spawnDelay + 8.f
        ) {

            fullReset();

            return;
        }

        // Fixed timestep simulation.

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

    bool ccTouchBegan(
        CCTouch*,
        CCEvent*
    ) override {

        kill();

        return true;
    }

    void keyDown(
        enumKeyCodes,
        double
    ) override {
        kill();
    }

    void keyBackClicked() override {
        kill();
    }

    bool scrollWheel(
        float,
        float
    ) override {

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

        if (
            inEditor ||
            (inPlay && !pauseOpen) ||
            ssActive
        ) {

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
        CCSet* touches,
        CCEvent* event,
        unsigned int type
    ) {

        CCTouchDispatcher::touches(
            touches,
            event,
            type
        );

        if (type == CCTOUCHBEGAN)
            idleTimer = 0.f;
    }
};

// --------------------------------------------------
// Keyboard dispatcher
// --------------------------------------------------

#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS)

class $modify(CCKeyboardDispatcher) {

    bool dispatchKeyboardMSG(
        enumKeyCodes key,
        bool down,
        bool repeat,
        double timestamp
    ) {

        bool result =
            CCKeyboardDispatcher::dispatchKeyboardMSG(
                key,
                down,
                repeat,
                timestamp
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

#if defined(GEODE_IS_WINDOWS) || defined(GEODE_IS_MACOS)

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

    bool init(
        GJGameLevel* level,
        bool unk
    ) {

        idleTimer = 0.f;

        return LevelEditorLayer::init(
            level,
            unk
        );
    }
};
