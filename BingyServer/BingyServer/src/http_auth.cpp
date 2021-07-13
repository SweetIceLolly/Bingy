/*
描述: Bingy 服务器的 HTTP 应用验证处理
作者: 冰棍
文件: http_auth.hpp
*/

#include "http_auth.hpp"
#include <unordered_map>

std::unordered_map<std::string, std::string> allHttpApps;

void bg_http_add_app(const std::string &appId, const std::string &secret) {
    allHttpApps.insert({ appId, secret });
}

bool bg_http_remove_app(const std::string &appId) {
    return (allHttpApps.erase(appId) == 1);
}

bool bg_http_app_exists(const std::string &appId) {
    return (allHttpApps.find(appId) == allHttpApps.end());
}

bool bg_http_app_auth(const std::string &appId, const std::string &secret) {
    auto it = allHttpApps.find(appId);
    if (it == allHttpApps.end())
        return false;
    return (it->second == secret);
}
