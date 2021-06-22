/*
描述: Bingy 游戏相关函数的接口
作者: 冰棍
文件: game.hpp
*/

#pragma once

#include <cqcppsdk/cqcppsdk.hpp>
#include <unordered_set>
#include "equipment.hpp"

using LL = long long;

extern std::unordered_set<LL>  blacklist;
extern std::unordered_set<LL>  allAdmins;

std::string bg_at(const cq::MessageEvent &ev);
bool accountCheck(const cq::MessageEvent &ev);

// 懒人宏
// 定义 pre*Callback 和 post*Callback
#define PRE_POST(name)                                      \
    bool pre##name##Callback(const cq::MessageEvent &ev);   \
    void post##name##Callback(const cq::MessageEvent &ev);  \

// 懒人宏
// 定义管理命令 admin*Callback
#define ADMIN(name)                                         \
    void admin##name##Callback(const cq::MessageEvent &ev, const std::string &arg);

// 懒人宏
#define GROUP_ID    ev.target.group_id.value()      // 从 ev 获取群号
#define USER_ID     ev.target.user_id.value()       // 从 ev 获取玩家 QQ 号
#define PLAYER      allPlayers.at(USER_ID)          // 线程安全地获取 QQ 号对应的玩家

PRE_POST(Register);
PRE_POST(ViewCoins);
PRE_POST(SignIn);
PRE_POST(ViewInventory);
bool prePawnCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, std::vector<LL> &rtnItems);
void postPawnCallback(const cq::MessageEvent &ev, std::vector<LL> &items);
PRE_POST(ViewProperties);
PRE_POST(ViewEquipments);
bool preEquipCallback(const cq::MessageEvent &ev, const std::string &arg, LL &equipItem);
void postEquipCallback(const cq::MessageEvent &ev, const LL &equipItem);
PRE_POST(UnequipHelmet);
PRE_POST(UnequipBody);
PRE_POST(UnequipLeg);
PRE_POST(UnequipBoot);
PRE_POST(UnequipArmor);
PRE_POST(UnequipPrimary);
PRE_POST(UnequipSecondary);
PRE_POST(UnequipWeapon);
PRE_POST(UnequipEarrings);
PRE_POST(UnequipRings);
PRE_POST(UnequipNecklace);
PRE_POST(UnequipJewelry);
PRE_POST(UnequipOrnament);
bool preUnequipSingleCallback(const cq::MessageEvent &ev, const std::string &arg, LL &unequipItem);
void postUnequipSingleCallback(const cq::MessageEvent &ev, const LL &unequipItem);
PRE_POST(UnequipAll);
bool preUpgradeCallback(const cq::MessageEvent &ev, const EqiType &eqiType, const std::string &arg, LL &upgradeTimes, LL &coinsNeeded);
void postUpgradeCallback(const cq::MessageEvent &ev, const EqiType &eqiType, const LL &upgradeTimes, const LL &coinsNeeded);
PRE_POST(ConfirmUpgrade);

ADMIN(AddCoins);
ADMIN(AddHeroCoin);
ADMIN(AddLevel);
ADMIN(AddBlessing);
ADMIN(AddEnergy);
ADMIN(AddExp);
ADMIN(AddInvCapacity);
ADMIN(AddVip);
