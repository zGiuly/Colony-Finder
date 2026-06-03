#pragma once

#include "update/IUpdateChecker.h"

class GithubUpdateChecker : public IUpdateChecker
{
public:
    GithubUpdateChecker(const std::string& owner, const std::string& repo, const std::string& assetName);
    UpdateInfo Check(const std::string& currentVersion) override;

private:
    std::string owner;
    std::string repo;
    std::string assetName;
};
