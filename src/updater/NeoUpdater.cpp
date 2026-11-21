#include "NeoUpdater.hpp"

#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>

#include <array>
#include <string>
#include <vector>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <algorithm>

using namespace geode::prelude;

namespace neo::updater {

// --------------------------------------------------
// GitHub
// --------------------------------------------------

static constexpr const char* RELEASE_API =
    "https://api.github.com/repos/Omax64MXG4ming/Neo-PS-Plus/releases/latest";

static constexpr const char* RELEASE_PAGE =
    "https://github.com/Omax64MXG4ming/Neo-PS-Plus/releases";

static constexpr const char* MOD_PACKAGE_PREFIX =
    "omarstudios.neo";

static constexpr size_t MAX_PACKAGE_SIZE =
    100 * 1024 * 1024; // 100 MB

static bool s_checked = false;
static bool s_downloading = false;

// --------------------------------------------------
// Helpers
// --------------------------------------------------

static std::string toLower(std::string value) {
    std::transform(
        value.begin(),
        value.end(),
        value.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return value;
}

static bool endsWith(
    const std::string& value,
    const std::string& suffix
) {
    return value.size() >= suffix.size() &&
        value.compare(
            value.size() - suffix.size(),
            suffix.size(),
            suffix
        ) == 0;
}

// --------------------------------------------------
// Version parser
// --------------------------------------------------

static std::array<int, 3> parseVersion(
    const std::string& version
) {
    std::array<int, 3> result = {0, 0, 0};

    size_t pos = 0;

    if (!version.empty() &&
        (version[0] == 'v' || version[0] == 'V')) {
        pos = 1;
    }

    for (int i = 0; i < 3; i++) {
        while (
            pos < version.size() &&
            !std::isdigit(
                static_cast<unsigned char>(version[pos])
            )
        ) {
            if (version[pos] == '-' || version[pos] == '+')
                return result;

            pos++;
        }

        if (pos >= version.size())
            break;

        while (
            pos < version.size() &&
            std::isdigit(
                static_cast<unsigned char>(version[pos])
            )
        ) {
            int digit = version[pos] - '0';

            if (result[i] <= (2147483647 - digit) / 10) {
                result[i] = result[i] * 10 + digit;
            }

            pos++;
        }

        if (pos < version.size() && version[pos] == '.') {
            pos++;
        } else {
            break;
        }
    }

    return result;
}

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
// Safe filename
// --------------------------------------------------

static std::string makeSafeVersion(
    const std::string& version
) {
    std::string result;

    for (unsigned char c : version) {
        if (
            std::isalnum(c) ||
            c == '.' ||
            c == '-' ||
            c == '_'
        ) {
            result.push_back(static_cast<char>(c));
        }
    }

    if (result.empty())
        result = "latest";

    return result;
}

// --------------------------------------------------
// Error popup
// --------------------------------------------------

static void showError(const std::string& message) {
    createQuickPopup(
        "Neo PS Plus Updater",
        message,
        "Close",
        nullptr
    );
}

// --------------------------------------------------
// CHECK FOR UPDATES
// --------------------------------------------------

void NeoUpdater::checkForUpdates(bool force) {
    if (s_checked && !force)
        return;

    if (s_downloading)
        return;

    s_checked = true;

    log::info("[NeoUpdater] Checking GitHub Releases...");

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

                s_checked = false;

                return;
            }

            auto jsonResult = response.json();

            if (!jsonResult) {
                log::error("[NeoUpdater] Invalid GitHub JSON");

                s_checked = false;

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

                s_checked = false;

                return;
            }

            std::string remoteVersion =
                tagResult.unwrap();

            // --------------------------------------
            // INSTALLED VERSION
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
            // COMPARE VERSIONS
            // --------------------------------------

            if (!isNewer(remoteVersion, localVersion)) {
                log::info(
                    "[NeoUpdater] Neo PS Plus is up to date."
                );

                return;
            }

            // --------------------------------------
            // GET RELEASE ASSETS
            // --------------------------------------

            auto assetsResult = json.get("assets");

            if (!assetsResult) {
                log::error("[NeoUpdater] Missing release assets");

                s_checked = false;

                return;
            }

            auto assetsArray =
                assetsResult.unwrap().asArray();

            if (!assetsArray) {
                log::error("[NeoUpdater] Invalid assets array");

                s_checked = false;

                return;
            }

            std::vector<std::pair<std::string, std::string>>
                matchingAssets;

            for (auto const& asset : assetsArray.unwrap()) {
                auto nameResult =
                    asset.get<std::string>("name");

                auto urlResult =
                    asset.get<std::string>(
                        "browser_download_url"
                    );

                if (!nameResult || !urlResult)
                    continue;

                std::string name =
                    nameResult.unwrap();

                std::string url =
                    urlResult.unwrap();

                std::string lowerName = toLower(name);

                // Only accept .geode packages belonging to this mod.
                if (
                    lowerName.find(
                        toLower(MOD_PACKAGE_PREFIX)
                    ) != 0 ||
                    !endsWith(lowerName, ".geode")
                ) {
                    continue;
                }

                // Only accept official GitHub release URLs.
                if (url.find(
                    "https://github.com/Omax64MXG4ming/Neo-PS-Plus/"
                ) != 0) {
                    continue;
                }

                matchingAssets.emplace_back(name, url);
            }

            // --------------------------------------
            // VALIDATE ASSET COUNT
            // --------------------------------------

            if (matchingAssets.empty()) {
                log::error(
                    "[NeoUpdater] No matching .geode asset found."
                );

                s_checked = false;

                return;
            }

            if (matchingAssets.size() != 1) {
                log::error(
                    "[NeoUpdater] Expected exactly one .geode asset, found {}.",
                    matchingAssets.size()
                );

                s_checked = false;

                return;
            }

            std::string downloadURL =
                matchingAssets.front().second;

            log::info(
                "[NeoUpdater] Update available: {}",
                remoteVersion
            );

            // --------------------------------------
            // SHOW UPDATE
            // --------------------------------------

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
            "Would you like to download this update?\n\n"
            "<cy>The installation will require your confirmation.</c>",
            fmt::to_string(Mod::get()->getVersion()),
            version
        ),

