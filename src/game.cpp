/*
描述: Bingy 游戏相关的函数. 整个游戏的主要逻辑
作者: 冰棍
文件: game.cpp
*/

#include "game.hpp"
#include "player.hpp"
#include "utils.hpp"
#include "signin_event.hpp"
#include <mongocxx/exception/exception.hpp>
#include <unordered_set>
#include <sstream>

std::unordered_set<LL>  blacklist;          // 黑名单 (修改项目前记得加锁)
std::unordered_set<LL>  allAdmins;          // 管理员 (修改项目前记得加锁)

// 取得艾特玩家字符串
std::string bg_at(const cq::MessageEvent &ev) {
    try {
        auto gmi = cq::get_group_member_info(GROUP_ID, USER_ID);
        return "[CQ:at,qq=" + std::to_string(gmi.user_id) + "] ";
    }
    catch (...) {
        return "@" + std::to_string(USER_ID) + " ";
    }
}

// 通用账号检查
bool accountCheck(const cq::MessageEvent &ev) {
    // 检查玩家是否已经注册
    if (!bg_player_exist(USER_ID)) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "要先注册哦! 快发送\"bg 注册\"加入游戏吧!");
        return false;
    }

    // 检查玩家是否在小黑屋
    if (blacklist.find(USER_ID) != blacklist.end()) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "你被拉黑了!");
        return false;
    }

    return true;
}

// 注册前检查
bool preRegisterCallback(const cq::MessageEvent &ev) {
    if (bg_player_exist(USER_ID)) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "你已经注册过啦!");
        return false;
    }
    return true;
}

// 注册
void postRegisterCallback(const cq::MessageEvent &ev) {
    try {
        if (bg_player_add(USER_ID))
            cq::send_group_message(GROUP_ID, bg_at(ev) + "注册成功!");
        else
            cq::send_group_message(GROUP_ID, bg_at(ev) + "注册期间发生错误!");
    }
    catch (mongocxx::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("注册期间发生错误: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "注册期间发生错误!");
    }
}

// 查看硬币前检查
bool preViewCoinsCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// 查看硬币
void postViewCoinsCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "硬币数: " +
            std::to_string(PLAYER.get_coins())
        );
    }
    catch (mongocxx::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("查看硬币发生错误: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看硬币发生错误!");
    }
}

// 签到前检查
bool preSignInCallback(const cq::MessageEvent &ev) {
    if (!accountCheck(ev))
        return false;

    // 如果玩家上次签到日期跟今天一样则拒绝签到
    dateTime signInDate = dateTime(static_cast<time_t>(PLAYER.get_lastSignIn()));
    dateTime today = dateTime();

    if (signInDate.get_year() == today.get_year() && signInDate.get_month() == today.get_month() && signInDate.get_day() == today.get_day()) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "你今天已经签到过啦!");
        return false;
    }
    return true;
}

