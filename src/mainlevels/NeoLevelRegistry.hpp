#pragma once

#include <Geode/Geode.hpp>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace neo::mainlevels {

struct NeoMainLevel {
    int id;
    std::string name;

    GJDifficulty difficulty;
    int stars;

    int audioTrack;
    std::string songIDs;
    std::string sfxIDs;

    std::string levelString;

    bool twoPlayerMode = false;
};

class NeoLevelRegistry {
public:
    static const std::vector<NeoMainLevel>& getLevels();

    static const NeoMainLevel* getLevel(int id);

    static bool exists(int id);

    static size_t count();
};

}
