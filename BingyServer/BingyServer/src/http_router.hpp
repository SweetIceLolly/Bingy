/*
描述: Bingy 服务器的路由初始化接口
作者: 冰棍
文件: http_router.hpp
*/

#pragma once

#include "rest_server/rest_server.hpp"

// 初始化 HTTP 路由
bool init_server_router(const rest_server &server);