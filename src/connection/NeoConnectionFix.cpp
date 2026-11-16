#include "NeoConnectionFix.hpp"
#include "../config/NeoConfig.hpp"
using namespace cocos2d::extension;

namespace neo::connection {

static constexpr const char* BASE_URL =
    "https://www.boomlings.com/database/";

static constexpr const char* SECONDARY_URL =
    "http://www.boomlings.com/database/";

static constexpr const char* NEO_URL =
std::string m_baseURL =
            neo::config::getBackendURL();

void NeoConnectionFix::send(CCHttpRequest* request) {

    
    if (request) {
        const char* rawURL = request->getUrl();

        if (rawURL) {
            std::string url = rawURL;

            if (url.starts_with(BASE_URL)) {
                url.replace(
                    0,
                    std::string(BASE_URL).length(),
                    NEO_URL
                );

                request->setUrl(url.c_str());
            }
            else if (url.starts_with(SECONDARY_URL)) {
                url.replace(
                    0,
                    std::string(SECONDARY_URL).length(),
                    NEO_URL
                );

                request->setUrl(url.c_str());
            }
        }
    }

    CCHttpClient::send(request);
  }
}
