/*
描述: Bingy 服务器的 HTTP 请求处理相关函数
作者: 冰棍
文件: http_handlers.cpp
*/

#include "string.h"
#include "http_handlers.hpp"
#include "json.hpp"
#include "http_auth.hpp"
#include "game.hpp"
#include "bingy.hpp"
#include "error_codes.hpp"

using namespace nlohmann;

// 初始化 HTTP 请求信息结构体
inline http_req* get_http_req(mg_connection *connection, mg_http_message *ev_data) {
    http_req *req = static_cast<http_req *>(malloc(sizeof(http_req)));
    if (!req) {
        mg_http_reply(connection, 500, "Content-Type: application/json\r\n", "申请内存失败, 无法处理请求");
        return nullptr;
    }

    // 复制 body 内容到临时内存
    char *buf = new char[ev_data->body.len + 1];
    strncpy(buf, ev_data->body.ptr, ev_data->body.len);
    buf[ev_data->body.len] = '\0';
    req->body = strdup(buf);
    delete[] buf;
    
    // 复制 query 内容到临时内存
    buf = new char[ev_data->query.len + 1];
    strncpy(buf, ev_data->query.ptr, ev_data->query.len);
    buf[ev_data->query.len] = '\0';
    req->query = strdup(buf);
    delete[] buf;

    // 把回应写入的位置记录下来
    req->res_body = &connection->bg_res_body;
    req->res_http_code = &connection->bg_res_http_code;
    req->signal = &connection->bg_res_ready;
    return req;
}

// 释放 HTTP 请求信息结构体所占用的内存
inline void free_http_req(http_req *req) {
    free(req->body);
    free(req->query);
    free(req);
}

// HTTP 服务器的 poll 回调函数
void bg_server_poll(mg_connection *connection, int &ev, mg_http_message *ev_data, void *fn_data) {
    // 从 connection 所对应的 socket pair 接收消息. 如果接收到完整的 HTTP 响应信息, 则发送给客户端
    if (connection->bg_res_ready == 1) {
        if (connection->bg_res_body == nullptr)
            return;
        mg_http_reply(connection, connection->bg_res_http_code, "Content-Type: application/json\r\n", connection->bg_res_body);
        connection->bg_res_ready = 0;
        free(connection->bg_res_body);
        connection->bg_res_body = nullptr;
    }
}

// 新玩家注册
CMD(register) {
    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req*>(_req);

        try {
            auto params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          groupId = params["groupId"].get<LL>();
            LL          playerId = params["qq"].get<LL>();

            if (appid == APPID_BINGY_GAME) {
                if (bg_http_app_auth(appid, secret)) {
                    //if (preRegisterCallback())
                    //    postRegisterCallback();
                }
            }
        }
        catch (...) {
            bg_http_reply_error(req, 400, "无效的请求内容", BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };

    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(handler, req));
}