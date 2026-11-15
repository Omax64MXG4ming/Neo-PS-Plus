#include "NeoGDHttpClient.hpp"
#include "NeoGDIntercept.hpp"

#include <string>
#include <map>
#include <vector>
#include <cctype>

using namespace cocos2d::extension;

namespace neo::intercept {

namespace {

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");

    if (first == std::string::npos) {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\r\n");

    return value.substr(first, last - first + 1);
}

std::map<std::string, std::string> extractHeaders(
    CCHttpRequest* request
) {
    std::map<std::string, std::string> headers;

    if (!request) {
        return headers;
    }

    const auto rawHeaders = request->getHeaders();

    for (const auto& header : rawHeaders) {
        const auto separator = header.find(':');

        if (separator == std::string::npos) {
            continue;
        }

        const std::string name = trim(
            header.substr(0, separator)
        );

        const std::string value = trim(
            header.substr(separator + 1)
        );

        if (!name.empty()) {
            headers[name] = value;
        }
    }

    return headers;
}

std::string extractBody(CCHttpRequest* request) {
    if (!request) {
        return "";
    }

    const char* data = request->getRequestData();
    const unsigned int size = request->getRequestDataSize();

    if (!data || size == 0) {
        return "";
    }

    return std::string(data, size);
}

}

void NeoGDHttpClient::send(CCHttpRequest* request) {
    if (request) {
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

        const char* rawURL = request->getUrl();

        const std::string url = rawURL
            ? rawURL
            : "";

        const auto headers = extractHeaders(request);
        const auto body = extractBody(request);

        NeoGDIntercept::requestStarted(
            method,
            url,
            headers,
            body
        );
    }

    // Ejecutar el send original de Geometry Dash.
    CCHttpClient::send(request);
}

}
