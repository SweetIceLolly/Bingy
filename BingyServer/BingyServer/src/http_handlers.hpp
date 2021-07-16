/*
描述: Bingy 服务器的 HTTP 请求处理接口
作者: 冰棍
文件: http_handlers.hpp
*/

#pragma once

#include "rest_server/rest_server.hpp"

// 懒人宏
// 定义形如 bg_cmd_xxx 的 HTTP 请求回调函数
#define CMD(cmd) void bg_cmd_ ##cmd (mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data)

// HTTP 服务器的 poll 回调函数
void bg_server_poll(mg_connection *connection, int &ev, mg_http_message *ev_data, void *fn_data);

// 用来存储 HTTP 请求内容的结构体
typedef struct _http_req {
    struct mg_str   query;              // 请求的 query. 记得用完之后 free 对应的指针
    char            *body;              // 请求的 body. 记得用完之后 free

    unsigned        *signal;            // 信号. 把它设置为 1 代表响应完毕, 该回应随之会被发送给客户端
    char            **res_body;         // HTTP 返回的文本
    int             *res_http_code;     // HTTP 返回码
} http_req;

// 写入请求回应
inline void bg_http_reply(http_req *req, const int &http_code, const char *body) {
    *req->res_http_code = http_code;
    *(req->res_body) = strdup(body);
    *req->signal = 1;
}

// 发送请求错误回应
inline void bg_http_reply_error(http_req *req, const int &http_code, const std::string &msg, const int &errid) {
    bg_http_reply(req, http_code, ("{ \"msg\": \"" + msg + "\", \"errid\": " + std::to_string(errid) + "}").c_str());
}

/**
 * 新玩家注册
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupid: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 无其余内容
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(register);

/**
 * 查看硬币
 * 类型: GET
 * 参数位置: query
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupid: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回 coins
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(view_coins);

/**
 * 签到
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupid: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回
 *      errors: 存有所有签到期间发生的错误的数组, 格式为 (错误描述, 错误号)
 *      signInCountCont: 连续签到天数
 *      signInCount: 签到次数
 *      deltaCoins: 获得硬币
 *      coins: 当前硬币
 *      deltaEnergy: 获得体力
 *      energy: 当前体力
 *      deltaExp: 获得经验
 *      eventMsg: 活动消息. 若没有活动则为空字符串
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(sign_in);