// 签到
void postSignInCallback(const cq::MessageEvent &ev) {
    try {
        dateTime now;

        // 检查连续签到
        if (is_day_sequential(dateTime(static_cast<time_t>(PLAYER.get_lastFight())), now)) {
            if (!PLAYER.inc_signInCountCont(1)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: 设置连续签到天数失败"));
                return;
            }
        }
        else {
            if (!PLAYER.set_signInCountCont(1)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: 设置连续签到天数失败"));
                return;
            }
        }

        // 首先设置签到时间, 免得真的出了漏洞给玩家不断签到
        if (!PLAYER.set_lastSignIn(static_cast<LL>(dateTime().get_timestamp()))) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: 设置签到时间失败"));
            return;
        }

        // 设置玩家签到次数
        if (!PLAYER.inc_signInCount(1)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: 添加签到次数失败"));
            return;
        }

        // 签到获得硬币 = 500 + 25 * 连续签到天数 + 10 * 总签到天数 + rnd(20 * 总签到天数)
        // 签到*之后的*体力 = 15 * 连续签到天数 + 5 * 总签到天数
        // 签到获得经验 = 15 * 连续签到次数 + 5 * 总签到天数
        LL deltaCoins = 500 + 25 * PLAYER.get_signInCountCont() + 10 * PLAYER.get_signInCount() + rndRange(20 * PLAYER.get_signInCount());
        LL deltaEnergy = 150 + 5 * PLAYER.get_level();
        LL deltaExp = 15 * PLAYER.get_signInCountCont() + 5 * PLAYER.get_signInCount();

        // 检查签到活动
        std::string eventMsg = "";              // 活动消息
        std::vector<LL> eventItems;             // 活动赠送物品
        bg_match_sign_in_event(now, deltaCoins, deltaEnergy, eventItems, eventMsg);
        for (const auto &item : eventItems) {   // 为玩家添加物品
            inventoryData itemData;
            itemData.id = item;
            itemData.level = 0;
            itemData.wear = allEquipments.at(item).wear;
            if (!PLAYER.add_inventory_item(itemData)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: 添加物品\"" + allEquipments.at(item).name + "\"失败"));
            }
        }

        if (!PLAYER.inc_coins(deltaCoins)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: 添加硬币失败"));
        }
        if (!PLAYER.set_energy(static_cast<LL>(PLAYER.get_energy() * 0.75) + deltaEnergy)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: 添加体力失败"));
        }
        if (!PLAYER.inc_exp(deltaExp)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: 添加经验失败"));
        }

        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string(
            "签到成功, 连续签到" + std::to_string(PLAYER.get_signInCountCont()) + "天, 总共签到" + std::to_string(PLAYER.get_signInCount()) + "天\n"
            "获得硬币: " + std::to_string(deltaCoins) + "   拥有硬币: " + std::to_string(PLAYER.get_coins()) + "\n"
            "获得体力: " + std::to_string(deltaEnergy) + "  获得经验: " + std::to_string(deltaExp) + (eventMsg.empty() ? "" : eventMsg)
        ));
    }
    catch (mongocxx::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("签到发生错误: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "签到发生错误!");
    }
}

// 查看背包前检查
bool preViewInventoryCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// 通用查看背包函数
std::string getInventoryStr(const LL &id) {
    auto tmp = allPlayers.at(id).get_inventory();
    if (tmp.size() == 0) {
        return "背包空空如也, 快去获取装备吧!";
    }
    else {
        std::string msg = "背包 (" + std::to_string(tmp.size()) + "/" + std::to_string(allPlayers.at(id).get_invCapacity()) + ")\n";
        LL index = 1;
        for (const auto &item : tmp) {
            if (allEquipments.at(item.id).type != EqiType::single_use)              // 非一次性物品
                msg += std::to_string(index) + "." + allEquipments.at(item.id).name + "+" + std::to_string(item.level) + " ";
            else                                                                    // 一次性物品
                msg += std::to_string(index) + ".[" + allEquipments.at(item.id).name + "] ";
            ++index;
        }
        if (!msg.empty())
            msg.pop_back();
        return msg;
    }
}

// 查看背包
void postViewInventoryCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_at(ev) + getInventoryStr(USER_ID));
    }
    catch (mongocxx::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("查看背包发生错误: ") + e.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看背包发生错误!");
    }
}

// 出售前检查
bool prePawnCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, std::vector<LL> &rtnItems) {
    if (!accountCheck(ev))
        return false;

    try {
        std::unordered_set<LL> sellList;
        for (const auto &item : args) {
            if (item.empty())
                continue;
            auto tmp = str_to_ll(item);                                         // 字符串转成整数
            if (tmp < 1 || tmp > PLAYER.get_inventory_size()) {                 // 检查是否超出背包范围
                cq::send_group_message(GROUP_ID, bg_at(ev) + "序号\"" + item + "\"超出了背包范围!");
                return false;
            }
            if (sellList.find(tmp) == sellList.end()) {                         // 检查是否有重复项目
                rtnItems.push_back(tmp - 1);                                    // 最后添加到返回列表 (注意内部列表以 0 为开头)
                sellList.insert(tmp);                                           // 记录该项目
            }
            else {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "序号\"" + item + "\"重复了!");
                return false;
            }
        }
        return true;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "输入格式错误! 请检查输入的都是有效的数字?");
        return false;
    }
}

