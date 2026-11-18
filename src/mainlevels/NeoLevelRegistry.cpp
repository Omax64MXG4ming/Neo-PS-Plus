#include "NeoLevelRegistry.hpp"

namespace neo::mainlevels {

const std::vector<NeoMainLevel> NeoLevelRegistry::s_levels = {

    {
        10001,
        "Neo Awakening",
        GJDifficulty::Easy,
        3,
        1,
        "",
        "",
        "kS38,1_0_2_0_3_0",
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
        "kS38,1_0_2_0_3_0",
        false
    }

};

const std::vector<NeoMainLevel>& NeoLevelRegistry::getLevels() {
    return s_levels;
}

const NeoMainLevel* NeoLevelRegistry::getLevel(int id) {
    for (const auto& level : s_levels) {
        if (level.id == id)
            return &level;
    }

    return nullptr;
}

bool NeoLevelRegistry::exists(int id) {
    return getLevel(id) != nullptr;
}

}
