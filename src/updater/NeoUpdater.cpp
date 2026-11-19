#include "NeoUpdater.hpp"

#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>

#include <array>
#include <string>
#include <vector>
#include <cctype>

using namespace geode::prelude;

namespace neo::updater {

static constexpr const char* RELEASE_API =
    "https://api.github.com/repos/Omax64MXG4ming/Neo-PS-Plus/releases/latest";

static constexpr const char* RELEASE_PAGE =
    "https://github.com/Omax64MXG4ming/Neo-PS-Plus/releases";

static bool s_checked = false;

// --------------------------------------------------
// VERSION PARSER
// --------------------------------------------------

static std::array<int, 3> parseVersion(std::string version) {

    std::array<int, 3> result = {0, 0, 0};

    size_t pos = 0;

    if (!version.empty() &&
        (version[0] == 'v' || version[0] == 'V')) {
        pos = 1;
    }

    for (int i = 0; i < 3; i++) {

        while (pos < version.size() &&
               !std::isdigit(
                   static_cast<unsigned char>(version[pos])
               )) {
            if (version[pos] == '-' || version[pos] == '+')
                return result;

            pos++;
        }

        if (pos >= version.size())
            break;

        while (pos < version.size() &&
               std::isdigit(
                   static_cast<unsigned char>(version[pos])
               )) {

            result[i] =
                result[i] * 10 + (version[pos] - '0');

            pos++;
        }

        if (pos < version.size() && version[pos] == '.')
            pos++;
        else
            break;
    }

    return result;
}

// --------------------------------------------------
// VERSION COMPARISON
// --------------------------------------------------

static bool isNewer(
    const std::string& remote,
    const std::string& local
) {

    auto remoteVersion = parseVersion(remote);
    auto localVersion = parseVersion(local);

    for (int i = 0; i < 3; i++) {

        if (remoteVersion[i] > localVersion[i])
            return true;

        if (remoteVersion[i] < localVersion[i])
            return false;
    }

    return false;
}

// --------------------------------------------------
// CHECK GITHUB RELEASE
// --------------------------------------------------

void NeoUpdater::checkForUpdates() {

    if (s_checked)
        return;

    s_checked = true;

    log::info("[NeoUpdater] Checking GitHub releases...");

    auto request = web::WebRequest()
        .header("Accept", "application/vnd.github+json")
        .header("X-GitHub-Api-Version", "2022-11-28")
        .userAgent("Neo-PS-Plus-Updater")
        .get(RELEASE_API);

    async::spawn(
        std::move(request),

        [](web::WebResponse response) {

            if (!response.ok()) {

                log::error(
                    "[NeoUpdater] GitHub request failed: HTTP {}",
                    response.code()
                );

                return;
            }

            auto jsonResult = response.json();

            if (!jsonResult) {
                log::error("[NeoUpdater] Invalid GitHub JSON");
                return;
            }

            auto json = jsonResult.unwrap();

            // --------------------------------------
            // RELEASE VERSION
            // --------------------------------------

            auto tagResult =
                json.get<std::string>("tag_name");

            if (!tagResult) {
                log::error("[NeoUpdater] Missing tag_name");
                return;
            }

            std::string remoteVersion =
                tagResult.unwrap();

            // --------------------------------------
            // CURRENT VERSION
            // --------------------------------------

            std::string localVersion =
                fmt::to_string(
                    Mod::get()->getVersion()
                );

            log::info(
                "[NeoUpdater] Installed: {} | Latest: {}",
                localVersion,
                remoteVersion
            );

            // --------------------------------------
            // COMPARE
            // --------------------------------------

            if (!isNewer(remoteVersion, localVersion)) {

                log::info(
                    "[NeoUpdater] Mod is up to date"
                );

                return;
            }

            // --------------------------------------
            // FIND GEODE ASSET
            // --------------------------------------

            auto assetsResult = json.get("assets");

            if (!assetsResult) {
                log::error("[NeoUpdater] No assets found");
                return;
            }

            auto assetsArray =
                assetsResult.unwrap().asArray();

            if (!assetsArray) {
                log::error("[NeoUpdater] Invalid assets");
                return;
            }

            std::string downloadURL;

            for (auto const& asset : assetsArray.unwrap()) {

                auto nameResult =
                    asset.get<std::string>("name");

                auto urlResult =
                    asset.get<std::string>(
                        "browser_download_url"
                    );

                if (!nameResult || !urlResult)
                    continue;

                std::string name = nameResult.unwrap();

                // Only accept our mod package.
                if (
                    name.find("omarstudios.neo") == 0 &&
                    name.size() >= 6 &&
                    name.substr(name.size() - 6) == ".geode"
                ) {

                    downloadURL = urlResult.unwrap();
                    break;
                }
            }

            if (downloadURL.empty()) {

                log::error(
                    "[NeoUpdater] No compatible .geode asset found"
                );

                return;
            }

            log::info(
                "[NeoUpdater] Update available: {}",
                remoteVersion
            );

            NeoUpdater::showUpdate(
                remoteVersion,
                downloadURL
            );
        }
    );
}

// --------------------------------------------------
// UPDATE ALERT
// --------------------------------------------------

void NeoUpdater::showUpdate(
    const std::string& version,
    const std::string& downloadURL
) {

    createQuickPopup(
        "Neo PS Plus Update",

        fmt::format(
            "A new version of <cy>Neo PS Plus</c> is available!\n\n"
            "Installed version: <cr>{}</c>\n"
            "Latest version: <cg>{}</c>\n\n"
            "Would you like to download the update?",
            fmt::to_string(Mod::get()->getVersion()),
            version
        ),

        "Later",
        "Download",

        [downloadURL](auto, bool btn2) {

            if (!btn2)
                return;

            log::info(
                "[NeoUpdater] Opening update download"
            );

            web::openLinkInBrowser(downloadURL);
        }
    );
}

}
