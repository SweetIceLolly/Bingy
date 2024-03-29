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
#include "trade.hpp"
#include "synthesis.hpp"
#include "monster.hpp"
#include "error_codes.hpp"
#include "json.hpp"
#include "utils.hpp"
#include "fight.hpp"
#include "secrets.hpp"

using namespace nlohmann;

// 懒人宏
// 线程安全地获取当前对应的玩家
#define PLAYER bg_player_get(bgReq.playerId)

// ========================================================
// 一些游戏的开关

// 游戏是否暂停
bool gamePaused = false;

// 交易场是否禁用
bool marketDisabled = false;

// 冷却时间重载
LL cdOverride = 0;

// ========================================================

// 通用账号检查
bool accountCheck(const bgGameHttpReq &bgReq) {
    // 检查游戏是否暂停
    if (gamePaused) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_GAME_PAUSED, BG_ERR_GAME_PAUSED);
        return false;
    }

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
    // 检查游戏是否暂停
    if (gamePaused) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_GAME_PAUSED, BG_ERR_GAME_PAUSED);
        return false;
    }

    // 检查玩家是否已经注册过了
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
            bg_http_reply(bgReq.req, 200, "{}");
        else
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
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
            ("{\"coins\": " + std::to_string(PLAYER.get_coins()) + "}").c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
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
        for (const auto &item : eventItems) {                   // 为玩家添加物品
            inventoryData itemData;
            itemData.id = item;
            itemData.level = 0;
            itemData.wear = allEquipments.at(item).wear;
            if (!PLAYER.add_inventory_item(itemData)) {
                errors.push_back({ BG_ERR_STR_ADD_ITEM_FAILED + std::string(": ") + allEquipments.at(item).name, BG_ERR_ADD_ITEM_FAILED });
            }
        }

        if (!PLAYER.inc_coins(deltaCoins)) {
            errors.push_back({ BG_ERR_STR_INC_COINS_FAILED , BG_ERR_INC_COINS_FAILED });
        }
        if (!PLAYER.set_energy(static_cast<LL>(PLAYER.get_energy() * 0.75) + deltaEnergy)) {
            errors.push_back({ BG_ERR_STR_INC_ENERGY_FAILED , BG_ERR_INC_ENERGY_FAILED });
        }
        if (!PLAYER.inc_exp(deltaExp)) {
            errors.push_back({ BG_ERR_STR_INC_EXP_FAILED , BG_ERR_INC_EXP_FAILED });
        }
        if (!PLAYER.check_exp_upgrade()) {
            errors.push_back({ BG_ERR_STR_INC_EXP_FAILED , BG_ERR_INC_EXP_FAILED });
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
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 查看背包前检查
bool preViewInventoryCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 通用查看背包函数
std::vector<std::pair<std::string, unsigned char>> getInventoryArr(LL id) {
    auto tmp = bg_player_get(id).get_inventory();

    std::vector<std::pair<std::string, unsigned char>> inventory;
    for (const auto &item : tmp) {
        auto &eqiEntry = allEquipments.at(item.id);
        if (eqiEntry.type != EqiType::single_use)                       // 非一次性物品
            inventory.push_back({ eqiEntry.name + "+" + std::to_string(item.level), static_cast<unsigned char>(eqiEntry.type) });
        else                                                            // 一次性物品
            inventory.push_back({ "[" + eqiEntry.name + "]", static_cast<unsigned char>(eqiEntry.type) });
    }
    
    return inventory;
}

// 查看背包
void postViewInventoryCallback(const bgGameHttpReq& bgReq) {
    try {
        auto capacity = PLAYER.get_invCapacity();

        json reply = {
            { "items", getInventoryArr(bgReq.playerId) },
            { "capacity", capacity }
        };
        bg_http_reply(bgReq.req, 200, reply.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 出售前检查
bool prePawnCallback(const bgGameHttpReq& bgReq, const std::vector<LL>& items) {
    if (!accountCheck(bgReq))
        return false;

    try {
        std::unordered_set<LL> sellList;
        for (const auto &item : items) {
            if (item >= PLAYER.get_inventory_size() || item < 0) {              // 检查是否超出背包范围
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ID_OUT_OF_RANGE + std::string(": ") + std::to_string(item + 1), BG_ERR_ID_OUT_OF_RANGE);
                return false;
            }
            if (sellList.find(item) == sellList.end()) {                        // 检查是否有重复项目
                sellList.insert(item);                                              // 记录该项目
            }
            else {
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ID_REPEATED + std::string(": ") + std::to_string(item + 1), BG_ERR_ID_REPEATED);
                return false;
            }
        }
        return true;
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
}

// 出售
void postPawnCallback(const bgGameHttpReq& bgReq, const std::vector<LL>& items) {
    try {
        std::vector<LL> pawnItems = items;
        std::sort(pawnItems.rbegin(), pawnItems.rend());    // 从大到小排序序号. 从后面往前删才不会出错

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
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_INC_COINS_FAILED, BG_ERR_INC_COINS_FAILED);
            return;
        }

        json reply = {
            { "count", pawnItems.size() },
            { "coins", static_cast<LL>(price) }
        };
        bg_http_reply(bgReq.req, 200, reply.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(e.what()), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 查看属性前检查
bool preViewPropertiesCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 懒人宏
// 获取某个属性的 JSON 字段
#define GET_EQI_PROP_STR(prop)                                                  \
    {                                                                           \
        #prop, {                                                                \
            { "total",                                  /* 最终值 */            \
                bg_player_get(id).get_ ##prop()                                 \
            },                                                                  \
            { "weapons",                                /* 武器属性总和 */       \
                equipments.at(EqiType::weapon_primary).calc_ ##prop() +         \
                equipments.at(EqiType::weapon_secondary).calc_ ##prop()         \
            },                                                                  \
            { "armors",                                 /* 护甲属性总和 */       \
                equipments.at(EqiType::armor_helmet).calc_ ##prop() +           \
                equipments.at(EqiType::armor_body).calc_ ##prop () +            \
                equipments.at(EqiType::armor_leg).calc_ ##prop () +             \
                equipments.at(EqiType::armor_boot).calc_ ##prop ()              \
            },                                                                  \
            { "ornaments",                              /* 饰品属性总和 */       \
                equipments.at(EqiType::ornament_earrings).calc_ ##prop() +      \
                equipments.at(EqiType::ornament_rings).calc_ ##prop () +        \
                equipments.at(EqiType::ornament_necklace).calc_ ##prop () +     \
                equipments.at(EqiType::ornament_jewelry).calc_ ##prop ()        \
            }                                                                   \
        }                                                                       \
    }

// 通用查看属性函数
std::string getPropertiesStr(LL id) {
    auto equipments = bg_player_get(id).get_equipments();

    return json {
        { "qq", id },
        { "level", bg_player_get(id).get_level() },
        { "blessing", bg_player_get(id).get_blessing() },
        { "exp", bg_player_get(id).get_exp() },
        { "expNeeded", bg_player_get(id).get_exp_needed() },
        GET_EQI_PROP_STR(hp),
        GET_EQI_PROP_STR(atk),
        GET_EQI_PROP_STR(def),
        GET_EQI_PROP_STR(mp),
        GET_EQI_PROP_STR(crt),
        GET_EQI_PROP_STR(brk),
        GET_EQI_PROP_STR(agi),
        { "energy", bg_player_get(id).get_energy() },
        { "coins", bg_player_get(id).get_coins() },
        { "heroCoin", bg_player_get(id).get_heroCoin() }
    }.dump();
}

// 查看属性
void postViewPropertiesCallback(const bgGameHttpReq& bgReq) {
    try {
        bg_http_reply(bgReq.req, 200, getPropertiesStr(bgReq.playerId).c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
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
std::string getEquipmentsStr(LL id) {
    const auto eqi = bg_player_get(id).get_equipments();
    const auto singleUseEqi = bg_player_get(id).get_equipItems();

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

    return json {
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
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 查找装备前检查
bool preSearchEquipmentsCallback(const bgGameHttpReq &bgReq, const std::string &keyword) {
    if (!accountCheck(bgReq))
        return false;

    // 检查关键字是否过长
    if (keyword.length() > 20) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_KEYWORD_TOO_LONG, BG_ERR_KEYWORD_TOO_LONG);
        return false;
    }
    return true;
}

// 查找装备
void postSearchEquipmentsCallback(const bgGameHttpReq &bgReq, const std::string &keyword) {
    try {
        bg_http_reply(bgReq.req, 200, json{ {"eqis", bg_search_equipment(keyword)} }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 查看 VIP 信息前检查
bool preViewVipCallback(const bgGameHttpReq &bgReq) {
    return accountCheck(bgReq);
}

// 查看 VIP 信息
void postViewVipCallback(const bgGameHttpReq &bgReq) {
    try {
        bg_http_reply(bgReq.req, 200, json {
            { "level", PLAYER.get_vip() },
            { "count", bg_player_get_count_vip() }
        }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 装备前检查
bool preEquipCallback(const bgGameHttpReq& bgReq, LL equipItem) {
    if (!accountCheck(bgReq))
        return false;

    try {
        if (equipItem >= PLAYER.get_inventory_size() || equipItem < 0) {        // 检查是否超出背包范围
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ID_OUT_OF_RANGE + std::string(": ") + std::to_string(equipItem + 1), BG_ERR_ID_OUT_OF_RANGE);
            return false;
        }

        auto invItem = PLAYER.get_inventory_item(equipItem);
        if (allEquipments[invItem.id].type == EqiType::single_use) {
            // 最多只能装备 5 个一次性物品
            if (PLAYER.get_equipItems_size() >= 5) {
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_MAX_ITEMS, BG_ERR_MAX_ITEMS);
                return false;
            }
            // 不允许装备重复的一次性物品
            for (const auto &item : PLAYER.get_equipItems()) {
                if (item.id == invItem.id) {
                    bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ITEM_REPEATED, BG_ERR_ITEM_REPEATED);
                    return false;
                }
            }
        }
        return true;
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
}

// 装备
void postEquipCallback(const bgGameHttpReq& bgReq, LL equipItem) {
    PLAYER.resetCache();                                            // 重算玩家属性

    // 获取背包里该序号的对应物品
    auto item = PLAYER.get_inventory_item(equipItem);

    // 把装备上去了的装备从背包移除
    if (!PLAYER.remove_at_inventory({ equipItem })) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_EQUIP_REMOVE_FAILED, BG_ERR_EQUIP_REMOVE_FAILED);
        return;
    }

    // 根据装备类型来装备
    auto eqiType = allEquipments.at(item.id).type;
    if (eqiType != EqiType::single_use) {                               // 不是一次性物品
        auto prevEquipItem = PLAYER.get_equipments_item(eqiType);       // 获取玩家之前装备的装备

        // 把新装备装备上去
        if (!PLAYER.set_equipments_item(eqiType, item)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_EQUIP_UPDATE_FAILED, BG_ERR_EQUIP_UPDATE_FAILED);
            return;
        }

        // 如果玩家之前的装备不为空, 就放回背包中
        if (prevEquipItem.id != -1) {
            if (!PLAYER.add_inventory_item(prevEquipItem)) {
                bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_EQUIP_ADD_FAILED, BG_ERR_EQUIP_ADD_FAILED);
                return;
            }
        }

        json reply = {
            { "type", eqiType_to_str(eqiType) },
            { "name", allEquipments.at(item.id).name },
            { "level", item.level },
            { "wear", item.wear },
            { "defWear", allEquipments.at(item.id).wear }
        };
        bg_http_reply(bgReq.req, 200, reply.dump().c_str());
    }
    else {                                                              // 是一次性物品
        // 把物品添加到一次性列表
        if (!PLAYER.add_equipItems_item(item)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_EQUIP_UPDATE_FAILED, BG_ERR_EQUIP_UPDATE_FAILED);
            return;
        }

        json reply = {
            { "type", eqiType_to_str(eqiType) },
            { "name", allEquipments.at(item.id).name }
        };
        bg_http_reply(bgReq.req, 200, reply.dump().c_str());
    }
}

// 把指定玩家的指定类型的装备卸下并放回包里.
// 如果卸下了装备, 则返回对应的名称; 否则返回空字符串
// 注意, 该函数不能处理卸下一次性物品
std::string unequipPlayer(LL qq, const EqiType &eqiType) {
    auto &p = bg_player_get(qq);
    inventoryData eqi = p.get_equipments().at(eqiType);

    if (eqi.id == -1)
        return "";
    else {
        p.resetCache();                                            // 重算玩家属性
        if (!p.set_equipments_item(eqiType, { -1, -1, -1 })) {
            throw std::runtime_error("设置玩家装备时失败!");
        }
        if (!p.add_inventory_item(eqi)) {
            throw std::runtime_error("为玩家添加装备到背包时失败!");
        }

        return allEquipments.at(eqi.id).name + "+" + std::to_string(eqi.level);
    }
}

// 卸下多个指定类型的装备并返回所有卸下了的装备的列表
template <typename ... Ts>
std::vector<std::string> UnequipMultiple(LL qq, const Ts&&... types) {
    auto items = std::vector<std::string> { unequipPlayer(qq, types)... };
    items.erase(std::remove_if(items.begin(), items.end(), [](const std::string &s) { return s.empty(); }), items.end());
    return items;
}

// 卸下指定玩家的一次性物品并放回包里
std::vector<std::string> UnequipAllSingles(LL qq) {
    auto items = bg_player_get(qq).get_equipItems();
    if (!bg_player_get(qq).clear_equipItems()) {
        throw std::runtime_error(BG_ERR_STR_CLEAR_SINGLE_FAILED);
    }

    std::vector<std::string> rtn;
    for (const auto &item : items) {
        if (bg_player_get(qq).add_inventory_item(item))
            rtn.push_back(allEquipments.at(item.id).name);
        else
            throw BG_ERR_STR_SINGLE_ADD_FAILED + std::string(": ") + allEquipments.at(item.id).name;
    }
    return rtn;
}

// 卸下指定类型的装备前检查
bool preUnequipCallback(const bgGameHttpReq& bgReq, const EqiType& type) {
    if (!accountCheck(bgReq))
        return false;
    if (PLAYER.get_equipments().at(type).id == -1) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_NOT_EQUIPPED + std::string(": ") +
            eqiType_to_str(type), BG_ERR_NOT_EQUIPPED);
        return false;
    }
    return true;
}

// 卸下指定类型的装备
void postUnequipCallback(const bgGameHttpReq& bgReq, const EqiType& type) {
    try {
        PLAYER.resetCache();                                            // 重算玩家属性
        auto rtn = unequipPlayer(bgReq.playerId, type);
        bg_http_reply(bgReq.req, 200, json { { "item", rtn } }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 卸下武器前检查
bool preUnequipWeaponCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 卸下所有武器
void postUnequipWeaponCallback(const bgGameHttpReq& bgReq) {
    try {
        PLAYER.resetCache();                                            // 重算玩家属性
        bg_http_reply(bgReq.req, 200, json {
            { "items", UnequipMultiple(
                bgReq.playerId,
                EqiType::weapon_primary,
                EqiType::weapon_secondary
                )
            }
        }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 卸下护甲前检查
bool preUnequipArmorCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 卸下所有护甲
void postUnequipArmorCallback(const bgGameHttpReq& bgReq) {
    try {
        PLAYER.resetCache();                                            // 重算玩家属性
        bg_http_reply(bgReq.req, 200, json {
            { "items", UnequipMultiple(
                bgReq.playerId,
                EqiType::armor_body,
                EqiType::armor_boot,
                EqiType::armor_helmet,
                EqiType::armor_leg
                )
            }
            }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 卸下饰品前检查
bool preUnequipOrnamentCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 卸下所有饰品
void postUnequipOrnamentCallback(const bgGameHttpReq& bgReq) {
    try {
        PLAYER.resetCache();                                            // 重算玩家属性
        bg_http_reply(bgReq.req, 200, json {
            { "items", UnequipMultiple(
                bgReq.playerId,
                EqiType::ornament_earrings,
                EqiType::ornament_jewelry,
                EqiType::ornament_necklace,
                EqiType::ornament_rings
                )
            }
            }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 卸下所有装备前检查
bool preUnequipAllCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 卸下所有装备
void postUnequipAllCallback(const bgGameHttpReq& bgReq) {
    try {
        PLAYER.resetCache();                                            // 重算玩家属性
        auto eqis = UnequipMultiple(
            bgReq.playerId,
            EqiType::weapon_primary,
            EqiType::weapon_secondary,
            EqiType::armor_body,
            EqiType::armor_boot,
            EqiType::armor_helmet,
            EqiType::armor_leg,
            EqiType::ornament_earrings,
            EqiType::ornament_jewelry,
            EqiType::ornament_necklace,
            EqiType::ornament_rings
        );
        auto singles = UnequipAllSingles(bgReq.playerId);
        eqis.insert(eqis.end(), singles.begin(), singles.end());
        bg_http_reply(bgReq.req, 200, json { { "items", eqis } }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 卸下指定的一次性装备前检查
bool preUnequipSingleCallback(const bgGameHttpReq& bgReq, LL index) {
    if (!accountCheck(bgReq))
        return false;

    try {
        if (index < 0 || index >= PLAYER.get_equipItems_size()) {               // 检查序号是否超出装备范围
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_SINGLE_OUT_OF_RANGE + std::string(": ") + std::to_string(index + 1), BG_ERR_SINGLE_OUT_OF_RANGE);
            return false;
        }
        return true;
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
}

// 卸下指定的一次性装备
void postUnequipSingleCallback(const bgGameHttpReq& bgReq, LL index) {
    try {
        PLAYER.resetCache();                    // 重算玩家属性

        // 先把要卸下的装备记录下来, 方便之后添加到背包
        auto items = PLAYER.get_equipItems();
        auto it = items.begin();
        std::advance(it, index);

        if (!PLAYER.remove_at_equipItems(index)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_REMOVE_SINGLE_FAILED, BG_ERR_REMOVE_SINGLE_FAILED);
            return;
        }
        if (!PLAYER.add_inventory_item(*it)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SINGLE_ADD_FAILED, BG_ERR_SINGLE_ADD_FAILED);
            return;
        }
        bg_http_reply(bgReq.req, 200, json { { "item", allEquipments.at(it->id).name} }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 强化装备前检查
bool preUpgradeCallback(const bgGameHttpReq& bgReq, const EqiType& type, LL upgradeTimes, LL& coinsNeeded) {
    if (!accountCheck(bgReq))
        return false;

    // 检查是否有装备对应类型的装备
    if (PLAYER.get_equipments().at(type).id == -1) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_NOT_EQUIPPED + std::string(": ") +
            eqiType_to_str(type), BG_ERR_NOT_EQUIPPED);
        return false;
    }

    // 获取升级次数
    try {
        if (upgradeTimes < 1) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INVALID_UPGRADE_TIMES, BG_ERR_INVALID_UPGRADE_TIMES);
            return false;
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
    if (upgradeTimes > 20) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_MAX_UPGRADE_TIMES, BG_ERR_MAX_UPGRADE_TIMES);
        return false;
    }

    // 计算升级所需硬币
    double currEqiLevel = static_cast<double>(PLAYER.get_equipments().at(type).level);
    if (upgradeTimes == 1) {
        // 装备升一级价格 = 210 * 1.61 ^ 当前装备等级
        coinsNeeded = static_cast<LL>(210.0 * pow(1.61, currEqiLevel));
        if (PLAYER.get_coins() < coinsNeeded) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INSUFFICIENT_COINS +
                std::string(": 还需要") + std::to_string(coinsNeeded - PLAYER.get_coins()) + "硬币",
                BG_ERR_INSUFFICIENT_COINS
            );
            return false;
        }
    }
    else {
        //   装备升 n 级价格
        // = sum(210 * 1.61 ^ x, x, 当前等级, 当前等级 + n - 1)
        // = 21000 * (1.61 ^ (当前等级 + n) - 1.61 ^ 当前等级) / 61
        // ≈ -344.262295082 * exp(0.476234178996 * 当前等级) + 344.262295082 * exp(0.476234178996 * (当前等级 + n))
        coinsNeeded = static_cast<LL>(-344.262295082 * exp(0.476234178996 * currEqiLevel) + 344.262295082 * exp(0.476234178996 * (currEqiLevel + upgradeTimes)));
        LL currCoins = PLAYER.get_coins();
        if (currCoins < coinsNeeded) {
            // 如果不够硬币, 则计算用户可以升级多少级, 即对下列方程中的 n 求解:
            // 21000 * (1.61 ^ (当前等级 + n) - 1.61 ^ 当前等级) / 61 = 当前硬币
            // 解得:
            // n = (当前等级 * ln(7) - 2 * 当前等级 * ln(10) + 当前等级 * ln(23) + ln(3) + ln(7) + 3 * ln(10) - ln(61 * 当前硬币 + 21000 * (161 / 100) ^ 当前等级)) / (-ln(7) + 2 * ln(10) - ln(23))
            //   ≈ -2.09980728831 * (-ln(21000.0 * exp(0.476234178996 * 当前等级) + 61.0 * 当前硬币) + 0.476234178996 * 当前等级 + 9.95227771671)
            // 再取 floor
            upgradeTimes = static_cast<LL>(floor(
                -2.09980728831 * (-log(21000.0 * exp(0.476234178996 * currEqiLevel) + 61.0 * currCoins) + 0.476234178996 * currEqiLevel + 9.95227771671)
            ));

            if (upgradeTimes < 1) {
                // 玩家的钱一次都升级不了
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INSUFFICIENT_COINS +
                    std::string(": 还需要") + std::to_string(coinsNeeded - currCoins) + "硬币",
                    BG_ERR_INSUFFICIENT_COINS
                );
            }
            else {
                // 重算需要硬币
                coinsNeeded = static_cast<LL>(-344.262295082 * exp(0.476234178996 * currEqiLevel) + 344.262295082 * exp(0.476234178996 * (currEqiLevel + upgradeTimes)));
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INSUFFICIENT_COINS +
                    std::string(": 当前硬币只够升级") + std::to_string(upgradeTimes) + "次, 强化完之后还剩" +
                    std::to_string(currCoins - coinsNeeded) + "硬币",
                    BG_ERR_INSUFFICIENT_COINS
                );
            }
            return false;
        }
        else {
            // 够钱进行多次升级, 创建一个确认操作
            bg_http_reply(bgReq.req, 200, json {
                { "type", eqiType_to_str(type) },
                { "times", upgradeTimes },
                { "coins", coinsNeeded }
            }.dump().c_str());

            if (PLAYER.confirmInProgress) {
                // 如果玩家当前有进行中的确认, 那么取消掉正在进行的确认, 并等待那个确认退出
                PLAYER.abortUpgrade();
                PLAYER.waitConfirmComplete();
            }
            return PLAYER.waitUpgradeConfirm();         // 等待玩家进行确认
        }
    }
    return true;
}

// 强化装备
void postUpgradeCallback(const bgGameHttpReq& bgReq, const EqiType& type, LL upgradeTimes, LL coinsNeeded) {
    try {
        // 扣硬币
        if (!PLAYER.inc_coins(-coinsNeeded)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_DEC_COINS_FAILED, BG_ERR_DEC_COINS_FAILED);
            return;
        }

        // 升级
        auto currItem = PLAYER.get_equipments_item(type);
        currItem.level += upgradeTimes;
        if (!PLAYER.set_equipments_item(type, currItem)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SET_EQI_FAILED, BG_ERR_SET_EQI_FAILED);
            return;
        }

        // 发送确认消息
        bg_http_reply(bgReq.req, 200, json {
            { "times", upgradeTimes },
            { "name", allEquipments.at(currItem.id).name + "+" + std::to_string(currItem.level) },
            { "coins", coinsNeeded },
            { "coinsLeft", PLAYER.get_coins() }
        }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 确认强化前检查
bool preConfirmUpgradeCallback(const bgGameHttpReq& bgReq) {
    if (!accountCheck(bgReq))
        return false;

    if (!PLAYER.confirmInProgress) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_NO_PENDING_UPGRADE, BG_ERR_NO_PENDING_UPGRADE);
        return false;
    }
    return true;
}

// 确认强化
void postConfirmUpgradeCallback(const bgGameHttpReq& bgReq) {
    PLAYER.confirmUpgrade();
    bg_http_reply(bgReq.req, 200, "{}");
}

// 升级祝福前检查
bool preUpgradeBlessingCallback(const bgGameHttpReq &bgReq, LL upgradeTimes, LL &coinsNeeded) {
    if (!accountCheck(bgReq))
        return false;

    // 获取升级次数
    try {
        if (upgradeTimes < 1) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INVALID_UPGRADE_TIMES, BG_ERR_INVALID_UPGRADE_TIMES);
            return false;
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
    if (upgradeTimes > 100) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_MAX_UPGRADE_TIMES, BG_ERR_MAX_UPGRADE_TIMES);
        return false;
    }

    // 计算升级所需硬币
    if (upgradeTimes == 1) {
        // 祝福升一级价格 = 1500 * 当前祝福等级
        coinsNeeded = 1500 * PLAYER.get_blessing();
        if (PLAYER.get_coins() < coinsNeeded) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INSUFFICIENT_COINS +
                std::string(": 还需要") + std::to_string(coinsNeeded - PLAYER.get_coins()) + "硬币",
                BG_ERR_INSUFFICIENT_COINS
            );
            return false;
        }
        return true;
    }
    else {
        //   祝福升 n 级价格
        // = sum(1500x, x, 当前祝福等级, 当前祝福等级 + n - 1)
        // = -750 * (当前祝福等级 ^ 2 - 当前祝福等级) + 750 * ((当前祝福等级 + n) ^ 2 - n - 当前祝福等级)
        auto currBlessing = PLAYER.get_blessing();
        coinsNeeded = -750 * (currBlessing * currBlessing - currBlessing) + 750 * ((currBlessing + upgradeTimes) * (currBlessing + upgradeTimes) - upgradeTimes - currBlessing);
        auto currCoins = PLAYER.get_coins();
        if (currCoins < coinsNeeded) {
            // 如果不够硬币, 则计算用户可以升级多少级, 即对下列方程中的 n 求解:
            // -750 * (当前祝福等级 ^ 2 - 当前祝福等级) + 750 * ((当前祝福等级 + n) ^ 2 - n - 当前祝福等级) = 当前硬币
            // 解得:
            // n = (1 / 750) * (5 * sqrt(15) * sqrt(1500 * 当前祝福等级 ^ 2 + 2 * 当前硬币 - 1500 * 当前祝福等级 + 375) - 750 * 当前祝福等级 + 375)
            //   ≈ 0.00133333333333 * (-750.0 * 当前祝福等级 + 19.364916731 * sqrt(1500.0 * 当前祝福等级 ^ 2 + 2.0 * coins - 1500.0 * 当前祝福等级 + 375.0) + 375.0)
            // 再取 floor
            upgradeTimes = static_cast<LL>(floor(
                0.00133333333333 * (-750.0 * currBlessing + 19.364916731 * std::sqrt(1500.0 * currBlessing * currBlessing + 2.0 * currCoins - 1500.0 * currBlessing + 375.0) + 375.0)
            ));

            if (upgradeTimes < 1) {
                // 玩家的钱一次都升级不了
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INSUFFICIENT_COINS +
                    std::string(": 还需要") + std::to_string(coinsNeeded - currCoins) + "硬币",
                    BG_ERR_INSUFFICIENT_COINS
                );
            }
            else {
                // 重算需要硬币
                coinsNeeded = -750 * (currBlessing * currBlessing - currBlessing) + 750 * ((currBlessing + upgradeTimes) * (currBlessing + upgradeTimes) - upgradeTimes - currBlessing);
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INSUFFICIENT_COINS +
                    std::string(": 当前硬币只够升级祝福") + std::to_string(upgradeTimes) + "次, 强化完之后还剩" +
                    std::to_string(currCoins - coinsNeeded) + "硬币",
                    BG_ERR_INSUFFICIENT_COINS
                );
            }
            return false;
        }
        else {
            // 够钱进行多次升级, 创建一个确认操作
            bg_http_reply(bgReq.req, 200, json{
                { "times", upgradeTimes },
                { "coins", coinsNeeded }
            }.dump().c_str());

            if (PLAYER.confirmInProgress) {
                // 如果玩家当前有进行中的确认, 那么取消掉正在进行的确认, 并等待那个确认退出
                PLAYER.abortUpgrade();
                PLAYER.waitConfirmComplete();
            }
            return PLAYER.waitUpgradeConfirm();         // 等待玩家进行确认
        }
    }
}

// 升级祝福
void postUpgradeBlessingCallback(const bgGameHttpReq &bgReq, LL upgradeTimes, LL coinsNeeded) {
    try {
        // 扣硬币
        if (!PLAYER.inc_coins(-coinsNeeded)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_DEC_COINS_FAILED, BG_ERR_DEC_COINS_FAILED);
            return;
        }

        // 升级
        if (!PLAYER.inc_blessing(upgradeTimes)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_INC_BLESSING_FAILED, BG_ERR_INC_BLESSING_FAILED);
            return;
        }

        // 发送确认消息
        bg_http_reply(bgReq.req, 200, json{
            { "times", upgradeTimes },
            { "blessing", PLAYER.get_blessing() },
            { "coins", coinsNeeded },
            { "coinsLeft", PLAYER.get_coins() }
        }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 查看交易场前检查
bool preViewTradeCallback(const bgGameHttpReq& bgReq) {
    return accountCheck(bgReq);
}

// 查看交易场
void postViewTradeCallback(const bgGameHttpReq& bgReq) {
    try {
        auto &items = bg_trade_get_items();
        json content = {};

        for (const auto &[tradeId, data] : items) {
            if (allEquipments.at(data.item.id).type == EqiType::single_use) {
                // 一次性商品
                content.push_back(json {
                    { "id", tradeId },
                    { "name", "[" + allEquipments.at(data.item.id).name + "]" },
                    { "price", data.price },
                    { "private", data.hasPassword }
                });
            }
            else {
                // 普通商品
                content.push_back(json {
                    { "id", tradeId },
                    { "name", allEquipments.at(data.item.id).name + "+" + std::to_string(data.item.level) },
                    { "wear", data.item.wear },
                    { "originalWear", allEquipments.at(data.item.id).wear },
                    { "price", data.price },
                    { "private", data.hasPassword }
                });
            }
        }

        bg_http_reply(bgReq.req, 200, content.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 购买交易场商品前检查
bool preBuyTradeCallback(const bgGameHttpReq& bgReq, LL tradeId, const std::string &password) {
        try {
        if (!accountCheck(bgReq))
            return false;
        
        // 检查指定 ID 是否存在
        if (allTradeItems.find(tradeId) == allTradeItems.end()) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_TRADEID_INVALID, BG_ERR_TRADEID_INVALID);
            return false;
        }

        // 检查是否有密码
        if (allTradeItems.at(tradeId).hasPassword) {
            if (password.empty()) {
                // 有密码但是玩家没有提供密码
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PASSWORD_REQUIRED, BG_ERR_PASSWORD_REQUIRED);
                return false;
            }
            else {
                // 检查密码是否匹配
                if (password != allTradeItems.at(tradeId).password) {
                    bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PASSWORD_INCORRECT, BG_ERR_PASSWORD_INCORRECT);
                    return false;
                }
            }
        }
        else {
            if (!password.empty()) {
                // 没有密码但是玩家提供了密码
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PASSWORD_NOT_REQUIRED, BG_ERR_PASSWORD_NOT_REQUIRED);
                return false;
            }
        }

        // 检查是否够钱买
        if (PLAYER.get_coins() < allTradeItems.at(tradeId).price) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INSUFFICIENT_COINS +
                std::string(": 还差") + std::to_string(allTradeItems.at(tradeId).price - PLAYER.get_coins()) + "硬币",
                BG_ERR_INSUFFICIENT_COINS);
            return false;
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
    return true;
}

// 购买交易场商品
void postBuyTradeCallback(const bgGameHttpReq& bgReq, LL tradeId) {
    try {
        tradeData item = allTradeItems.at(tradeId);

        // 交易场移除对应条目
        if (!bg_trade_remove_item(tradeId)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_REMOVE_TRADE_FAILED, BG_ERR_REMOVE_TRADE_FAILED);
            return;
        }

        // 买方扣钱
        if (!PLAYER.inc_coins(-item.price)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_DEC_COINS_FAILED, BG_ERR_DEC_COINS_FAILED);
            return;
        }

        // 背包添加相应物品
        if (!PLAYER.add_inventory_item(item.item)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_ADD_ITEM_FAILED, BG_ERR_ADD_ITEM_FAILED);
            return;
        }

        // 卖方加钱
        if (!bg_player_get(item.sellerId).inc_coins(item.price)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_INC_COINS_FAILED, BG_ERR_INC_COINS_FAILED);
            return;
        }

        // 按照装备类型发送购买成功消息
        if (allEquipments.at(item.item.id).type == EqiType::single_use) {
            bg_http_reply(bgReq.req, 200, json {
                { "name", allEquipments.at(item.item.id).name },
                { "coins", item.price }
            }.dump().c_str());
        }
        else {
            bg_http_reply(bgReq.req, 200, json {
                { "name", allEquipments.at(item.item.id).name + "+" + std::to_string(item.item.level) },
                { "wear", item.item.wear },
                { "originalWear", allEquipments.at(item.item.id).wear },
                { "coins", item.price }
            }.dump().c_str());
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 上架交易场商品前检查
bool preSellTradeCallback(const bgGameHttpReq& bgReq, LL invId, LL price, bool hasPassword) {
    try {
        if (!accountCheck(bgReq))
            return false;

        // todo 检查玩家是否已经私密交易过

        // 检查背包序号是否超出范围
        if (invId >= PLAYER.get_inventory_size() || invId < 0) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ID_OUT_OF_RANGE + std::string(": ") + std::to_string(invId + 1), BG_ERR_ID_OUT_OF_RANGE);
            return false;
        }

        // 检查价格是否合理
        // 默认价格 * 0.25 <= 价格 <= 默认价格 * 10
        auto playerInv = PLAYER.get_inventory();
        auto it = playerInv.begin();
        std::advance(it, invId);                                                                                    // 获取 ID 对应的背包物品
        LL defPrice = static_cast<LL>(allEquipments.at(it->id).price + 100.0 * (pow(1.6, it->level) - 1) / 0.6);    // 获取该物品的默认出售价格
        if (defPrice / 4 > price || price > defPrice * 10) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INAPPROPRIATE_PRICE + std::string(": 价格不可低于出售价格的百分之25 (") +
                std::to_string(defPrice / 4) + "), 也不可高于出售价格的十倍 (" + std::to_string(defPrice * 10) + ")",
                BG_ERR_INAPPROPRIATE_PRICE);
            return false;
        }

        // 检查是否够钱交税
        if (PLAYER.get_coins() < price * 0.05) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_CANT_AFFORD_TAX + std::string(": 还需要") +
                std::to_string(static_cast<LL>(price * 0.05 - PLAYER.get_coins())) + "硬币",
                BG_ERR_CANT_AFFORD_TAX);
            return false;
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
    return true;
}

