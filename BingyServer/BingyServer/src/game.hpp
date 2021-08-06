/*
描述: Bingy 游戏相关函数的接口
作者: 冰棍
文件: game.hpp
*/

#pragma once

#include <unordered_set>
#include <set>
#include "inventory.hpp"
#include "http_handlers.hpp"

// Bingy 游戏部分的 HTTP 请求内容
typedef struct _bgGameHttpReq {
    http_req    *req;           // 请求
    LL          playerId;       // 玩家 QQ 号
    LL          groupId;        // 群号
} bgGameHttpReq;

bool accountCheck(const bgGameHttpReq &req);

// 懒人宏
// 定义 pre*Callback 和 post*Callback
#define PRE_POST(name)                                          \
    bool pre ##name## Callback(const bgGameHttpReq &bgReq);     \
    void post ##name## Callback(const bgGameHttpReq &bgReq);    \

PRE_POST(Register);
PRE_POST(ViewCoins);
PRE_POST(SignIn);
PRE_POST(ViewInventory);
bool prePawnCallback(const bgGameHttpReq &bgReq, const std::vector<LL> &items);
void postPawnCallback(const bgGameHttpReq &bgReq, const std::vector<LL> &items);
PRE_POST(ViewProperties);
PRE_POST(ViewEquipments);
bool preEquipCallback(const bgGameHttpReq &bgReq, const LL &equipItem);
void postEquipCallback(const bgGameHttpReq &bgReq, const LL &equipItem);
bool preUnequipCallback(const bgGameHttpReq &bgReq, const EqiType &type);
void postUnequipCallback(const bgGameHttpReq &bgReq, const EqiType &type);
PRE_POST(UnequipWeapon);
PRE_POST(UnequipArmor);
PRE_POST(UnequipOrnament);
PRE_POST(UnequipAll);
bool preUnequipSingleCallback(const bgGameHttpReq &bgReq, const LL &index);
void postUnequipSingleCallback(const bgGameHttpReq &bgReq, const LL &index);
bool preUpgradeCallback(const bgGameHttpReq &bgReq, const EqiType &type, LL &upgradeTimes, LL &coinsNeeded);
void postUpgradeCallback(const bgGameHttpReq &bgReq, const EqiType &type, const LL &upgradeTimes, const LL &coinsNeeded);
PRE_POST(ConfirmUpgrade);
PRE_POST(ViewTrade);
bool preBuyTradeCallback(const bgGameHttpReq &bgReq, const LL &tradeId, const std::string &password);
void postBuyTradeCallback(const bgGameHttpReq &bgReq, const LL &tradeId);
bool preSellTradeCallback(const bgGameHttpReq &bgReq, const LL &invId, const LL &price, const bool &hasPassword);
void postSellTradeCallback(const bgGameHttpReq &bgReq, const LL &invId, const LL &price, const bool &hasPassword);
bool preRecallTradeCallback(const bgGameHttpReq &bgReq, const LL &tradeId);
void postRecallTradeCallback(const bgGameHttpReq &bgReq, const LL &tradeId);
bool preSynthesisCallback(const bgGameHttpReq &bgReq, const std::set<LL, std::greater<LL>> &invList, const std::string &target, LL &targetId, LL &coins, LL &level);
void postSynthesisCallback(const bgGameHttpReq &bgReq, const std::set<LL, std::greater<LL>> &invList, const LL &targetId, const LL &coins, const LL &level);
bool preFightCallback(const bgGameHttpReq &bgReq, const LL &dungeonLevel);
void postFightCallback(const bgGameHttpReq &bgReq, const LL &dungeonLevel);
bool preAdminModifyFieldCallback(const bgGameHttpReq &bgReq, unsigned char fieldType, unsigned char mode, const LL &targetId, const LL &val);
void postAdminModifyFieldCallback(const bgGameHttpReq &bgReq, unsigned char fieldType, unsigned char mode, const LL &targetId, const LL &val);
