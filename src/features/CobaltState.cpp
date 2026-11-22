#include "CobaltState.hpp"

#include <Geode/Geode.hpp>
#include <Geode/cocos/CCDirector.h>
#include <Geode/binding/FMODAudioEngine.hpp>

using namespace geode::prelude;

namespace cobalt {

State g_state;

void load() {
    auto mod = Mod::get();

    g_state.showOnStartup =
        mod->getSettingValue<bool>("ShowOnStartup");

    g_state.debug =
        mod->getSettingValue<bool>("isDebugOn");

    g_state.speedhackSong =
        mod->getSavedValue<bool>("isSpeedhackSong", false);

    g_state.autoLDM =
        mod->getSavedValue<bool>("isAutoLDM", false);

    g_state.copyHack =
        mod->getSavedValue<bool>("isCopyHack", false);

    g_state.editHack =
        mod->getSavedValue<bool>("isEditHack", false);

    g_state.disableQuit =
        mod->getSavedValue<bool>("isDisableQuit", false);

    g_state.iconBypass =
        mod->getSavedValue<bool>("isIconBypass", false);

    g_state.characterFilterBypass =
        mod->getSavedValue<bool>("isCharacterFilterBypass", false);

    g_state.sliderHack =
        mod->getSavedValue<bool>("isSliderHack", false);

    g_state.autoVerify =
        mod->getSavedValue<bool>("isAutoVerify", false);

    g_state.noHide =
        mod->getSavedValue<bool>("isNotHide", false);

    g_state.noDeathEffect =
        mod->getSavedValue<bool>("isNotDeathEffect", false);

    g_state.noShake =
        mod->getSavedValue<bool>("isNotShake", false);

    g_state.noTransitions =
        mod->getSavedValue<bool>("isTransitionOff", false);

    g_state.lockAllIcons = false;
}

void save() {
    auto mod = Mod::get();

    mod->setSavedValue("isSpeedhackSong", g_state.speedhackSong);

    mod->setSavedValue("isAutoLDM", g_state.autoLDM);
    mod->setSavedValue("isCopyHack", g_state.copyHack);
    mod->setSavedValue("isEditHack", g_state.editHack);
    mod->setSavedValue("isDisableQuit", g_state.disableQuit);

    mod->setSavedValue("isIconBypass", g_state.iconBypass);

    mod->setSavedValue(
        "isCharacterFilterBypass",
        g_state.characterFilterBypass
    );

    mod->setSavedValue("isSliderHack", g_state.sliderHack);
    mod->setSavedValue("isAutoVerify", g_state.autoVerify);

    mod->setSavedValue("isNotHide", g_state.noHide);
    mod->setSavedValue("isNotDeathEffect", g_state.noDeathEffect);
    mod->setSavedValue("isNotShake", g_state.noShake);
    mod->setSavedValue("isTransitionOff", g_state.noTransitions);
}

void applySpeedhack() {
    auto director = CCDirector::sharedDirector();

    if (!director)
        return;

    float speed = g_state.speedhack
        ? g_state.speed
        : 1.0f;

    director->getScheduler()->setTimeScale(speed);

    if (g_state.speedhackSong) {
        auto audio = FMODAudioEngine::sharedEngine();

        if (audio && audio->m_globalChannel) {
            audio->m_globalChannel->setPitch(speed);
        }
    }
 }

void applyPatches() {
    // Los patches específicos de plataforma y versión
    // deben implementarse en archivos separados.
 }

}
