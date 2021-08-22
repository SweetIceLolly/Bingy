/*
描述: REST HTTP 服务器相关接口
作者: 冰棍
文件: rest_server.hpp
*/

#pragma once

#if !defined(_MSC_VER)
#include "mongoose.h"
#else
extern "C" {
#include "mongoose.h"
}
#endif

#include <unordered_map>
#include <string>
#include <functional>

// 请求处理函数
typedef std::function<void(mg_connection *connection, int &ev, mg_http_message *ev_data, void *fn_data)> handler;

// 请求处理函数信息
typedef struct _handlerInfo {
    char            method[8];
    handler         eventHandler;
} handlerInfo;

// 代表某个请求处理函数记录
typedef std::unordered_map<std::string, handlerInfo>::iterator handler_identifier;

class rest_server {
public:
    handler pollHandler;            // poll 处理回调函数

    // 添加请求处理
    handler_identifier addHandler(const std::string &method, const std::string &path, const handler &eventHandler);

    // 移除请求处理
    void removeHandler(const handler_identifier &item);

    // 设置 poll 处理
    void setPollHandler(const handler &eventHandler);

    // 移除 poll 处理
    void removePollHandler();

    // 启动服务器
    void startServer(const std::string &connStr, int pollFreq, void *userdata);

    // 停止服务器
    void stopServer();

    // 内部使用, 匹配合适的请求处理函数. 该函数保证返回值不为 NULL
    handler matchHandler(const struct mg_str &method, const std::string &path);

private:
    // 路由
    std::unordered_map<std::string, handlerInfo> router;

    // 服务器是否正在停止
    bool stopping = false;
};

typedef struct _dispatcherInfo {
    rest_server  *ptrToClass;
    void        *userdata;
} dispatcherInfo;

std::string get_query_param(const struct mg_str *query, const char *fieldName);
