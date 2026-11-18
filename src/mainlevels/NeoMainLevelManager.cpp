#include "NeoMainLevelManager.hpp"

namespace neo::mainlevels {

NeoMainLevelManager& NeoMainLevelManager::get() {
    static NeoMainLevelManager instance;
    return instance;
}

const NeoMainLevel* NeoMainLevelManager::getLevel(int id) const {
    return NeoLevelRegistry::getLevel(id);
}

bool NeoMainLevelManager::isNeoLevel(int id) const {
    return NeoLevelRegistry::exists(id);
}

const std::vector<NeoMainLevel>&
NeoMainLevelManager::getLevels() const {
    return NeoLevelRegistry::getLevels();
}

}
