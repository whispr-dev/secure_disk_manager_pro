#pragma once
#include <string>
#include <vector>

class IdentitySwitcher {
public:
    IdentitySwitcher(const std::string& identityBasePath);

    bool switchToIdentity(const std::string& identityName);
    std::vector<std::string> listIdentities() const;

private:
    std::string basePath;
};