// 出售
void postPawnCallback(const cq::MessageEvent &ev, std::vector<LL> &items) {
    try {
        std::sort(items.rbegin(), items.rend());            // 从大到小排序序号. 从后面往前删才不会出错

        double  price = 0;
        auto    inv = PLAYER.get_inventory();
        LL      prevIndex = static_cast<LL>(inv.size()) - 1;
        auto    it = inv.rbegin();
        for (const auto &index : items) {                   // 计算出售总价
            std::advance(it, prevIndex - index);
            price += allEquipments.at(it->id).price;
            prevIndex = index;
        }

        if (!PLAYER.remove_at_inventory(items)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "出售失败: 移除背包物品时发生错误!");
            return;
        }
        if (!PLAYER.inc_coins(static_cast<LL>(price))) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "出售失败: 添加硬币时发生错误!");
            return;
        }
        cq::send_group_message(GROUP_ID, bg_at(ev) + "成功出售" + std::to_string(items.size()) + "个物品, 获得" + std::to_string(static_cast<LL>(price)) + "硬币");
    }
    catch (std::exception &ex) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "出售失败! 错误原因: " + ex.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "出售失败!");
    }
}

// 查看属性前检查
bool preViewPropertiesCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// 懒人宏
// 获取某个属性的字符串. 比如获取攻击 (atk), 则字符串为 攻击值 (武器攻击 + 护甲攻击 + 饰品攻击)
#define GET_EQI_PROP_STR(prop)                                                                                  \
    std::setprecision(1) <<                                                                                     \
    allPlayers.at(id).get_##prop##() << " (" <<                                                                 \
    std::setprecision(0) <<                                                                                     \
    allPlayers.at(id).get_equipments().at(EqiType::weapon_primary).calc_##prop##() +      /* 武器属性总和*/      \
    allPlayers.at(id).get_equipments().at(EqiType::weapon_secondary).calc_##prop##()                            \
    << "+" <<                                                                                                   \
    allPlayers.at(id).get_equipments().at(EqiType::armor_helmet).calc_##prop##() +        /* 护甲属性总和*/      \
    allPlayers.at(id).get_equipments().at(EqiType::armor_body).calc_##prop##() +                                \
    allPlayers.at(id).get_equipments().at(EqiType::armor_leg).calc_##prop##() +                                 \
    allPlayers.at(id).get_equipments().at(EqiType::armor_boot).calc_##prop##()                                  \
    << "+" <<                                                                                                   \
    allPlayers.at(id).get_equipments().at(EqiType::ornament_earrings).calc_##prop##() +   /* 饰品属性总和*/      \
    allPlayers.at(id).get_equipments().at(EqiType::ornament_rings).calc_##prop##() +                            \
    allPlayers.at(id).get_equipments().at(EqiType::ornament_necklace).calc_##prop##() +                         \
    allPlayers.at(id).get_equipments().at(EqiType::ornament_jewelry).calc_##prop##()                            \
    << ")"

