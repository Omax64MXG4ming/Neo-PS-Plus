#include "NeoConnectionFix.hpp"
#include <string>

using namespace cocos2d::extension;

namespace neo::connection {

static constexpr const char* BASE_URL =
    "https://www.boomlings.com/database/";

static constexpr const char* SECONDARY_URL =
    "http://www.boomlings.com/database/";

static constexpr const char* NEO_URL =
    "https://neops.x10.mx/database/";

void NeoConnectionFix::send(CCHttpRequest* request) {
    if (!request) {
        CCHttpClient::send(request);
        return;
    }

    const char* rawURL = request->getUrl();

    if (rawURL) {
        std::string url = rawURL;
        std::string baseURL = NEO_URL;

        if (!baseURL.empty() && baseURL.back() != '/') {
            baseURL += '/';
        }

        if (url.starts_with(BASE_URL)) {
            url.replace(
                0,
                std::string(BASE_URL).length(),
                baseURL
            );

            request->setUrl(url.c_str());
        }
        else if (url.starts_with(SECONDARY_URL)) {
            url.replace(
                0,
                std::string(SECONDARY_URL).length(),
                baseURL
            );

            request->setUrl(url.c_str());
        }
    }

    CCHttpClient::send(request);
}

} // namespace neo::connection
