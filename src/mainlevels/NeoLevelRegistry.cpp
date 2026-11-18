#include "NeoLevelRegistry.hpp"

namespace neo::mainlevels {

static const std::vector<NeoMainLevel> s_levels = {

    {
        10001,
        "Neo Awakening",
        GJDifficulty::Easy,
        3,
        1,
        "",
        "",
        "",
        false
    },

    {
        10002,
        "Neo Dimension",
        GJDifficulty::Hard,
        6,
        2,
        "",
        "",
        "",
        false
    },

    {
        10003,
        "Neo Revolution",
        GJDifficulty::Harder,
        7,
        3,
        "",
        "",
        "",
        false
    }

};

const std::vector<NeoMainLevel>& NeoLevelRegistry::getLevels() {
    return s_levels;
}

const NeoMainLevel* NeoLevelRegistry::getLevel(int id) {
    for (const auto& level : s_levels) {
        if (level.id == id) {
            return &level;
        }
    }

    return nullptr;
}

bool NeoLevelRegistry::exists(int id) {
    return getLevel(id) != nullptr;
}

size_t NeoLevelRegistry::count() {
    return s_levels.size();
}

}