// 上架交易场商品
void postSellTradeCallback(const bgGameHttpReq& bgReq, LL invId, LL price, bool hasPassword) {
    try {
        // 如果是有密码的交易, 则随机生成一个密码
        std::string password = "";
        if (hasPassword) {
            password = std::to_string(rndRange(100000, 999999));
        }

        // 从玩家背包移除指定项目
        auto playerInv = PLAYER.get_inventory();
        auto it = playerInv.begin();
        std::advance(it, invId);                        // 获取 ID 对应的背包物品
        if (!PLAYER.remove_at_inventory(invId)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_REMOVE_ITEM_FAILED, BG_ERR_REMOVE_ITEM_FAILED);
            return;
        }

        // 扣除税款
        LL tax = static_cast<LL>(price * 0.05);
        if (tax < 1)
            tax = 1;
        if (!PLAYER.inc_coins(-tax)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_DEC_COINS_FAILED, BG_ERR_DEC_COINS_FAILED);
            return;
        }

        // 获取并更新交易场 ID
        tradeData   item;
        item.item = *it;
        item.password = password;
        item.hasPassword = hasPassword;
        item.price = price;
        item.sellerId = bgReq.playerId;
        item.tradeId = bg_get_tradeId();
        item.addTime = dateTime().get_timestamp();
        if (!bg_inc_tradeId()) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_TRADEID_UPDATE_FAILED, BG_ERR_TRADEID_UPDATE_FAILED);
            return;
        }

        // 添加物品到交易场
        if (!bg_trade_insert_item(item)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_ADD_TRADE_FAILED, BG_ERR_ADD_TRADE_FAILED);
            return;
        }

        // 发送消息
        bg_http_reply(bgReq.req, 200, json {
            { "tradeId", item.tradeId },
            { "tax", tax },
            { "password", password }
        }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 下架交易场商品前检查
bool preRecallTradeCallback(const bgGameHttpReq& bgReq, LL tradeId) {
    if (!accountCheck(bgReq))
        return false;

    try {
        // 检查交易 ID 是否在交易场中
        if (allTradeItems.find(tradeId) == allTradeItems.end()) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_TRADEID_INVALID, BG_ERR_TRADEID_INVALID);
            return false;
        }

        // 检查卖方是否是玩家
        if (allTradeItems.at(tradeId).sellerId != bgReq.playerId) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PLAYER_MISMATCH, BG_ERR_PLAYER_MISMATCH);
            return false;
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
    return true;
}

