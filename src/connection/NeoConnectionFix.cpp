#include "NeoConnectionFix.hpp"
#include <string>

using namespace cocos2d::extension;
namespace neo::connection {

static constexpr const char* BASE_URL =
    "https://www.boomlings.com/database/";
static constexpr const char* SECONDARY_URL =
    "http://www.boomlings.com/database/";
void NeoConnectionFix::send(CCHttpRequest* request) {

    if (request) {
        const char* rawURL = request->getUrl();
        if (rawURL) {
            std::string url = rawURL;
            // Get configured backend URL
            std::string baseURL =
            "https://neops.x10.mx/database/";
            // Normalize trailing slash
            if (!baseURL.empty() && baseURL.back() != '/') {
                baseURL += '/';
            }
            // Only redirect when a backend is configured
            if (!baseURL.empty()) {
                // HTTPS Geometry Dash
                if (url.starts_with(BASE_URL)) {
                    url.replace(
                        0,
                        std::string(BASE_URL).length(),
                        baseURL
                    );
                    request->setUrl(url.c_str());
                }
                // HTTP Geometry Dash
                else if (url.starts_with(SECONDARY_URL)) {
                    url.replace(
                        0,
                        std::string(SECONDARY_URL).length(),
                        baseURL
                    );

                    request->setUrl(url.c_str());
                }
            }
        }
    }
    CCHttpClient::send(request);
  }
} // namespace neo::connection
