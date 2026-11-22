#include <Geode/Geode.hpp>

#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include "../features/CobaltState.hpp"

using namespace geode::prelude;

// --------------------------------------------------
// LEVEL INFO
// --------------------------------------------------

class $modify(CobaltLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge))
            return false;

        return true;
    }
};

// --------------------------------------------------
// EDIT LEVEL
// --------------------------------------------------

class $modify(CobaltEditLevelLayer, EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level))
            return false;

        return true;
    }
};

// --------------------------------------------------
// PAUSE LAYER
// --------------------------------------------------

class $modify(CobaltPauseLayer, PauseLayer) {
    void onQuit(CCObject* sender) {
        PauseLayer::onQuit(sender);
    }
};

// --------------------------------------------------
// GAME MANAGER
// --------------------------------------------------

class $modify(CobaltGameManager, GameManager) {
    bool isColorUnlocked(int id, UnlockType type) {
        return GameManager::isColorUnlocked(id, type);
    }

    bool isIconUnlocked(int id, IconType type) {
        return GameManager::isIconUnlocked(id, type);
    }
};

// --------------------------------------------------
// PLAYER OBJECT
// --------------------------------------------------

class $modify(CobaltPlayerObject, PlayerObject) {
    void playDeathEffect() {
        PlayerObject::playDeathEffect();
    }
};

// --------------------------------------------------
// PLAY LAYER
// --------------------------------------------------
//
// TEMPORARILY DISABLED:
//
// The PlayLayer bindings declare:
// circleWaveWillBeRemoved(CCCircleWave*)
//
// However, there is no Android implementation address
// in the current 2.2091 binding declaration.
//
// Disabling this hook avoids introducing the
// problematic PlayLayer ABI thunk while testing.
//
// Reintroduce individual PlayLayer hooks after
// confirming the Android binding compatibility.
//