// 下架交易场商品
void postRecallTradeCallback(const bgGameHttpReq& bgReq, LL tradeId) {
    try {
        // 从交易场移除对应物品
        tradeData item = allTradeItems.at(tradeId);
        if (!bg_trade_remove_item(tradeId)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_REMOVE_TRADE_FAILED, BG_ERR_REMOVE_TRADE_FAILED);
            return;
        }

        // 添加物品到玩家背包
        if (!PLAYER.add_inventory_item(item.item)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_ADD_ITEM_FAILED, BG_ERR_ADD_ITEM_FAILED);
            return;
        }

        const auto& eqi = allEquipments.at(item.item.id);
        if (eqi.type == EqiType::single_use) {
            bg_http_reply(bgReq.req, 200, json {
                { "tradeId", tradeId },
                { "name", eqi.name }
            }.dump().c_str());
        }
        else {
            bg_http_reply(bgReq.req, 200, json {
                { "tradeId", tradeId },
                { "name", eqi.name + "+" + std::to_string(item.item.level) }
            }.dump().c_str());
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 合成前检查
bool preSynthesisCallback(const bgGameHttpReq& bgReq, const std::set<LL, std::greater<LL>>& invList, const std::string &target, LL &targetId, LL& coins, LL& level) {
    if (!accountCheck(bgReq))
        return false;

    try {
        // 尝试根据指定的装备 ID 或者名称获取对应的装备
        try {
            targetId = str_to_ll(target);

            // 检查目标装备是否存在
            if (allEquipments.find(targetId) == allEquipments.end()) {
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INVALID_EQI_ID +
                    std::string(": ") + std::to_string(targetId), BG_ERR_INVALID_EQI_ID);
                return false;
            }
        }
        catch (...) {
            auto eqis = bg_search_equipment(target);

            // 去掉不能合成的装备
            auto it = std::remove_if(eqis.begin(), eqis.end(), [](auto &item) {
                return std::get<2>(item) == EqiType::single_use || allSyntheses.find(std::get<0>(item)) == allSyntheses.end();
            });
            eqis.erase(it, eqis.end());

            if (eqis.size() == 0)                   // 没有找到对应名称的装备
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_EQI_NOT_FOUND, BG_ERR_EQI_NOT_FOUND);
            else if (eqis.size() == 1)              // 找到唯一匹配, 继续
                targetId = std::get<0>(eqis[0]);
            else                                    // 找到多个匹配, 取消操作, 询问用户要合成哪个
                bg_http_reply(bgReq.req, 200, json{ { "matches", eqis } }.dump().c_str());
        }

        // 如果只指定了目标装备, 则列出可用的合成
        if (invList.empty()) {
            const auto result = allSyntheses.equal_range(targetId);         // 获取可行的合成方案
            if (std::distance(result.first, result.second) == 0) {          // 没有合成方案
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_CANT_SYNTHESIS +
                    std::string(": ") + allEquipments.at(targetId).name, BG_ERR_CANT_SYNTHESIS);
                return false;
            }

            std::vector<std::pair<std::vector<std::string>, LL>> methods;
            for (auto it = result.first; it != result.second; ++it) {       // 生成所有合成方案的字串
                std::vector<std::string> requirements;
                for (const auto &item : it->second.requirements) {
                    requirements.push_back(allEquipments.at(item).name);
                }
                methods.push_back({ requirements, it->second.coins });
            }
            bg_http_reply(bgReq.req, 200, json { { "methods", methods }, { "name", allEquipments.at(targetId).name } }.dump().c_str());
            return false;
        }

        auto inventory = PLAYER.get_inventory();                            // 获取玩家背包
        for (const auto &index : invList) {
            if (index < 0 || index >= inventory.size()) {                   // 不允许序号超出背包范围
                bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_ID_OUT_OF_RANGE +
                    std::string(": ") + std::to_string(index + 1), BG_ERR_ID_OUT_OF_RANGE);
                return false;
            }
        }

        std::unordered_multiset<LL> materials;                              // 所有提供的材料
        auto invIter = inventory.begin();
        LL prevInvId = 0;
        for (auto invIdIter = invList.begin(); invIdIter != invList.end(); ++invIdIter) {
            std::advance(invIter, *invIdIter - prevInvId);
            materials.insert(invIter->id);
            if (invIter->level > level)                                     // 获取材料的最高等级
                level = invIter->level;
            prevInvId = *invIdIter;
        }

        if (!bg_match_synthesis(targetId, materials, coins)) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_SYNTHESIS_NOT_EXIST, BG_ERR_SYNTHESIS_NOT_EXIST);
            return false;
        }

        if (PLAYER.get_coins() < coins) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INSUFFICIENT_COINS +
                std::string(": 还需要") + std::to_string(coins - PLAYER.get_coins()) + "硬币", BG_ERR_INSUFFICIENT_COINS);
            return false;
        }
        return true;
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
}

