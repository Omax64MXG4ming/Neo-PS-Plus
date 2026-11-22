#pragma once

namespace cobalt {

struct State {
    // Menu
    bool showOnStartup = false;
    bool debug = false;

    // Speedhack
    bool speedhack = false;
    bool speedhackSong = false;
    float speed = 1.0f;

    // Gameplay
    bool noclip = false;
    bool noCollision = false;
    bool autoComplete = false;

    // Level
    bool autoLDM = false;
    bool copyHack = false;
    bool editHack = false;
    bool disableQuit = false;

    // Bypass
    bool iconBypass = false;
    bool lockAllIcons = false;
    bool characterFilterBypass = false;
    bool sliderHack = false;
    bool autoVerify = false;

    // Visual
    bool noHide = false;
    bool noDeathEffect = false;
    bool noShake = false;
    bool noTransitions = false;
};

extern State g_state;

void load();
void save();

void applySpeedhack();
void applyPatches();

}
