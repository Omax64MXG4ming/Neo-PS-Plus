#pragma once

#include <Geode/Geode.hpp>
#include <string>

namespace neo::updater {

class NeoUpdater {
public:
    static void checkForUpdates();

private:
    static void showUpdate(
        const std::string& version,
        const std::string& downloadURL
    );
};

}