// 通用查看属性函数
std::string getPropertiesStr(const LL &id) {
    std::stringstream msg;

    msg << std::fixed << std::setprecision(1)
        << "账号: " << id << "\n"
        << "等级: " << allPlayers.at(id).get_level() << ", 祝福: " << allPlayers.at(id).get_blessing() << "\n"
        << "经验: " << allPlayers.at(id).get_exp() << "/" << allPlayers.at(id).get_exp_needed()
            << " (" << (double)allPlayers.at(id).get_exp() / (double)allPlayers.at(id).get_exp_needed() * 100.0 << "%)" << "\n"
        << "生命: " << GET_EQI_PROP_STR(hp) << "\n"
        << "攻击: " << GET_EQI_PROP_STR(atk) << "\n"
        << "防护: " << GET_EQI_PROP_STR(def) << "\n"
        << "魔力: " << GET_EQI_PROP_STR(mp) << "\n"
        << "暴击: " << GET_EQI_PROP_STR(crt) << "\n"
        << "破甲: " << GET_EQI_PROP_STR(brk) << "\n"
        << "敏捷: " << GET_EQI_PROP_STR(agi) << "\n"
        << "体力: " << allPlayers.at(id).get_energy() << " 硬币: " << allPlayers.at(id).get_coins() << "\n"
        << "英雄币: " << allPlayers.at(id).get_heroCoin();
    return msg.str();
}

// 查看属性
void postViewPropertiesCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_at(ev) + getPropertiesStr(USER_ID));
    }
    catch (std::exception &ex) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看属性失败! 错误原因: " + ex.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看属性失败!");
    }
}

// 查看装备前检查
bool preViewEquipmentsCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// 懒人宏
// 生成对应类型的装备的字符串. 若 ID = -1, 则显示"未装备"; 否则显示 装备名称+等级 (磨损/原始磨损)
#define GET_EQI_STR(eqitype)   \
    (##eqitype##.id == -1 ? "未装备" : allEquipments.at(##eqitype##.id).name + "+" + std::to_string(##eqitype##.level) +    \
    " (" + std::to_string(##eqitype##.wear) + "/" + std::to_string(allEquipments.at(##eqitype##.id).wear) + ")")

// 通用查看装备函数
std::string getEquipmentsStr(const LL &id) {
    const auto& eqi = allPlayers.at(id).get_equipments();
    const auto& singleUseEqi = allPlayers.at(id).get_equipItems();
    
    const auto& armor_helmet = eqi.at(EqiType::armor_helmet);
    const auto& armor_body = eqi.at(EqiType::armor_body);
    const auto& armor_leg = eqi.at(EqiType::armor_leg);
    const auto& armor_boot = eqi.at(EqiType::armor_boot);
    const auto& weapon_primary = eqi.at(EqiType::weapon_primary);
    const auto& weapon_secondary = eqi.at(EqiType::weapon_secondary);
    const auto& ornament_earrings = eqi.at(EqiType::ornament_earrings);
    const auto& ornament_rings = eqi.at(EqiType::ornament_rings);
    const auto& ornament_necklace = eqi.at(EqiType::ornament_necklace);
    const auto& ornament_jewelry = eqi.at(EqiType::ornament_jewelry);

    std::string eqiStr =
        "---护甲---\n"
        "头盔: " + GET_EQI_STR(armor_helmet) + ", " +
        "战甲: " + GET_EQI_STR(armor_body) + "\n" +
        "护腿: " + GET_EQI_STR(armor_leg) + ", " +
        "战靴: " + GET_EQI_STR(armor_boot) + "\n" +
        "---武器---\n"
        "主武器: " + GET_EQI_STR(weapon_primary) + ", " +
        "副武器: " + GET_EQI_STR(weapon_secondary) + "\n" +
        "---饰品---\n"
        "耳环: " + GET_EQI_STR(ornament_earrings) + ", " +
        "戒指: " + GET_EQI_STR(ornament_rings) + "\n" +
        "项链: " + GET_EQI_STR(ornament_necklace) + ", " +
        "宝石: " + GET_EQI_STR(ornament_jewelry);

    // 显示已装备的一次性物品
    if (!singleUseEqi.empty()) {
        LL index = 1;
        eqiStr += "\n---一次性---\n";
        for (const auto &item : singleUseEqi) {
            eqiStr += std::to_string(index) + "." + allEquipments[item.id].name + " ";
            ++index;
        }
        if (!eqiStr.empty())
            eqiStr.pop_back();
    }
    return eqiStr;
}

// 查看装备
void postViewEquipmentsCallback(const cq::MessageEvent &ev) {
    try {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "\n" + getEquipmentsStr(USER_ID));
    }
    catch (std::exception &ex) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看装备失败! 错误原因: " + ex.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看装备失败!");
    }
}