        "Later",
        "Download",

        [version, downloadURL](auto, bool btn2) {
            if (!btn2)
                return;

            NeoUpdater::downloadUpdate(
                version,
                downloadURL
            );
        }
    );
}

// --------------------------------------------------
// DOWNLOAD UPDATE
// --------------------------------------------------

void NeoUpdater::downloadUpdate(
    const std::string& version,
    const std::string& downloadURL
) {
    if (s_downloading)
        return;

    // Verify URL before downloading.
    if (
        downloadURL.find(
            "https://github.com/Omax64MXG4ming/Neo-PS-Plus/"
        ) != 0
    ) {
        showError(
            "Invalid update download URL."
        );

        return;
    }

    s_downloading = true;

    log::info(
        "[NeoUpdater] Downloading version {}...",
        version
    );

    auto request = web::WebRequest()
        .userAgent("Neo-PS-Plus-Updater")
    
        .get(downloadURL);

    async::spawn(
        std::move(request),

        [version](web::WebResponse response) {
            s_downloading = false;

            if (!response.ok()) {
                log::error(
                    "[NeoUpdater] Download failed: HTTP {}",
                    response.code()
                );

                showError(
                    "The update download failed.\n\n"
                    "Please check your internet connection and try again."
                );

                return;
            }

            auto const& bytes = response.data();

            // --------------------------------------
            // VALIDATE SIZE
            // --------------------------------------

            if (
                bytes.size() < 4 ||
                bytes.size() > MAX_PACKAGE_SIZE
            ) {
                log::error(
                    "[NeoUpdater] Invalid package size: {} bytes",
                    bytes.size()
                );

                showError(
                    "The downloaded package has an invalid size."
                );

                return;
            }

            // --------------------------------------
            // VALIDATE ZIP SIGNATURE
            // --------------------------------------

            if (
                bytes[0] != 0x50 ||
                bytes[1] != 0x4B
            ) {
                log::error(
                    "[NeoUpdater] Downloaded file is not a valid ZIP package."
                );

                showError(
                    "The downloaded file is not a valid .geode package."
                );

                return;
            }

            // --------------------------------------
            // CREATE UPDATE DIRECTORY
            // --------------------------------------

            std::filesystem::path saveDir(
                Mod::get()->getSaveDir().string()
            );

            std::filesystem::path updateDir =
                saveDir / "updates";

            std::error_code ec;

            std::filesystem::create_directories(
                updateDir,
                ec
            );

            if (ec) {
                log::error(
                    "[NeoUpdater] Cannot create update directory: {}",
                    ec.message()
                );

                showError(
                    "Could not create the update directory."
                );

                return;
            }

            // --------------------------------------
            // PACKAGE PATH
            // --------------------------------------

            std::string safeVersion =
                makeSafeVersion(version);

            std::filesystem::path finalPath =
                updateDir /
                (
                    std::string("omarstudios.neo-") +
                    safeVersion +
                    ".geode"
                );

            std::filesystem::path tempPath =
                finalPath.string() + ".part";

            // --------------------------------------
            // WRITE PACKAGE
            // --------------------------------------

            {
                std::ofstream file(
                    tempPath,
                    std::ios::binary | std::ios::out | std::ios::trunc
                );

                if (!file.is_open()) {
                    log::error(
                        "[NeoUpdater] Cannot create update package."
                    );

                    showError(
                        "Could not save the downloaded update."
                    );

                    return;
                }

                file.write(
                    reinterpret_cast<const char*>(bytes.data()),
                    static_cast<std::streamsize>(bytes.size())
                );

                file.flush();

                if (!file.good()) {
                    file.close();

                    std::filesystem::remove(
                        tempPath,
                        ec
                    );

                    log::error(
                        "[NeoUpdater] Failed writing update package."
                    );

                    showError(
                        "An error occurred while saving the update."
                    );

                    return;
                }
            }

            // --------------------------------------
            // FINALIZE FILE
            // --------------------------------------

            std::filesystem::remove(
                finalPath,
                ec
            );

            ec.clear();

            std::filesystem::rename(
                tempPath,
                finalPath,
                ec
            );

            if (ec) {
                std::filesystem::remove(
                    tempPath,
                    ec
                );

                log::error(
                    "[NeoUpdater] Cannot finalize package: {}",
                    ec.message()
                );

                showError(
                    "Could not finalize the downloaded update."
                );

                return;
            }

            log::info(
                "[NeoUpdater] Update downloaded successfully: {}",
                finalPath.string()
            );

            NeoUpdater::showDownloadResult(
                version,
                finalPath.string()
            );
        }
    );
}

// --------------------------------------------------
// DOWNLOAD RESULT
// --------------------------------------------------

void NeoUpdater::showDownloadResult(
    const std::string& version,
    const std::string& filePath
) {
    createQuickPopup(
        "Update Downloaded",

        fmt::format(
            "<cg>Neo PS Plus {} was downloaded successfully!</c>\n\n"
            "File:\n<cy>{}</c>\n\n"
            "The installed mod has NOT been replaced yet.\n\n"
            "Open Geode's mod manager and use its "
            "<cy>Install from file</c> option to select this package. "
            "Confirm the installation and restart Geometry Dash.",
            version,
            filePath
        ),

        "Close",
        "GitHub Releases",

        [filePath](auto, bool btn2) {
            log::info(
                "[NeoUpdater] Update package saved at: {}",
                filePath
            );

            if (btn2) {
                web::openLinkInBrowser(RELEASE_PAGE);
            }
        }
    );
}

}
