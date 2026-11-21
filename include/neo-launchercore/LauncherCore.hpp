void checkLauncherCore() {

    auto result = neo::launcher::api::isReady();

    if (!result) {
        log::warn(
            "Neo PS LauncherCore is not available."
        );

        return;
    }

    if (result.unwrap()) {
        log::info(
            "Neo PS LauncherCore detected and ready."
        );

        auto version =
            neo::launcher::api::getVersion();

        if (version) {
            log::info(
                "LauncherCore version: {}",
                version.unwrap()
            );
        }

        return;
    }

    log::warn(
        "Neo PS LauncherCore has not initialized."
    );
}
