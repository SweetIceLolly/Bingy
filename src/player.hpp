/*
描述: Bingy 玩家相关操作的接口
作者: 冰棍
文件: player.hpp
*/

#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <list>
#include "inventory.hpp"
#include "equipment.hpp"

#define INV_DEFAULT_CAPACITY 50

using LL = long long;

class player {
private:
    std::mutex mutexPlayer;                                                         // 玩家操作锁
    LL id;                                                                          // QQ 号

public:
    // [注意] 以下属性请使用对应的 getter 和 setter. 除非你知道你在做什么, 否则不要直接读写他们的值
    std::string nickname;                   bool nickname_cache = false;            // 昵称 (请使用对应的 getter 或 setter)
    LL signInCount;                         bool signInCount_cache = false;         // 签到次数 (请使用对应的 getter 或 setter)
    LL signInCountCont;                     bool signInCountCont_cache = false;     // 连续签到次数 (请使用对应的 getter 或 setter)
    LL lastFight;                           bool lastFight_cache = false;           // 上一次打怪时间. 为 UNIX 时间戳, 单位为秒 (请使用对应的 getter 或 setter)
    LL lastSignIn;                          bool lastSignIn_cache = false;          // 上一次签到时间. 为 UNIX 时间戳, 单位为秒 (请使用对应的 getter 或 setter)
    LL coins;                               bool coins_cache = false;               // 硬币数 (请使用对应的 getter 或 setter)
    LL heroCoin;                            bool heroCoin_cache = false;            // 英雄币数 (请使用对应的 getter 或 setter)
    LL level;                               bool level_cache = false;               // 等级 (请使用对应的 getter 或 setter)
    LL energy;                              bool energy_cache = false;              // 体力 (请使用对应的 getter 或 setter)
    LL exp;                                 bool exp_cache = false;                 // 经验数 (请使用对应的 getter 或 setter)
    LL invCapacity;                         bool invCapacity_cache = false;         // 背包容量 (请使用对应的 getter 或 setter)
    std::list<inventoryData> inventory;     bool inventory_cache = false;           // 背包 (请使用对应的 getter 或 setter)
    std::unordered_map<LL, LL> buyCount;    bool buyCount_cache = false;            // 商品购买次数 (商品 ID -> 次数) (请使用对应的操作函数)
    std::unordered_map<EqiType,
        inventoryData> equipments;          bool equipments_cache = false;          // 已装备的装备 (装备类型 -> inventoryData) (请使用对应的操作函数)
    std::list<inventoryData> equipItems;    bool equipItems_cache = false;          // 已装备的一次性物品 (请使用对应的操作函数)
    LL vip;                                 bool vip_cache = false;                 // VIP (请使用对应的 getter 或 setter)

    // ---------------------------------------------------------

    // 不允许使用默认构造函数
    player();

    // 复制构造函数
    player(const player &p);

    // 指定 QQ 号的构造函数
    player(const LL &qq);

    // ---------------------------------------------------------

    LL getId();

    std::string getNickname(bool use_cache = true);
    bool player::setNickname(const std::string &val);

    LL player::getCoins(bool use_cache = true);
    bool player::setCoins(const LL &val);
    bool player::incCoins(const LL &val);
};

extern std::unordered_map<long long, player>   allPlayers;
extern std::mutex                              mutexAllPlayers;

bool bg_player_exist(const LL &id);
bool bg_player_add(const LL &id);
bool bg_get_allplayers_from_db();
