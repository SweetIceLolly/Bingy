/*
����: Bingy ��Ϸ��غ����Ľӿ�
����: ����
�ļ�: game.hpp
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

// ���˺�
// ���� pre*Callback �� post*Callback
#define PRE_POST(name)                                      \
    bool pre##name##Callback(const cq::MessageEvent &ev);   \
    void post##name##Callback(const cq::MessageEvent &ev);  \

// ���˺�
// ����������� admin*Callback
#define ADMIN(name)                                         \
    void admin##name##Callback(const cq::MessageEvent &ev, const std::string &arg);

// ���˺�
#define GROUP_ID    ev.target.group_id.value()      // �� ev ��ȡȺ��
#define USER_ID     ev.target.user_id.value()       // �� ev ��ȡ��� QQ ��
#define PLAYER      allPlayers.at(USER_ID)          // �̰߳�ȫ�ػ�ȡ QQ �Ŷ�Ӧ�����

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
PRE_POST(ViewTrade);
bool preBuyTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, LL &tradeId);
void postBuyTradeCallback(const cq::MessageEvent &ev, const LL &tradeId);
bool preSellTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, LL &invId, bool &hasPassword, LL &price);
void postSellTradeCallback(const cq::MessageEvent &ev, const LL &invId, const bool &hasPassword, const LL &price);
bool preRecallTradeCallback(const cq::MessageEvent &ev, const std::string &arg, LL &tradeId);
void postRecallTradeCallback(const cq::MessageEvent &ev, const LL &tradeId);
bool preSynthesisCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args, std::set<LL, std::greater<LL>> &invList, LL &targetId, LL &coins, LL &level);
void postSynthesisCallback(const cq::MessageEvent &ev, const std::set<LL, std::greater<LL>> &invList, const LL &targetId, const LL &coins, const LL &level);
bool preFightCallback(const cq::MessageEvent &ev, const std::string &arg, LL &dungeonLevel);
void postFightCallback(const cq::MessageEvent &ev, const LL &dungeonLevel);

ADMIN(AddCoins);
ADMIN(AddHeroCoin);
ADMIN(AddLevel);
ADMIN(AddBlessing);
ADMIN(AddEnergy);
ADMIN(AddExp);
ADMIN(AddInvCapacity);
ADMIN(AddVip);
