/*
描述: Bingy 游戏相关函数的接口
作者: 冰棍
文件: game.hpp
*/

#pragma once

#include <cqcppsdk/cqcppsdk.hpp>
#include "equipment.hpp"

#define DEFAULT_SERVER_URI  "http://127.0.0.1:8000"     // 默认服务器地址

extern std::string serverUri;                           // 服务器地址
extern std::string appId;                               // 应用 ID
extern std::string appSecret;                           // 应用密匙

using LL = long long;

// 取得艾特玩家字符串
std::string bg_at(const cq::MessageEvent &ev);

// 懒人宏
// 定义管理命令 admin*Callback
#define ADMIN(name)                                         \
    void admin##name##Callback(const cq::MessageEvent &ev, const std::string &arg)

// 懒人宏
#define GROUP_ID    ev.target.group_id.value()      // 从 ev 获取群号
#define USER_ID     ev.target.user_id.value()       // 从 ev 获取玩家 QQ 号

void registerCallback(const cq::MessageEvent &ev);
void viewCoinsCallback(const cq::MessageEvent &ev);
void signInCallback(const cq::MessageEvent &ev);
void viewInventoryCallback(const cq::MessageEvent &ev);
void pawnCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args);
void viewPropertiesCallback(const cq::MessageEvent &ev);
void viewEquipmentsCallback(const cq::MessageEvent &ev);
void searchEquipmentsCallback(const cq::MessageEvent &ev, const std::string &arg);
void equipCallback(const cq::MessageEvent &ev, const std::string &arg);
void unequipHelmetCallback(const cq::MessageEvent &ev);
void unequipBodyCallback(const cq::MessageEvent &ev);
void unequipLegCallback(const cq::MessageEvent &ev);
void unequipBootCallback(const cq::MessageEvent &ev);
void unequipArmorCallback(const cq::MessageEvent &ev);
void unequipPrimaryCallback(const cq::MessageEvent &ev);
void unequipSecondaryCallback(const cq::MessageEvent &ev);
void unequipWeaponCallback(const cq::MessageEvent &ev);
void unequipEarringsCallback(const cq::MessageEvent &ev);
void unequipRingsCallback(const cq::MessageEvent &ev);
void unequipNecklaceCallback(const cq::MessageEvent &ev);
void unequipJewelryCallback(const cq::MessageEvent &ev);
void unequipOrnamentCallback(const cq::MessageEvent &ev);
void unequipSingleCallback(const cq::MessageEvent &ev, const std::string &arg);
void unequipAllCallback(const cq::MessageEvent &ev);
void upgradeCallback(const cq::MessageEvent &ev, const EqiType &eqiType, const std::string &arg);
void confirmUpgradeCallback(const cq::MessageEvent &ev);
void upgradeBlessingCallback(const cq::MessageEvent &ev, const std::string &arg);
void viewTradeCallback(const cq::MessageEvent &ev);
void buyTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args);
void sellTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args);
void recallTradeCallback(const cq::MessageEvent &ev, const std::string &arg);
void synthesisCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args);
void fightCallback(const cq::MessageEvent &ev, const std::string &arg);
void chatCallback(const cq::MessageEvent &ev);

ADMIN(AddCoins);
ADMIN(AddHeroCoin);
ADMIN(AddLevel);
ADMIN(AddBlessing);
ADMIN(AddEnergy);
ADMIN(AddExp);
ADMIN(AddInvCapacity);
ADMIN(AddVip);
ADMIN(SetCoins);
ADMIN(SetHeroCoin);
ADMIN(SetLevel);
ADMIN(SetBlessing);
ADMIN(SetEnergy);
ADMIN(SetExp);
ADMIN(SetInvCapacity);
ADMIN(SetVip);