// 合成
void postSynthesisCallback(const bgGameHttpReq& bgReq, const std::set<LL, std::greater<LL>>& invList, LL targetId, LL coins, LL level) {
    try {
        // 扣除硬币
        if (!PLAYER.inc_coins(-coins)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_DEC_COINS_FAILED, BG_ERR_DEC_COINS_FAILED);
            return;
        }

        // 从玩家背包移除装备
        std::vector<LL> invId;
        for (const auto &inv : invList)
            invId.push_back(inv);
        if (!PLAYER.remove_at_inventory(invId)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_REMOVE_ITEM_FAILED, BG_ERR_REMOVE_ITEM_FAILED);
            return;
        }

        // 添加合成后的装备到玩家背包
        inventoryData newItem;
        newItem.id = targetId;
        newItem.level = level;
        newItem.wear = allEquipments.at(targetId).wear;
        if (!PLAYER.add_inventory_item(newItem)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_ADD_ITEM_FAILED +
                std::string(": ") + allEquipments.at(targetId).name + "+" + std::to_string(level), BG_ERR_ADD_ITEM_FAILED);
            return;
        }

        bg_http_reply(bgReq.req, 200, json {
            { "name", allEquipments.at(targetId).name + "+" + std::to_string(level) },
            { "coins", coins }
        }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// 挑战副本前检查
bool preFightCallback(const bgGameHttpReq& bgReq, const std::string&levelName, LL &levelId) {
    if (!accountCheck(bgReq))
        return false;

    try {
        // 检查等级是否有效
        levelId = str_to_ll(levelName);
        if (allDungeons.find(levelId) == allDungeons.end()) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INVALID_DUNGEON, BG_ERR_INVALID_DUNGEON);
            return false;
        }

        // 检查玩家是否还有体力
        if (PLAYER.get_energy() < 10) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_NO_ENERGY, BG_ERR_NO_ENERGY);
            return false;
        }

        // 检查冷却时间
        LL timeDiff = dateTime().get_timestamp() - PLAYER.get_lastFight();
        LL fightCd = cdOverride == -1 ? PLAYER.get_cd() : cdOverride;
        if (timeDiff < fightCd) {
            timeDiff = fightCd - timeDiff;
            LL min = timeDiff / 60;
            LL sec = timeDiff % 60;
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_IN_CD +
                std::string(", 还剩") + (min < 10 ? "0" : "") + std::to_string(min) + ":" + (sec < 10 ? "0" : "") + std::to_string(sec), BG_ERR_IN_CD);
            return false;
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
    return true;
}

