/*
描述: Bingy 服务器的 HTTP 应用验证处理接口
作者: 冰棍
文件: http_auth.hpp
*/

#pragma once

#include <string>

// 所有已知的应用 ID
#define APPID_BINGY_GAME  "bggame"

// 注册新的应用
// 注意, 该函数不是线程安全的
void bg_http_add_app(const std::string &appId, const std::string &secret);

// 移除指定应用
// 注意, 该函数不是线程安全的
bool bg_http_remove_app(const std::string &appId);

// 检查指定应用是否存在
bool bg_http_app_exists(const std::string &appId);

// 验证指定应用的 AppId 和对应的 Secret
bool bg_http_app_auth(const std::string &appId, const std::string &secret);
