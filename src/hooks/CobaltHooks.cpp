#include <Geode/Geode.hpp>

#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

#include "../features/CobaltState.hpp"

using namespace geode::prelude;

// Level Info
class $modify(CobaltLevelInfo, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge))
            return false;

        if (level) {
            if (cobalt::g_state.copyHack) {
                level->m_password = 1;
            }

            if (cobalt::g_state.autoLDM) {
                level->m_lowDetailModeToggled = true;
            }
        }

        return true;
    }
};

// Edit Level
class $modify(CobaltEditLevel, EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level))
            return false;

        if (level) {
            if (cobalt::g_state.autoLDM) {
                level->m_lowDetailModeToggled = true;
            }

            if (cobalt::g_state.autoVerify) {
                level->m_isVerified = true;
            }
        }

        return true;
    }
};

// Pause Layer
class $modify(CobaltPauseLayer, PauseLayer) {
    void onQuit(CCObject* sender) {
        if (cobalt::g_state.disableQuit)
            return;

        PauseLayer::onQuit(sender);
    }
};

// Game Manager
class $modify(CobaltGameManager, GameManager) {
    bool isColorUnlocked(int id, UnlockType type) {
        if (cobalt::g_state.iconBypass)
            return true;

        if (cobalt::g_state.lockAllIcons)
            return false;

        return GameManager::isColorUnlocked(id, type);
    }

    bool isIconUnlocked(int id, IconType type) {
        if (cobalt::g_state.iconBypass)
            return true;

        if (cobalt::g_state.lockAllIcons)
            return false;

        return GameManager::isIconUnlocked(id, type);
    }
};

// Play Layer
class $modify(CobaltPlayLayer, PlayLayer) {
   void update(float dt) {
     PlayLayer::update(dt);

       if (cobalt::g_state.autoComplete) {
          cobalt::g_state.autoComplete = false;
            PlayLayer::showEndLayer();
        }
   }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (!cobalt::g_state.noclip) {
            PlayLayer::destroyPlayer(player, object);
        }
    }

    int checkCollisions(
        PlayerObject* player,
        float dt,
        bool ignoreDamage
    ) {
        if (cobalt::g_state.noCollision)
            return 0;

        return PlayLayer::checkCollisions(player, dt, ignoreDamage);
    }

    void shakeCamera(float a, float b, float c) {
        if (!cobalt::g_state.noShake) {
            PlayLayer::shakeCamera(a, b, c);
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        cobalt::applySpeedhack();
    }
};

// Player Object
class $modify(CobaltPlayerObject, PlayerObject) {
    void playDeathEffect() {
        if (!cobalt::g_state.noDeathEffect) {
            PlayerObject::playDeathEffect();
        }
    }
};
