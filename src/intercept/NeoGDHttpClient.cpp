#include "NeoGDHttpClient.hpp"
#include "NeoGDIntercept.hpp"

#include <string>
#include <map>
#include <vector>
#include <utility>

using namespace cocos2d;
using namespace cocos2d::extension;

namespace neo::intercept {

namespace {

// --------------------------------------------------
// TRIM
// --------------------------------------------------

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos)
        return "";

    const auto last = value.find_last_not_of(" \t\r\n");

    return value.substr(first, last - first + 1);
}

// --------------------------------------------------
// EXTRACT HEADERS
// --------------------------------------------------

std::map<std::string, std::string> extractHeaders(
    CCHttpRequest* request
) {
    std::map<std::string, std::string> headers;

    if (!request)
        return headers;

    const auto rawHeaders = request->getHeaders();

    for (const auto& header : rawHeaders) {
        const auto separator = header.find(':');

        if (separator == std::string::npos)
            continue;

        const std::string name = trim(
            header.substr(0, separator)
        );

        const std::string value = trim(
            header.substr(separator + 1)
        );

        if (!name.empty())
            headers[name] = value;
    }

    return headers;
}

// --------------------------------------------------
// EXTRACT REQUEST BODY
// --------------------------------------------------

std::string extractBody(CCHttpRequest* request) {
    if (!request)
        return "";

    const char* data = request->getRequestData();
    const int size = request->getRequestDataSize();

    if (!data || size <= 0)
        return "";

    return std::string(data, static_cast<size_t>(size));
}

// --------------------------------------------------
// EXTRACT RESPONSE BODY
// --------------------------------------------------

std::string extractResponseBody(
    CCHttpResponse* response
) {
    if (!response)
        return "";

    auto* data = response->getResponseData();

    if (!data || data->empty())
        return "";

    return std::string(data->begin(), data->end());
}

// --------------------------------------------------
// RESPONSE CALLBACK PROXY
// --------------------------------------------------

class NeoHttpResponseProxy : public CCObject {
public:

    NeoHttpResponseProxy(
        uint64_t requestID,
        CCObject* originalTarget,
        SEL_HttpResponse originalSelector
    )
        : m_requestID(requestID),
          m_originalTarget(originalTarget),
          m_originalSelector(originalSelector) {

        // Keep the original callback target alive.
        if (m_originalTarget)
            m_originalTarget->retain();
    }

    ~NeoHttpResponseProxy() override {
        if (m_originalTarget) {
            m_originalTarget->release();
            m_originalTarget = nullptr;
        }
    }

    void onResponse(
        CCHttpClient* client,
        CCHttpResponse* response
    ) {

        // ------------------------------------------
        // UPDATE INSPECTOR
        // ------------------------------------------

        if (response) {

            const int responseCode =
                response->getResponseCode();

            if (response->isSucceed()) {

                const std::string responseBody =
                    extractResponseBody(response);

                NeoGDIntercept::requestCompleted(
                    m_requestID,
                    responseCode,
                    responseBody
                );

            } else {

                const char* rawError =
                    response->getErrorBuffer();

                const std::string error =
                    rawError && rawError[0]
                        ? rawError
                        : "HTTP request failed";

                NeoGDIntercept::requestFailed(
                    m_requestID,
                    error
                );
            }

        } else {

            NeoGDIntercept::requestFailed(
                m_requestID,
                "HTTP response is null"
            );
        }

        // ------------------------------------------
        // EXECUTE ORIGINAL GAME CALLBACK
        // ------------------------------------------

        if (m_originalTarget && m_originalSelector) {

            (m_originalTarget->*m_originalSelector)(
                client,
                response
            );
        }

        // ------------------------------------------
        // RELEASE PROXY
        // ------------------------------------------

        this->release();
    }

private:

    uint64_t m_requestID = 0;

    CCObject* m_originalTarget = nullptr;

    SEL_HttpResponse m_originalSelector = nullptr;
};

}

// --------------------------------------------------
// HTTP CLIENT INTERCEPT
// --------------------------------------------------

void NeoGDHttpClient::send(CCHttpRequest* request) {

    if (!request) {
        CCHttpClient::send(request);
        return;
    }

    // ----------------------------------------------
    // REQUEST METHOD
    // ----------------------------------------------

    const char* method = "UNKNOWN";

    switch (request->getRequestType()) {

        case CCHttpRequest::kHttpGet:
            method = "GET";
            break;

        case CCHttpRequest::kHttpPost:
            method = "POST";
            break;

        case CCHttpRequest::kHttpPut:
            method = "PUT";
            break;

        case CCHttpRequest::kHttpDelete:
            method = "DELETE";
            break;

        default:
            break;
    }

    // ----------------------------------------------
    // REQUEST DATA
    // ----------------------------------------------

    const char* rawURL = request->getUrl();

    const std::string url =
        rawURL ? rawURL : "";

    const auto headers = extractHeaders(request);
    const auto body = extractBody(request);

    // ----------------------------------------------
    // SAVE ORIGINAL CALLBACK
    // ----------------------------------------------

    CCObject* originalTarget = request->getTarget();

    SEL_HttpResponse originalSelector =
        static_cast<SEL_HttpResponse>(
            request->getSelector()
        );

    // ----------------------------------------------
    // REGISTER REQUEST
    // ----------------------------------------------

    const uint64_t requestID =
        NeoGDIntercept::requestStarted(
            method,
            url,
            headers,
            body
        );

    // ----------------------------------------------
    // INSTALL RESPONSE CALLBACK
    // ----------------------------------------------

    auto proxy = new NeoHttpResponseProxy(
        requestID,
        originalTarget,
        originalSelector
    );

    request->setResponseCallback(
        proxy,
        httpresponse_selector(
            NeoHttpResponseProxy::onResponse
        )
    );

    // ----------------------------------------------
    // ORIGINAL HTTP SEND
    // ----------------------------------------------

    CCHttpClient::send(request);
}

}
