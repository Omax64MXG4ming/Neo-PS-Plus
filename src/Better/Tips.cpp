#include <Geode/Geode.hpp>
#include <Geode/loader/SettingV3.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/binding/ButtonSprite.hpp>
#include <Geode/binding/CCMenuItemSpriteExtra.hpp>
#include <Geode/binding/TextArea.hpp>
#include <random>
using namespace geode::prelude;

// helpers — splits on literal \n in the saved string
static std::vector<std::string> getLines() {
    return {
        "Welcome to Neo PS!",
        "Enjoy playing Geometry Dash!",
        "Thanks for playing!",
        "Have fun!",
        "Good luck!"
    };
}
