/*
描述: Bingy 游戏相关的函数. 整个游戏的主要逻辑
作者: 冰棍
文件: game.cpp
*/

#include "game.hpp"
#include "player.hpp"
#include "signin_event.hpp"
#include "error_codes.hpp"
#include "json.hpp"
#include "utils.hpp"

using namespace nlohmann;

// 懒人宏
// 获取当前对应的玩家
#define PLAYER allPlayers.at(bgReq.playerId)

std::unordered_set<LL>  allAdmins;
std::unordered_set<LL>  blacklist;

// 通用账号检查
bool accountCheck(const bgGameHttpReq &bgReq) {
    // 检查玩家是否已经注册
    if (!bg_player_exist(bgReq.playerId)) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_UNREGISTERED, BG_ERR_UNREGISTERED);
        return false;
    }

    // 检查玩家是否在小黑屋
    if (blacklist.find(bgReq.playerId) != blacklist.end()) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_BLACKLISTED, BG_ERR_BLACKLISTED);
        return false;
    }

    return true;
}

// 注册前检查
bool preRegisterCallback(const bgGameHttpReq &bgReq) {
    if (bg_player_exist(bgReq.playerId)) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ALREADY_REGISTERED, BG_ERR_ALREADY_REGISTERED);
        return false;
    }
    return true;
}

// 注册
void postRegisterCallback(const bgGameHttpReq &bgReq) {
    try {
        if (bg_player_add(bgReq.playerId))
            bg_http_reply(bgReq.req, 200, "");
        else
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_ADDING_NEW_PLAYER_FAILED, BG_ERR_ADDING_NEW_PLAYER_FAILED);
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_ADDING_NEW_PLAYER_FAILED + std::string(": ") + e.what(), BG_ERR_ADDING_NEW_PLAYER_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_ADDING_NEW_PLAYER_FAILED, BG_ERR_ADDING_NEW_PLAYER_FAILED);
    }
}

// 查看硬币前检查
bool preViewCoinsCallback(const bgGameHttpReq &bgReq) {
    return accountCheck(bgReq);
}

// 查看硬币
void postViewCoinsCallback(const bgGameHttpReq &bgReq) {
    try {
        bg_http_reply(bgReq.req, 200, 
            ("{coins: " + std::to_string(PLAYER.get_coins()) + "}").c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_GET_COINS_FAILED + std::string(": ") + e.what(), BG_ERR_GET_COINS_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_GET_COINS_FAILED, BG_ERR_GET_COINS_FAILED);
    }
}

// 签到前检查
bool preSignInCallback(const bgGameHttpReq &bgReq) {
    if (!accountCheck(bgReq))
        return false;

    // 如果玩家上次签到日期跟今天一样则拒绝签到
    dateTime signInDate = dateTime(PLAYER.get_lastSignIn());
    dateTime today = dateTime();

    if (signInDate.get_year() == today.get_year() && signInDate.get_month() == today.get_month() && signInDate.get_day() == today.get_day()) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ALREADY_SIGNED_IN, BG_ERR_ALREADY_SIGNED_IN);
        return false;
    }
    return true;
}

// 签到
void postSignInCallback(const bgGameHttpReq &bgReq) {
    try {
        dateTime now;

        // 检查连续签到
        if (is_day_sequential(dateTime(PLAYER.get_lastSignIn()), now)) {
            if (!PLAYER.inc_signInCountCont(1)) {
                bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SIGN_IN_SET_CONT_FAILED, BG_ERR_SIGN_IN_SET_CONT_FAILED);
                return;
            }
        }
        else {
            if (!PLAYER.set_signInCountCont(1)) {
                bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SIGN_IN_SET_CONT_FAILED, BG_ERR_SIGN_IN_SET_CONT_FAILED);
                return;
            }
        }

        // 设置签到时间
        if (!PLAYER.set_lastSignIn(dateTime().get_timestamp())) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SIGN_IN_SET_TIME_FAILED, BG_ERR_SIGN_IN_SET_TIME_FAILED);
            return;
        }

        // 设置玩家签到次数
        if (!PLAYER.inc_signInCount(1)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SIGN_IN_INC_COUNT_FAILED, BG_ERR_SIGN_IN_INC_COUNT_FAILED);
            return;
        }

        // 签到获得硬币 = 500 + 25 * 连续签到天数 + 10 * 总签到天数 + rnd(20 * 总签到天数)
        // 签到*之后的*体力 = 15 * 连续签到天数 + 5 * 总签到天数
        // 签到获得经验 = 15 * 连续签到次数 + 5 * 总签到天数
        LL deltaCoins = 500 + 25 * PLAYER.get_signInCountCont() + 10 * PLAYER.get_signInCount() + rndRange(20 * PLAYER.get_signInCount());
        LL deltaEnergy = 150 + 5 * PLAYER.get_level();
        LL deltaExp = 15 * PLAYER.get_signInCountCont() + 5 * PLAYER.get_signInCount();

        std::vector<std::pair<std::string, int>> errors;        // 发生的所有错误

        // 检查签到活动
        std::string eventMsg = "";                              // 活动消息
        std::vector<LL> eventItems;                             // 活动赠送物品
        bg_match_sign_in_event(now, deltaCoins, deltaEnergy, eventItems, eventMsg);
        for (const auto &item : eventItems) {       // 为玩家添加物品
            inventoryData itemData;
            itemData.id = item;
            itemData.level = 0;
            itemData.wear = allEquipments.at(item).wear;
            if (!PLAYER.add_inventory_item(itemData)) {
                errors.push_back({
                    BG_ERR_STR_SIGN_IN_ADD_ITEM_FAILED + std::string(": ") + allEquipments.at(item).name,
                    BG_ERR_SIGN_IN_ADD_ITEM_FAILED
                });
            }
        }

        if (!PLAYER.inc_coins(deltaCoins)) {
            errors.push_back({ BG_ERR_STR_SIGN_IN_INC_COINS_FAILED , BG_ERR_SIGN_IN_INC_COINS_FAILED });
        }
        if (!PLAYER.set_energy(static_cast<LL>(PLAYER.get_energy() * 0.75) + deltaEnergy)) {
            errors.push_back({ BG_ERR_STR_SIGN_IN_INC_ENERGY_FAILED , BG_ERR_SIGN_IN_INC_ENERGY_FAILED });
        }
        if (!PLAYER.inc_exp(deltaExp)) {
            errors.push_back({ BG_ERR_STR_SIGN_IN_INC_EXP_FAILED , BG_ERR_SIGN_IN_INC_EXP_FAILED });
        }

        json reply = {
            { "errors", errors },
            { "signInCountCont", PLAYER.get_signInCountCont() },
            { "signInCount", PLAYER.get_signInCount() },
            { "deltaCoins", deltaCoins },
            { "coins", PLAYER.get_coins() },
            { "deltaEnergy", deltaEnergy },
            { "energy", PLAYER.get_energy() },
            { "deltaExp", deltaExp },
            { "eventMsg", eventMsg }
        };
        bg_http_reply(bgReq.req, 200, reply.dump().c_str());
    }

    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SIGN_IN_INC_COUNT_FAILED + std::string(": ") + e.what(), BG_ERR_SIGN_IN_INC_COUNT_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SIGN_IN_INC_COUNT_FAILED, BG_ERR_SIGN_IN_INC_COUNT_FAILED);
    }
}

