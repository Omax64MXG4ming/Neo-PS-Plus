#include <neo-launchercore/LauncherCore.hpp>

void checkLauncherCore() {
    auto result = neo::launcher::api::isReady();

    if (!result) {
        log::warn("Neo PS LauncherCore is not available.");
        return;
    }

    if (!result.unwrap()) {
        log::warn("Neo PS LauncherCore has not initialized.");
        return;
    }

    auto version = neo::launcher::api::getVersion();

    log::info(
        "Neo PS LauncherCore detected. Version: {}",
        version ? version.unwrap() : "Unknown"
    );
}
