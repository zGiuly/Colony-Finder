#include "update/GithubUpdateChecker.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>

namespace
{
    size_t WriteToString(void* contents, size_t size, size_t nmemb, void* userp)
    {
        size_t total = size * nmemb;
        std::string* out = static_cast<std::string*>(userp);
        out->append(static_cast<char*>(contents), total);
        return total;
    }

    std::string StripV(const std::string& tag)
    {
        if (!tag.empty() && (tag[0] == 'v' || tag[0] == 'V'))
        {
            return tag.substr(1);
        }
        return tag;
    }

    bool IsNewer(const std::string& latest, const std::string& current)
    {
        auto parse = [](const std::string& v) {
            std::vector<int> parts;
            std::stringstream ss(v);
            std::string segment;
            while (std::getline(ss, segment, '.'))
            {
                try { parts.push_back(std::stoi(segment)); }
                catch (...) { parts.push_back(0); }
            }
            while (parts.size() < 3) parts.push_back(0);
            return parts;
        };

        auto a = parse(latest);
        auto b = parse(current);
        for (size_t i = 0; i < 3; ++i)
        {
            if (a[i] > b[i]) return true;
            if (a[i] < b[i]) return false;
        }
        return false;
    }
}

GithubUpdateChecker::GithubUpdateChecker(const std::string& ownerVal, const std::string& repoVal, const std::string& assetNameVal)
    : owner(ownerVal), repo(repoVal), assetName(assetNameVal)
{
}

UpdateInfo GithubUpdateChecker::Check(const std::string& currentVersion)
{
    UpdateInfo info;

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        info.error = "curl init failed";
        return info;
    }

    std::string url = "https://api.github.com/repos/" + owner + "/" + repo + "/releases/latest";
    std::string response;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "ColonyFinder-Updater");

    CURLcode rc = curl_easy_perform(curl);
    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK)
    {
        info.error = std::string("network error: ") + curl_easy_strerror(rc);
        return info;
    }
    if (httpCode != 200)
    {
        info.error = "github http " + std::to_string(httpCode);
        return info;
    }

    try
    {
        auto json = nlohmann::json::parse(response);
        std::string tag = json.value("tag_name", "");
        if (tag.empty())
        {
            info.error = "missing tag_name";
            return info;
        }

        info.latestVersion = StripV(tag);
        if (!IsNewer(info.latestVersion, currentVersion))
        {
            info.available = false;
            return info;
        }

        if (!json.contains("assets") || !json["assets"].is_array())
        {
            info.error = "no assets in release";
            return info;
        }

        for (const auto& asset : json["assets"])
        {
            std::string name = asset.value("name", "");
            if (name == assetName)
            {
                info.downloadUrl = asset.value("browser_download_url", "");
                info.available = !info.downloadUrl.empty();
                return info;
            }
        }

        info.error = "asset not found: " + assetName;
        return info;
    }
    catch (const std::exception& ex)
    {
        info.error = std::string("json parse error: ") + ex.what();
        return info;
    }
}
