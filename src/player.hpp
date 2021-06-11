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

#define DEF_LL_GET_SET_INC(propName)                            \
    LL player::get_##propName##(const bool &use_cache = true);  \
    bool player::set_##propName##(const LL &val);               \
    bool player::inc_##propName##(const LL &val);               \

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

    // 默认构造函数
    player();

    // 复制构造函数
    player(const player &p);

    // 指定 QQ 号的构造函数
    player(const LL &qq);

    // ---------------------------------------------------------

    LL get_id();

    std::string get_nickname(const bool &use_cache = true);
    bool player::set_nickname(const std::string &val);

    DEF_LL_GET_SET_INC(signInCount);
    DEF_LL_GET_SET_INC(signInCountCont);
    DEF_LL_GET_SET_INC(lastFight);
    DEF_LL_GET_SET_INC(lastSignIn);
    DEF_LL_GET_SET_INC(coins);
    DEF_LL_GET_SET_INC(heroCoin);
    DEF_LL_GET_SET_INC(level);
    DEF_LL_GET_SET_INC(energy);
    DEF_LL_GET_SET_INC(exp);
    DEF_LL_GET_SET_INC(invCapacity);
    DEF_LL_GET_SET_INC(vip);

    // 获取整个背包列表
    std::list<inventoryData> get_inventory(const bool &use_cache = true);
    // 按照指定序号获取背包物品. 如果指定序号无效, 则返回 false
    bool get_inventory_item(const LL &index, inventoryData &item, const bool &use_cache = true);
    // 按照指定序号移除背包物品. 如果指定序号无效, 则返回 false
    bool remove_at_inventory(const LL &index);
    // 添加新物品到背包末尾
    bool add_inventory_item(const inventoryData &item);
    // 设置整个背包列表
    bool set_inventory(const std::list<inventoryData> &val);

    // 获取整个购买次数表
    std::unordered_map<LL, LL> get_buyCount(const bool &use_cache = true);
    // 获取购买次数表中某个商品的购买次数. 如果找不到对应的商品购买记录, 则返回 0
    LL get_buyCount_item(const LL &id, const bool &use_cache = true);
    // 设置购买次数表中某个商品的购买次数. 如果对应商品的购买记录不存在, 则会创建
    bool set_buyCount_item(const LL &id, const LL &count);
    // 设置整个购买次数表
    bool set_buyCount(const std::unordered_map<LL, LL> &val);

    // 获取整个已装备的装备表
    std::unordered_map<EqiType, inventoryData> get_equipments(const bool &use_cache = true);
    // 获取某个类型的装备
    inventoryData get_equipments_item(const EqiType &type, const bool &use_cache = true);
    // 设置某个类型的装备. 如果要移除, 则把 item 的 id 设置为 -1
    bool set_equipments_item(const EqiType &type, const inventoryData &item);
    // 设置整个已装备的装备表
    bool set_equipments(const std::unordered_map<EqiType, inventoryData> &val);

    // 获取整个已装备的一次性物品表
    std::list<inventoryData> get_equipItems(const bool &use_cache = true);
    // 获取某个已装备的一次性物品. 如果指定序号无效, 则返回 false
    bool get_equipItems_item(const LL &index,  const bool &use_cache = true);
    // 移除某个已装备的一次性物品. 如果指定序号无效, 则返回 false
    bool get_equipItems_item(const LL &index, inventoryData &item, const bool &use_cache = true);
    // 添加新物品到已装备的一次性物品列表末尾
    bool add_equipItems_item(const inventoryData &item);
    // 设置整个已装备的一次性物品列表
    bool set_equipItems(const std::list<inventoryData> &val);
};

extern std::unordered_map<long long, player>   allPlayers;
extern std::mutex                              mutexAllPlayers;

bool bg_player_exist(const LL &id);
bool bg_player_add(const LL &id);
bool bg_get_allplayers_from_db();