// 挑战副本
void postFightCallback(const bgGameHttpReq& bgReq, LL levelId) {
    try {
        // 从副本抽取一个怪物
        LL monsterId = allDungeons[levelId].monstersDraw.draw();
        monsterData &monster = allMonsters[monsterId];

        // 扣除玩家体力
        if (!PLAYER.inc_energy(-10)) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_DEC_ENERGY_FAILED, BG_ERR_DEC_ENERGY_FAILED);
            return;
        }

        // 更新最后打怪时间
        if (!PLAYER.set_lastFight(dateTime().get_timestamp())) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SET_FIGHTTIME_FAILED, BG_ERR_SET_FIGHTTIME_FAILED);
            return;
        }

        // 进行对战
        bool playerFirst, playerWins;
        std::string preMsg, postMsg;
        auto rounds = bg_fight(fightable(PLAYER), fightable(monster), playerWins, playerFirst, preMsg, postMsg);

        std::vector<std::pair<std::string, unsigned char>> drops;       // [[装备名称, 装备类型], ...]
        std::vector<std::pair<std::string, int>> errors;
        LL loseCoins;
        if (playerWins) {
            // 如果背包还有空间则掉落装备
            if (PLAYER.get_inventory_size() < PLAYER.get_invCapacity()) {
                std::unordered_set<LL> dropped;                             // 已经掉落的装备
                for (int i = 0; i < monster.drop.size(); i++) {
                    LL drop = monster.dropDraw.draw();
                    if (dropped.find(drop) == dropped.end())                // 防止掉落重复的装备
                        dropped.insert(drop);
                    else
                        continue;

                    if (drop != -1) {
                        auto &item = allEquipments[drop];
                        inventoryData itemData;

                        itemData.id = drop;
                        itemData.level = 0;
                        itemData.wear = item.wear;
                        drops.push_back({ item.name, static_cast<unsigned char>(item.type) });
                        if (!PLAYER.add_inventory_item(itemData)) {
                            errors.push_back({ BG_ERR_STR_ADD_ITEM_FAILED + std::string(": ") + item.name, BG_ERR_ADD_ITEM_FAILED });
                        }
                    }
                }
            }
            else {
                if (!postMsg.empty() && postMsg.back() != '\n')
                    postMsg += "\n";
                postMsg += "【警告】你的背包已满, 将不能获得掉落物品! 请及时清理或者扩容。";
            }

            // 添加经验
            if (!PLAYER.inc_exp(monster.exp)) {
                errors.push_back({ BG_ERR_STR_INC_EXP_FAILED + std::string(": ") + std::to_string(monster.exp), BG_ERR_INC_EXP_FAILED });
            }
            if (!PLAYER.check_exp_upgrade()) {
                errors.push_back({ BG_ERR_STR_INC_EXP_FAILED , BG_ERR_INC_EXP_FAILED });
            }

            // 添加硬币
            if (!PLAYER.inc_coins(monster.coin)) {
                errors.push_back({ BG_ERR_STR_INC_COINS_FAILED + std::string(": ") + std::to_string(monster.coin), BG_ERR_INC_COINS_FAILED });
            }
        }
        else {
            // 扣除硬币
            // 失去硬币数 = [0, max(怪物硬币数 - sqrt(怪物硬币数 * 玩家等级), 0)] 范围内的随机数
            double  dLoseCoins = static_cast<double>(monster.coin);
            dLoseCoins = std::max(dLoseCoins - sqrt(dLoseCoins * PLAYER.get_level()), 0.0);
            loseCoins = rndRange(static_cast<LL>(dLoseCoins) / 3, static_cast<LL>(dLoseCoins));
            if (!PLAYER.inc_coins(loseCoins)) {
                errors.push_back({ BG_ERR_STR_DEC_COINS_FAILED + std::string(": ") + std::to_string(loseCoins), BG_ERR_DEC_COINS_FAILED });
            }

            // 返还一部分体力
            if (!PLAYER.inc_energy(5)) {
                errors.push_back({ BG_ERR_STR_DEC_ENERGY_FAILED, BG_ERR_DEC_ENERGY_FAILED });
                return;
            }
        }

        // 把怪物出场消息和战斗前消息拼接起来
        if (monster.message.length() > 0 && preMsg.length() > 0)
            preMsg = monster.message + "\n" + preMsg;
        
        bg_http_reply(bgReq.req, 200, json {
            { "fighterFirst", playerFirst },
            { "rounds", rounds },
            { "targetName", monster.name },
            { "msg", preMsg },
            { "drops", drops },
            { "coins", playerWins ? monster.coin : loseCoins },
            { "exp", monster.exp },
            { "energy", PLAYER.get_energy() },
            { "postMsg", postMsg },
            { "win", playerWins },
            { "errors", errors }
        }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// PVP 前检查
bool prePvpCallback(const bgGameHttpReq &bgReq, LL targetId) {
    if (!accountCheck(bgReq))
        return false;

    try {
        // 检查等级是否有效
        if (!bg_player_exist(targetId)) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INVALID_TARGET, BG_ERR_INVALID_TARGET);
            return false;
        }

        // 检查冷却时间
        LL timeDiff = dateTime().get_timestamp() - PLAYER.get_lastFight();
        LL fightCd = cdOverride == -1 ? PLAYER.get_cd() : cdOverride;
        if (timeDiff < fightCd) {
            timeDiff = fightCd - timeDiff;
            LL min = timeDiff / 60;
            LL sec = timeDiff % 60;
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_IN_CD +
                std::string(", 还剩") + (min < 10 ? "0" : "") + std::to_string(min) + ":" + (sec < 10 ? "0" : "") + std::to_string(sec), BG_ERR_IN_CD);
            return false;
        }
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED + std::string(": ") + e.what(), BG_ERR_PRE_OP_FAILED);
        return false;
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_PRE_OP_FAILED, BG_ERR_PRE_OP_FAILED);
        return false;
    }
    return true;
}

