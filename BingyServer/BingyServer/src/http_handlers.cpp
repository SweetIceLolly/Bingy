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
    // 从 connection 所对应的共享内存获取消息. 如果检测到 HTTP 响应信息, 则发送给客户端
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
            auto        params = json::parse(req->body);
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
            auto        params = json::parse(req->body);
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
        catch (const std::exception &e) {
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
    threadPool.addJob(thread_pool_job( make_bg_post_handler_param<std::vector<LL>>(prePawnCallback, postPawnCallback, "items"), req));
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

// 装备
CMD(equip) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler_param<LL>(preEquipCallback, postEquipCallback, "item"), req));
}

// 卸下装备
CMD(unequip) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;

    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto        params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            EqiType     type = params["type"].get<EqiType>();
            LL          index;

            if (type == EqiType::single_use)
                index = params["index"].get<LL>();
            else
                index = -1;

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                if (type == EqiType::single_use) {                      // 卸下一次性装备
                    if (preUnequipSingleCallback(bgReq, index))
                        postUnequipSingleCallback(bgReq, index);
                }
                else {                                                  // 卸下普通装备
                    if (preUnequipCallback(bgReq, type))
                        postUnequipCallback(bgReq, type);
                }
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (const std::exception &e) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
    threadPool.addJob(thread_pool_job(handler, req));
}

// 卸下武器
CMD(unequip_weapon) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler(preUnequipWeaponCallback, postUnequipWeaponCallback), req));
}

// 卸下护甲
CMD(unequip_armor) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler(preUnequipArmorCallback, postUnequipArmorCallback), req));
}

// 卸下饰品
CMD(unequip_ornament) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler(preUnequipOrnamentCallback, postUnequipOrnamentCallback), req));
}

// 卸下全部装备
CMD(unequip_all) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler(preUnequipAllCallback, postUnequipAllCallback), req));
}

// 强化装备
CMD(upgrade) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;

    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto        params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            EqiType     type = params["type"].get<EqiType>();
            LL          times = params["times"].get<LL>();
            
            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                LL coinsNeeded;                                         // 强化需要硬币数

                if (preUpgradeCallback(bgReq, type, times, coinsNeeded))
                    postUpgradeCallback(bgReq, type, times, coinsNeeded);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (const std::exception &e) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
    threadPool.addDetachedJob(thread_pool_job(handler, req));
}

// 确认强化
CMD(confirm_upgrade) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler(preConfirmUpgradeCallback, postConfirmUpgradeCallback), req));
}

// 升级祝福
CMD(upgrade_blessing) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;

    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto        params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            LL          times = params["times"].get<LL>();

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                LL coinsNeeded;                                         // 强化需要硬币数

                if (preUpgradeBlessingCallback(bgReq, times, coinsNeeded))
                    postUpgradeBlessingCallback(bgReq, times, coinsNeeded);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (const std::exception &e) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
    threadPool.addDetachedJob(thread_pool_job(handler, req));
}

// 查看交易场
CMD(view_trade) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_get_handler(preViewTradeCallback, postViewTradeCallback), req));
}

// 购买交易场商品
CMD(buy_trade) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;

    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto        params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            LL          tradeId = params["tradeId"].get<LL>();
            std::string password = params["password"].get<std::string>();

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                if (preBuyTradeCallback(bgReq, tradeId, password))
                    postBuyTradeCallback(bgReq, tradeId);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (const std::exception &e) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
    threadPool.addJob(thread_pool_job(handler, req));
}

// 上架交易场商品
CMD(sell_trade) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;

    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto        params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            LL          invId = params["invId"].get<LL>();
            LL          price = params["price"].get<LL>();
            bool        hasPassword = params["hasPassword"].get<bool>();

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                if (preSellTradeCallback(bgReq, invId, price, hasPassword))
                    postSellTradeCallback(bgReq, invId, price, hasPassword);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (const std::exception &e) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
    threadPool.addJob(thread_pool_job(handler, req));
}

// 下架交易场商品
CMD(recall_trade) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;
    threadPool.addJob(thread_pool_job(make_bg_post_handler_param<LL>(preRecallTradeCallback, postRecallTradeCallback, "tradeId"), req));
}

// 合成装备
CMD(synthesis) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;

    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto        params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            std::set<LL, std::greater<LL>> invList = params["invList"].get<std::set<LL, std::greater<LL>>>();
            std::string target = params["target"].get<std::string>();
            LL          coins = 0;
            LL          level = 0;
            LL          targetId = -1;

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                if (preSynthesisCallback(bgReq, invList, target, targetId, coins, level))
                    postSynthesisCallback(bgReq, invList, targetId, coins, level);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (const std::exception &e) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
    threadPool.addJob(thread_pool_job(handler, req));
}

// 挑战副本
CMD(fight) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;

    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto        params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            std::string level = params["level"].get<std::string>();

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                LL levelId;

                if (preFightCallback(bgReq, level, levelId))
                    postFightCallback(bgReq, levelId);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (const std::exception &e) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
    threadPool.addJob(thread_pool_job(handler, req));
}

// 管理命令: 为玩家修改属性值
CMD(admin_modify_field) {
    auto req = get_http_req(connection, ev_data);
    if (!req)
        return;

    auto handler = [](void *_req) {
        http_req *req = static_cast<http_req *>(_req);

        try {
            auto        params = json::parse(req->body);
            std::string appid = params["appid"].get<std::string>();
            std::string secret = params["secret"].get<std::string>();
            LL          playerId = params["qq"].get<LL>();
            LL          groupId = params["groupId"].get<LL>();
            auto        fieldType = params["type"].get<unsigned char>();
            auto        mode = params["mode"].get<unsigned char>();
            LL          targetId = params["targetId"].get<LL>();
            LL          val = params["val"].get<LL>();

            if (appid == APPID_BINGY_GAME && bg_http_app_auth(appid, secret)) {
                bgGameHttpReq bgReq = { req, playerId, groupId };
                if (preAdminModifyFieldCallback(bgReq, fieldType, mode, targetId, val))
                    postAdminModifyFieldCallback(bgReq, fieldType, mode, targetId, val);
            }
            else
                bg_http_reply_error(req, 400, "", BG_ERR_AUTH_FAILED);
        }
        catch (const std::exception &e) {
            bg_http_reply_error(req, 400, BG_ERR_STR_INVALID_REQUEST, BG_ERR_INVALID_REQUEST);
        }

        free_http_req(req);
    };
    threadPool.addJob(thread_pool_job(handler, req));
}
