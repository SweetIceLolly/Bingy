/*
描述: 消息路由头文件, 提供消息分发接口
作者: 冰棍
文件: msg_router.hpp
*/

#pragma once

#include "msg_handlers.hpp"

void bg_groupmsg_dispatch(const cq::MessageEvent &ev);
void bg_privatemsg_dispatch(const cq::MessageEvent &ev);

// msg 参数格式为: bg 命令
void bg_groupmsg_router_add(const std::string &msg, const std::function<void(const cq::MessageEvent &ev)> &handler);
void bg_privatemsg_router_add(const std::string &msg, const std::function<void(const cq::MessageEvent &ev)> &handler);
