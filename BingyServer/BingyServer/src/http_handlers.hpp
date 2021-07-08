/*
描述: Bingy 服务器的 HTTP 请求处理接口
作者: 冰棍
文件: http_handlers.hpp
*/

#pragma once

#include "rest_server/rest_server.hpp"

#define CMD(cmd) void bg_cmd_##cmd##(mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data)

/**
 * 客户端验证
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 * 返回值:
 *  200: 成功
 *  400: 失败, 详情见返回的 msg
 *  500: 内部错误, 详情见返回的 msg
 */
CMD(auth);

/**
 * 新玩家注册
 * 参数:
 *  token: 认证
 *  groupid: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功
 *  400: 失败, 详情见返回的 msg
 *  500: 内部错误, 详情见返回的 msg
 */
CMD(register);