// PVP
void postPvpCallback(const bgGameHttpReq &bgReq, LL targetId) {
    try {
        // 更新最后打怪时间
        if (!PLAYER.set_lastFight(dateTime().get_timestamp())) {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_SET_FIGHTTIME_FAILED, BG_ERR_SET_FIGHTTIME_FAILED);
            return;
        }

        // 进行对战
        bool playerFirst, playerWins;
        std::string preMsg, postMsg;
        auto rounds = bg_fight(fightable(PLAYER), fightable(bg_player_get(targetId)), playerWins, playerFirst, preMsg, postMsg);

        // todo 出场消息

        bg_http_reply(bgReq.req, 200, json{
            { "fighterFirst", playerFirst },
            { "rounds", rounds },
            { "targetName", std::to_string(targetId) },     // todo 昵称
            { "msg", preMsg },
            { "postMsg", postMsg },
            { "win", playerWins }
        }.dump().c_str());
    }
    catch (const std::exception &e) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED + std::string(": ") + e.what(), BG_ERR_POST_OP_FAILED);
    }
    catch (...) {
        bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
    }
}

// =====================================================================================================
// 管理指令
// =====================================================================================================

// 管理员为玩家的某个属性添加数值前检查
// fieldType: 0: coins; 1: heroCoin; 2: level; 3: blessing; 4: energy; 5: exp; 6: invCapacity; 7: vip
// mode: 0: inc; 1: set
bool preAdminModifyFieldCallback(const bgGameHttpReq &bgReq, unsigned char fieldType, unsigned char mode, LL targetId, LL val) {
    // 检察权限
    if (!bg_is_admin(bgReq.playerId)) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_NOT_ADMIN, BG_ERR_NOT_ADMIN);
        return false;
    }

    // 检查请求类型
    if (fieldType > 7 || mode > 1) {
        bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INVALID_ADMIN_REQUEST, BG_ERR_INVALID_ADMIN_REQUEST);
        return false;
    }

    // 获取目标玩家
    if (targetId != -1) {
        if (!bg_player_exist(targetId)) {
            bg_http_reply_error(bgReq.req, 400, BG_ERR_STR_INVALID_TARGET, BG_ERR_INVALID_TARGET);
            return false;
        }
    }
    return true;
}

