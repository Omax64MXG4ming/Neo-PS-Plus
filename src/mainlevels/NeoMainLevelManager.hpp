#pragma once

#include "NeoLevelRegistry.hpp"

namespace neo::mainlevels {

class NeoMainLevelManager {
public:
    static NeoMainLevelManager& get();
    const NeoMainLevel* getLevel(int id) const;
    bool isNeoLevel(int id) const;
    const std::vector<NeoMainLevel>& getLevels() const;

private:
    NeoMainLevelManager() = default;
};

}
