#include <Geode/Geode.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <random>

using namespace geode::prelude;

// Loading tips
static std::vector<std::string> getLines() {
    return {
        "Welcome to Neo PS!",
        "Que Quieres",
        "Modified by Omar Studios Inc",
        "Maxwell House is in The World",
        "Pebbles",
        "The ",
        "Minecraft",
        "itzkjohn :fire:",
        "More than 61 FPS on the game",
        "plus mod, i remember Plus GDPS...",
        "GTA VI is Out (Bugs)",
        "Hola Compa",
        "Aver,Aver,Aver, como esta eso?",
        "BEST FUTBOL GAME EVER :keydemon:",
        "I know you, you are thst player",
        "Geode Inc",
        "There was a time when a cube saw a spike and jumped over it.",
        "you data has been Losed",
        "F",
        "Robert Has Been Renovated",
        "Emmanuel Was Here Fixing The Server",
        "Emmanuel",
        "Son Hermano",
        "Bruh",
        "Trying to Solve your IP",
        "Wdym",
        "F***, i dont see you",
        "MenuLayer has Crashed, Exiting The Game",
        "Nothing Here yet",
        "2.21 in middles of 2027",
        "Neo PS",
        "Ese,Viste el Pajarito ese en el Campo :eyes:",
        "No te dejo entrar",
        "Powered By PowerPoint",
        "Give and CP only for AFK",
        "Android TV",
        "Oye, que vas a hacer con lo que dijiste",
        "Ah?, espera, porque me despiertas",
        "Nada más vine a jugar contigo"
    };
}

// LoadingLayer
class $modify(MyLoadingLayer, LoadingLayer) {
    bool init(bool refresh) {
        if (!LoadingLayer::init(refresh))
            return false;

        static std::mt19937 rng(std::random_device{}());

        auto lines = getLines();

        if (lines.empty())
            return true;

        std::uniform_int_distribution<size_t> dist(0, lines.size() - 1);

        auto const& chosen = lines[dist(rng)];

        if (m_textArea) {
            m_textArea->setString(chosen.c_str());
        }

        return true;
    }
};