// 管理员为玩家的某个属性添加数值
void postAdminModifyFieldCallback(const bgGameHttpReq &bgReq, unsigned char fieldType, unsigned char mode, LL targetId, LL val) {
    bool rtn = false; 
    
    if (targetId == -1) {                           // 为所有玩家修改属性
        if (mode == 0) {                                // 增加数值
            switch (fieldType) {
            case 0:
                rtn = bg_all_player_inc_coins(val);
                break;
            case 1:
                rtn = bg_all_player_inc_heroCoin(val);
                break;
            case 2:
                rtn = bg_all_player_inc_level(val);
                break;
            case 3:
                rtn = bg_all_player_inc_blessing(val);
                break;
            case 4:
                rtn = bg_all_player_inc_energy(val);
                break;
            case 5:
                rtn = bg_all_player_inc_exp(val);
                break;
            case 6:
                rtn = bg_all_player_inc_invCapacity(val);
                break;
            case 7:
                rtn = bg_all_player_inc_vip(val);
                break;
            }
        }
        else if (mode == 1) {                           // 修改数值
            switch (fieldType) {
            case 0:
                rtn = bg_all_player_set_coins(val);
                break;
            case 1:
                rtn = bg_all_player_set_heroCoin(val);
                break;
            case 2:
                rtn = bg_all_player_set_level(val);
                break;
            case 3:
                rtn = bg_all_player_set_blessing(val);
                break;
            case 4:
                rtn = bg_all_player_set_energy(val);
                break;
            case 5:
                rtn = bg_all_player_set_exp(val);
                break;
            case 6:
                rtn = bg_all_player_set_invCapacity(val);
                break;
            case 7:
                rtn = bg_all_player_set_vip(val);
                break;
            }
        }
        if (rtn) {
            bg_http_reply(bgReq.req, 200, json {
                { "count", bg_player_get_count() },
                { "val", val }
            }.dump().c_str());
        }
        else {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
        }
    }
    else {                                          // 为指定玩家修改属性
        if (mode == 0) {                                // 增加数值
            switch (fieldType) {
            case 0:
                rtn = bg_player_get(targetId).inc_coins(val);
                break;
            case 1:
                rtn = bg_player_get(targetId).inc_heroCoin(val);
                break;
            case 2:
                rtn = bg_player_get(targetId).inc_level(val);
                break;
            case 3:
                rtn = bg_player_get(targetId).inc_blessing(val);
                break;
            case 4:
                rtn = bg_player_get(targetId).inc_energy(val);
                break;
            case 5:
                rtn = bg_player_get(targetId).inc_exp(val);
                rtn &= bg_player_get(targetId).check_exp_upgrade();
                break;
            case 6:
                rtn = bg_player_get(targetId).inc_invCapacity(val);
                break;
            case 7:
                rtn = bg_player_get(targetId).inc_vip(val);
                break;
            }
        }
        else if (mode == 1) {                           // 修改数值
            switch (fieldType) {
            case 0:
                rtn = bg_player_get(targetId).set_coins(val);
                break;
            case 1:
                rtn = bg_player_get(targetId).set_heroCoin(val);
                break;
            case 2:
                rtn = bg_player_get(targetId).set_level(val);
                break;
            case 3:
                rtn = bg_player_get(targetId).set_blessing(val);
                break;
            case 4:
                rtn = bg_player_get(targetId).set_energy(val);
                break;
            case 5:
                rtn = bg_player_get(targetId).set_exp(val);
                for (auto &p : allPlayers) {
                    std::unique_lock lock(p.second.mutexPlayer);
                    p.second.check_exp_upgrade();
                    lock.unlock();
                }
                break;
            case 6:
                rtn = bg_player_get(targetId).set_invCapacity(val);
                break;
            case 7:
                rtn = bg_player_get(targetId).set_vip(val);
                break;
            }
        }
        if (rtn) {
            bg_http_reply(bgReq.req, 200, json {
                { "player", targetId },
                { "val", val }
            }.dump().c_str());
        }
        else {
            bg_http_reply_error(bgReq.req, 500, BG_ERR_STR_POST_OP_FAILED, BG_ERR_POST_OP_FAILED);
        }
    }
}