// 装备前检查
bool preEquipCallback(const cq::MessageEvent &ev, const std::string arg, LL &equipItem) {
    if (!accountCheck(ev))
        return false;

    try {
        equipItem = str_to_ll(arg) - 1;                                         // 字符串转成整数. 注意内部的存储序号由 0 开始
        if (equipItem > PLAYER.get_inventory_size()) {                          // 检查是否超出背包范围
            cq::send_group_message(GROUP_ID, bg_at(ev) + "序号\"" + arg + "\"超出了背包范围!");
            return false;
        }
        return true;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "输入格式错误! 请检查输入的都是有效的数字?");
        return false;
    }
}

// 装备
void postEquipCallback(const cq::MessageEvent &ev, const LL &equipItem) {
    PLAYER.resetCache();                                            // 重算玩家属性

    // 获取背包里该序号的对应物品
    auto invItems = PLAYER.get_inventory();
    auto it = invItems.begin();
    std::advance(it, equipItem);
    
    // 把装备上去了的装备从背包移除
    if (!PLAYER.remove_at_inventory({ equipItem })) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "装备时发生错误: 把将要装备的装备从背包中移除时发生错误!");
        return;
    }

    // 根据装备类型来装备
    auto eqiType = allEquipments.at((*it).id).type;
    if (eqiType != EqiType::single_use) {                               // 不是一次性物品
        // 把新装备装备上去
        if (!PLAYER.set_equipments_item(eqiType, *it)) { 
            cq::send_group_message(GROUP_ID, bg_at(ev) + "装备时发生错误: 修改玩家装备时发生错误!");
            return;
        }

        auto prevEquipItem = PLAYER.get_equipments_item(eqiType);       // 获取玩家之前装备的装备
        if (prevEquipItem.id != -1) {                                   // 如果玩家之前的装备不为空, 就放回背包中
            if (!PLAYER.add_inventory_item(prevEquipItem)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "装备时发生错误: 把之前的装备放回背包时发生错误!");
                return;
            }
        }
        cq::send_group_message(GROUP_ID, bg_at(ev) + "成功装备" + eqiType_to_str(eqiType) + ": " + allEquipments.at(it->id).name + "+" +
            std::to_string(it->level) + ", 磨损" + std::to_string(it->wear) + "/" + std::to_string(allEquipments.at(it->id).wear));
    }
    else {                                                              // 是一次性物品
        // 把物品添加到一次性列表
        if (!PLAYER.add_equipItems_item(*it)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "装备时发生错误: 修改玩家装备时发生错误!");
            return;
        }
        cq::send_group_message(GROUP_ID, bg_at(ev) + "成功装备一次性物品: " + allEquipments.at(it->id).name);
    }
}

// 把指定玩家的指定类型的装备卸下并放回包里.
// 如果卸下了装备, 则返回对应的名称; 否则返回空字符串
// 注意, 该函数不能处理卸下一次性物品
std::string unequipPlayer(const LL &qq, const EqiType &eqiType) {
    auto &p = allPlayers.at(qq);
    inventoryData eqi = p.get_equipments().at(eqiType);

    if (eqi.id == -1)
        return "";
    else {
        p.resetCache();
        if (!p.set_equipments_item(eqiType, { -1, -1, -1 })) {
            throw std::exception("设置玩家装备时失败!");
        }
        if (!p.add_inventory_item(eqi)) {
            throw std::exception("为玩家添加装备到背包时失败!");
        }

        return allEquipments.at(eqi.id).name + "+" + std::to_string(eqi.level);
    }
}

