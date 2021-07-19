/*
描述: Bingy 游戏相关的函数. 整个游戏的主要逻辑
作者: 冰棍
文件: game.cpp
*/

#include <sstream>
#include <iomanip>
#include "game.hpp"
#include "player.hpp"
#include "signin_event.hpp"
#include "equipment.hpp"
#include "error_codes.hpp"
#include "json.hpp"
#include "utils.hpp"

using namespace nlohmann;

// 懒人宏
// 获取当前对应的玩家 (非线程安全, 请确保调用该宏时玩家列表没有被修改)
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
        LOCK_PLAYERS_LIST;
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
    LOCK_PLAYERS_LIST;
    dateTime signInDate = dateTime(PLAYER.get_lastSignIn());
    UNLOCK_PLAYERS_LIST;
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
        LOCK_PLAYERS_LIST;

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

// 查看背包前检查
bool preViewInventoryCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 通用查看背包函数
std::vector<std::string> getInventoryArr(const LL &id) {
    LOCK_PLAYERS_LIST;
    auto tmp = allPlayers.at(id).get_inventory();
    UNLOCK_PLAYERS_LIST;

    std::vector<std::string> inventory;
    for (const auto &item : tmp) {
        auto &eqiEntry = allEquipments.at(item.id);
        if (eqiEntry.type != EqiType::single_use)                       // 非一次性物品
            inventory.push_back(eqiEntry.name + "+" + std::to_string(item.level));
        else                                                            // 一次性物品
            inventory.push_back("[" + eqiEntry.name + "]");
    }
    
    return inventory;
}

// 查看背包
void postViewInventoryCallback(const bgGameHttpReq& bgReq) {
    try {
        LOCK_PLAYERS_LIST;
        auto capacity = PLAYER.get_invCapacity();
        UNLOCK_PLAYERS_LIST;

        json reply = {
            { "items", getInventoryArr(bgReq.playerId) },
            { "capacity", capacity }
        };
        bg_http_reply(bgReq.req, 200, reply.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_VIEW_INV_FAILED + std::string(": ") + e.what(), BG_ERR_VIEW_INV_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_VIEW_INV_FAILED, BG_ERR_VIEW_INV_FAILED);
    }
}

// 出售前检查
bool prePawnCallback(const bgGameHttpReq& bgReq, const std::vector<LL>& items) {
    if (!accountCheck(bgReq))
        return false;

    try {
        std::unordered_set<LL> sellList;
        LOCK_PLAYERS_LIST;
        for (const auto &item : items) {
            if (item > PLAYER.get_inventory_size() || item < 1) {               // 检查是否超出背包范围
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ID_OUT_OF_RANGE + std::string(": ") + std::to_string(item), BG_ERR_ID_OUT_OF_RANGE);
                return false;
            }
            if (sellList.find(item) == sellList.end()) {                        // 检查是否有重复项目
                sellList.insert(item);                                              // 记录该项目
            }
            else {
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ID_REPEATED + std::string(": ") + std::to_string(item), BG_ERR_ID_REPEATED);
                return false;
            }
        }
        return true;
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_PAWN_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_PAWN_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_PAWN_FAILED, BG_ERR_PRE_PAWN_FAILED);
        return false;
    }
}

// 出售
void postPawnCallback(const bgGameHttpReq& bgReq, const std::vector<LL>& items) {
    try {
        std::vector<LL> pawnItems = items;
        std::sort(pawnItems.rbegin(), pawnItems.rend());    // 从大到小排序序号. 从后面往前删才不会出错

        LOCK_PLAYERS_LIST;
        double  price = 0;
        auto    inv = PLAYER.get_inventory();
        LL      prevIndex = static_cast<LL>(inv.size()) - 1;
        auto    it = inv.rbegin();
        for (const auto &index : pawnItems) {               // 计算出售总价
            std::advance(it, prevIndex - index);
            if (allEquipments.at(it->id).type == EqiType::single_use)   // 一次性装备价格 = 原始价格
                price += allEquipments.at(it->id).price;
            else                                                        // 装备价格 = 原始价格 + 100 * (1.6 ^ 装备等级 - 1) / 0.6
                price += allEquipments.at(it->id).price + 100.0 * (pow(1.6, it->level) - 1) / 0.6;
            prevIndex = index;
        }

        if (!PLAYER.remove_at_inventory(pawnItems)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_REMOVE_ITEM_FAILED, BG_ERR_REMOVE_ITEM_FAILED);
            return;
        }
        if (!PLAYER.inc_coins(static_cast<LL>(price))) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SIGN_IN_INC_COINS_FAILED, BG_ERR_SIGN_IN_INC_COINS_FAILED);
            return;
        }

        json reply = {
            { "count", pawnItems.size() },
            { "coins", static_cast<LL>(price) }
        };
        bg_http_reply(bgReq.req, 200, reply.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_PAWN_FAILED + std::string(e.what()), BG_ERR_PAWN_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_PAWN_FAILED, BG_ERR_PAWN_FAILED);
    }
}

