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
#include "utils.hpp"

using namespace nlohmann;

// 初始化 HTTP 请求信息结构体
inline http_req* get_http_req(mg_connection *connection, mg_http_message *ev_data) {
    http_req *req = static_cast<http_req *>(malloc(sizeof(http_req)));
    if (!req) {
        mg_http_reply(connection, 500, "Content-Type: application/json\r\n",
            "{ \"msg\": \"" BG_ERR_STR_MALLOC "\", \"errid\": " BG_ERR_STR_MALLOC_VAL "}");
        return nullptr;
    }

    // 复制 body 内容到临时内存
    char prevCh;

    if (ev_data->body.len > 0) {
        prevCh = ev_data->body.ptr[ev_data->body.len];
        const_cast<char *>(ev_data->body.ptr)[ev_data->body.len] = '\0';
        req->body = strdup(ev_data->body.ptr);
        const_cast<char *>(ev_data->body.ptr)[ev_data->body.len] = prevCh;
    }
    else {
        req->body = strdup("");
    }
    
    // 复制 query 内容到临时内存
    if (ev_data->query.len > 0) {
        prevCh = ev_data->query.ptr[ev_data->query.len];
        const_cast<char *>(ev_data->query.ptr)[ev_data->query.len] = '\0';
        req->query.ptr = strdup(ev_data->query.ptr);
        const_cast<char *>(ev_data->query.ptr)[ev_data->query.len] = prevCh;
        req->query.len = ev_data->query.len;
    }
    else {
        req->query.ptr = strdup("");
        req->query.len = 0;
    }

    // 把回应写入的位置记录下来
    req->res_body = &connection->bg_res_body;
    req->res_http_code = &connection->bg_res_http_code;
    req->signal = &connection->bg_res_ready;
    return req;
}

// 释放 HTTP 请求信息结构体所占用的内存
inline void free_http_req(http_req *req) {
    free(req->body);
    free(const_cast<char*>(req->query.ptr));
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

// 无附加参数的 POST 请求处理线程函数
inline std::function<void(void*)> make_bg_post_handler(
    const std::function<bool(const bgGameHttpReq&)> &preCallback,
    const std::function<void(const bgGameHttpReq&)> &postCallback
) {
    return [=](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                if (preCallback(bgReq))
                    postCallback(bgReq);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (...) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
}

// 无附加参数的 GET 请求处理线程函数
inline std::function<void(void*)> make_bg_get_handler(
    const std::function<bool(const bgGameHttpReq&)> &preCallback,
    const std::function<void(const bgGameHttpReq&)> &postCallback
) {
    return [=](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            std::string appid = get_query_param(&req->query, "appid");
            std::string secret = get_query_param(&req->query, "secret");
            LL          playerId = str_to_ll(get_query_param(&req->query, "qq"));
            LL          groupId = str_to_ll(get_query_param(&req->query, "groupId"));

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                if (preCallback(bgReq))
                    postCallback(bgReq);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (...) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
}

// 有一个附加参数的 POST 请求处理线程函数
template <typename ParamType>
std::function<void(void *)> make_bg_post_handler_param(
    const std::function<bool(const bgGameHttpReq&, const ParamType&)> &preCallback,
    const std::function<void(const bgGameHttpReq&, const ParamType&)> &postCallback,
    const char *jsonFieldName
) {
    return [=](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            ParamType   param = params[jsonFieldName].get<ParamType>();

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                if (preCallback(bgReq, param))
                    postCallback(bgReq, param);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (...) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
}

// 新玩家注册
CMD(register) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler(preRegisterCallback, postRegisterCallback), req));
}

// 查看硬币
CMD(view_coins) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_get_handler(preViewCoinsCallback, postViewCoinsCallback), req));
}

// 签到
CMD(sign_in) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler(preSignInCallback, postSignInCallback), req));
}

// 查看背包
CMD(view_inventory) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_get_handler(preViewInventoryCallback, postViewInventoryCallback), req));
}

// 出售
CMD(pawn) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(
        make_bg_post_handler_param<std::vector<LL>>(prePawnCallback, postPawnCallback, "items"), req)
    );
}

// 查看属性
CMD(view_properties) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_get_handler(preViewPropertiesCallback, postViewPropertiesCallback), req));
}

// 查看装备
CMD(view_equipments) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_get_handler(preViewEquipmentsCallback, postViewEquipmentsCallback), req));
}