// 懒人宏
// 处理卸下指定装备
// 首先检查玩家是否有对应类型的装备, 然后卸下
// name: 回调函数名称, 如 Helmet; type: 装备类型, 如 armor_helmet; typestr: 装备类型的字串, 如 "头盔"
#define CALLBACK_UNEQUIP(name, type, typestr)                                                       \
    bool preUnequip ##name## Callback(const cq::MessageEvent &ev) {                                 \
        if (!accountCheck(ev))                                                                      \
            return false;                                                                           \
        if (PLAYER.get_equipments().at(EqiType::##type##).id == -1) {                               \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "目前没有装备" + typestr + "哦!");          \
            return false;                                                                           \
        }                                                                                           \
        return true;                                                                                \
    }                                                                                               \
                                                                                                    \
    void postUnequip##name##Callback(const cq::MessageEvent &ev) {                                  \
        try {                                                                                       \
            PLAYER.resetCache();                                                                    \
            auto rtn = unequipPlayer(USER_ID, EqiType::##type##);                                   \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "已卸下" + rtn);                           \
        }                                                                                           \
        catch (std::exception &ex) {                                                                \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败! 错误原因: " + ex.what());    \
        }                                                                                           \
        catch (...) {                                                                               \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败!");                           \
        }                                                                                           \
    }

CALLBACK_UNEQUIP(Helmet,    armor_helmet,       "头盔");
CALLBACK_UNEQUIP(Body,      armor_body,         "战甲");
CALLBACK_UNEQUIP(Leg,       armor_leg,          "护腿");
CALLBACK_UNEQUIP(Boot,      armor_boot,         "战靴");
CALLBACK_UNEQUIP(Primary,   weapon_primary,     "主武器");
CALLBACK_UNEQUIP(Secondary, weapon_secondary,   "副武器");
CALLBACK_UNEQUIP(Earrings,  ornament_earrings,  "耳环");
CALLBACK_UNEQUIP(Rings,     ornament_rings,     "戒指");
CALLBACK_UNEQUIP(Necklace,  ornament_necklace,  "项链");
CALLBACK_UNEQUIP(Jewelry,   ornament_jewelry,   "宝石");

// 懒人宏
// 生成卸下装备的字符串
#define UNEQUIP(type)                                       \
    rtn = unequipPlayer(USER_ID, EqiType::##type##);        \
    if (!rtn.empty())                                       \
        msg += rtn + ", ";

// 卸下护甲前检查
bool preUnequipArmorCallback(const cq::MessageEvent & ev) {
    return accountCheck(ev);
}

// 卸下护甲
void postUnequipArmorCallback(const cq::MessageEvent &ev) {
    try {
        PLAYER.resetCache();                    // 重算玩家属性

        std::string msg = "";
        auto UNEQUIP(armor_body);
        UNEQUIP(armor_boot);
        UNEQUIP(armor_helmet);
        UNEQUIP(armor_leg);

        if (msg.empty()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "目前没有装备任何护甲");
        }
        else {
            msg.pop_back();                     // 去掉结尾的 ", "
            msg.pop_back();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "已卸下" + msg);
        }
    }
    catch (std::exception &ex) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败! 错误原因: " + ex.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败!");
    }
}

// 卸下武器前检查
bool preUnequipWeaponCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// 卸下武器
void postUnequipWeaponCallback(const cq::MessageEvent &ev) {
    try {
        PLAYER.resetCache();                    // 重算玩家属性

        std::string msg = "";
        auto UNEQUIP(weapon_primary);
        UNEQUIP(weapon_secondary);

        if (msg.empty()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "目前没有装备任何武器");
        }
        else {
            msg.pop_back();                     // 去掉结尾的 ", "
            msg.pop_back();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "已卸下" + msg);
        }
    }
    catch (std::exception &ex) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败! 错误原因: " + ex.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败!");
    }
}