// 查看属性前检查
bool preViewPropertiesCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 懒人宏
// 获取某个属性的 JSON 字段
#define GET_EQI_PROP_STR(prop)                                                                          \
    {                                                                                                   \
        #prop, {                                                                                        \
            { "total",                                                 /* 最终值 */                      \
                allPlayers.at(id).get_ ##prop()                                                         \
            },                                                                                          \
            { "weapons",                                               /* 武器属性总和 */                \
                allPlayers.at(id).get_equipments().at(EqiType::weapon_primary).calc_ ##prop() +         \
                allPlayers.at(id).get_equipments().at(EqiType::weapon_secondary).calc_ ##prop()         \
            },                                                                                          \
            { "armors",                                                /* 护甲属性总和 */                \
                allPlayers.at(id).get_equipments().at(EqiType::armor_helmet).calc_ ##prop() +           \
                allPlayers.at(id).get_equipments().at(EqiType::armor_body).calc_ ##prop () +            \
                allPlayers.at(id).get_equipments().at(EqiType::armor_leg).calc_ ##prop () +             \
                allPlayers.at(id).get_equipments().at(EqiType::armor_boot).calc_ ##prop ()              \
            },                                                                                          \
            { "ornaments",                                             /* 饰品属性总和 */                \
                allPlayers.at(id).get_equipments().at(EqiType::ornament_earrings).calc_ ##prop() +      \
                allPlayers.at(id).get_equipments().at(EqiType::ornament_rings).calc_ ##prop () +        \
                allPlayers.at(id).get_equipments().at(EqiType::ornament_necklace).calc_ ##prop () +     \
                allPlayers.at(id).get_equipments().at(EqiType::ornament_jewelry).calc_ ##prop ()        \
            }                                                                                           \
        }                                                                                               \
    }

// 通用查看属性函数
std::string getPropertiesStr(const LL &id) {
    LOCK_PLAYERS_LIST;

    return json {
        { "qq", id },
        { "level", allPlayers.at(id).get_level() },
        { "blessing", allPlayers.at(id).get_blessing() },
        { "exp", allPlayers.at(id).get_exp() },
        { "expNeeded", allPlayers.at(id).get_exp_needed() },
        { GET_EQI_PROP_STR(hp) },
        { GET_EQI_PROP_STR(atk) },
        { GET_EQI_PROP_STR(def) },
        { GET_EQI_PROP_STR(mp) },
        { GET_EQI_PROP_STR(crt) },
        { GET_EQI_PROP_STR(brk) },
        { GET_EQI_PROP_STR(agi) },
        { "energy", allPlayers.at(id).get_energy() },
        { "coins", allPlayers.at(id).get_coins() },
        { "heroCoin", allPlayers.at(id).get_heroCoin() }
    }.dump();
}

// 查看属性
void postViewPropertiesCallback(const bgGameHttpReq& bgReq) {
    try {
        bg_http_reply(bgReq.req, 200, getPropertiesStr(bgReq.playerId).c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_VIEW_PROP_FAILED + std::string(": ") + e.what(), BG_ERR_VIEW_PROP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_VIEW_PROP_FAILED, BG_ERR_VIEW_PROP_FAILED);
    }
}

// 查看装备前检查
bool preViewEquipmentsCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 懒人宏
// 生成对应类型的装备的 JSON 字段. 若 ID = -1, 则显示"未装备"; 否则显示 装备名称+等级 (磨损/原始磨损)
#define GET_EQI_STR(eqitype)                                                                                       \
    {                                                                                                              \
        #eqitype,                                                                                                  \
        (eqitype.id == -1 ? "未装备" : allEquipments.at(eqitype.id).name + "+" + std::to_string(eqitype.level) +   \
            " (" + std::to_string(eqitype.wear) + "/" + std::to_string(allEquipments.at(eqitype.id).wear) + ")")   \
    }

// 通用查看装备函数
std::string getEquipmentsStr(const LL &id) {
    LOCK_PLAYERS_LIST;
    const auto eqi = allPlayers.at(id).get_equipments();
    const auto singleUseEqi = allPlayers.at(id).get_equipItems();
    UNLOCK_PLAYERS_LIST;

    const auto &armor_helmet = eqi.at(EqiType::armor_helmet);
    const auto &armor_body = eqi.at(EqiType::armor_body);
    const auto &armor_leg = eqi.at(EqiType::armor_leg);
    const auto &armor_boot = eqi.at(EqiType::armor_boot);
    const auto &weapon_primary = eqi.at(EqiType::weapon_primary);
    const auto &weapon_secondary = eqi.at(EqiType::weapon_secondary);
    const auto &ornament_earrings = eqi.at(EqiType::ornament_earrings);
    const auto &ornament_rings = eqi.at(EqiType::ornament_rings);
    const auto &ornament_necklace = eqi.at(EqiType::ornament_necklace);
    const auto &ornament_jewelry = eqi.at(EqiType::ornament_jewelry);
    
    std::vector<std::string> singleUseItems;
    for (const auto &item : singleUseEqi) {
        singleUseItems.push_back(allEquipments.at(item.id).name);
    }

    return json{
        GET_EQI_STR(armor_helmet),
        GET_EQI_STR(armor_body),
        GET_EQI_STR(armor_leg),
        GET_EQI_STR(armor_boot),
        GET_EQI_STR(weapon_primary),
        GET_EQI_STR(weapon_secondary),
        GET_EQI_STR(ornament_earrings),
        GET_EQI_STR(ornament_rings),
        GET_EQI_STR(ornament_necklace),
        GET_EQI_STR(ornament_jewelry),
        { "single_use", singleUseItems }
    }.dump();
}

//查看装备
void postViewEquipmentsCallback(const bgGameHttpReq& bgReq) {
    try {
        bg_http_reply(bgReq.req, 200, getEquipmentsStr(bgReq.playerId).c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_VIEW_EQI_FAILED + std::string(": ") + e.what(), BG_ERR_VIEW_PROP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_VIEW_EQI_FAILED, BG_ERR_VIEW_EQI_FAILED);
    }
}