// 卸下饰品前检查
bool preUnequipOrnamentCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// 卸下饰品
void postUnequipOrnamentCallback(const cq::MessageEvent &ev) {
    try {
        PLAYER.resetCache();                    // 重算玩家属性

        std::string msg = "";
        auto UNEQUIP(ornament_earrings);
        UNEQUIP(ornament_jewelry);
        UNEQUIP(ornament_necklace);
        UNEQUIP(ornament_rings);

        if (msg.empty()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "目前没有装备任何饰品");
        }
        else {
            msg.pop_back();                     // 去掉结尾的 ", "
            msg.pop_back();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "已卸下" + msg);
        }
    }
    catch (std::exception &ex) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败! 错误原因: " + ex.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败!");
    }
}

// 卸下所有装备前检查
bool preUnequipAllCallback(const cq::MessageEvent &ev) {
    return accountCheck(ev);
}

// 卸下所有装备
void postUnequipAllCallback(const cq::MessageEvent &ev) {
    try {
        PLAYER.resetCache();                    // 重算玩家属性

        std::string msg = "";
        auto UNEQUIP(armor_body);
        UNEQUIP(armor_boot);
        UNEQUIP(armor_helmet);
        UNEQUIP(armor_leg);
        UNEQUIP(weapon_primary);
        UNEQUIP(weapon_secondary);
        UNEQUIP(ornament_earrings);
        UNEQUIP(ornament_jewelry);
        UNEQUIP(ornament_necklace);
        UNEQUIP(ornament_rings);

        auto items = PLAYER.get_equipItems();
        if (!PLAYER.clear_equipItems()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下物品失败: 清空一次性装备时发生错误!");
            return;
        }
        for (const auto &item : items) {
            if (!PLAYER.add_inventory_item(item)) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下物品失败: 把一次性装备添加到背包时发生错误!");
                return;
            }
            msg += allEquipments.at(item.id).name + ", ";
        }

        if (msg.empty()) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "目前没有装备任何装备");
        }
        else {
            msg.pop_back();                     // 去掉结尾的 ", "
            msg.pop_back();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "已卸下" + msg);
        }
    }
    catch (std::exception &ex) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败! 错误原因: " + ex.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备失败!");
    }
}

// 卸下指定物品前检查
bool preUnequipSingleCallback(const cq::MessageEvent &ev, const std::string arg, LL &unequipItem) {
    if (!accountCheck(ev))
        return false;
    
    try {
        auto tmp = str_to_ll(arg);                                              // 字符串转成整数
        if (tmp < 1 || tmp > PLAYER.get_equipItems_size()) {                    // 检查序号是否超出装备范围
            cq::send_group_message(GROUP_ID, bg_at(ev) + "没有找到序号为\"" + std::to_string(tmp) + "\"的物品!");
            return false;
        }
        unequipItem = tmp - 1;                                                  // 注意内部列表以 0 为开头
        return true;
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "输入格式错误! 请检查输入的都是有效的数字?");
        return false;
    }
}

// 卸下指定物品
void postUnequipSingleCallback(const cq::MessageEvent &ev, const LL &unequipItem) {
    try {
        PLAYER.resetCache();                    // 重算玩家属性

        // 先把要卸下的装备记录下来, 方便之后添加到背包
        auto items = PLAYER.get_equipItems();
        auto it = items.begin();
        std::advance(it, unequipItem);

        if (!PLAYER.remove_at_equipItems(unequipItem)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下物品失败: 移除装备时发生错误!");
            return;
        }
        if (!PLAYER.add_inventory_item(*it)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下物品失败: 把装备添加到背包时发生错误!");
            return;
        }
        cq::send_group_message(GROUP_ID, bg_at(ev) + "成功卸下一次性物品: " + allEquipments.at(it->id).name);
    }
    catch (std::exception &ex) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下物品失败! 错误原因: " + ex.what());
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下物品失败!");
    }
}
