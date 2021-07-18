/*
描述: Bingy 玩家相关操作的接口
作者: 冰棍
文件: player.hpp
*/

#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <list>
#include <vector>
#include "inventory.hpp"
#include "equipment.hpp"

#define INV_DEFAULT_CAPACITY 50                                                     // 默认背包容量

// 懒人宏
// 定义 LL 类型的属性的 getter, setter, 和 inc (增加数值)的函数原型
#define DEF_LL_GET_SET_INC(propName)                            \
    LL get_ ##propName (const bool &use_cache = true);          \
    bool set_ ##propName (const LL &val);                       \
    bool inc_ ##propName (const LL &val);                       \

// 玩家列表锁, 防止出现一条线程添加玩家, 另一条线程读取玩家的情况
#define LOCK_PLAYERS_LIST std::unique_lock _all_players_lock(mutexAllPlayers);

// 解锁玩家列表锁
#define UNLOCK_PLAYERS_LIST _all_players_lock.unlock();

using LL = std::int64_t;

class player {
private:
    LL id;                                                                          // QQ 号

    // 多次强化的确认状态
    std::mutex mutexStatus;                                                         // 线程状态锁
    bool upgrading;                                                                 // 玩家是否确认要强化
    std::condition_variable cvStatusChange;                                         // 状态发生变化
    std::condition_variable cvPrevConfirmCompleted;                                 // 上一次操作是否完成

public:
    std::mutex mutexPlayer;                                                         // 玩家操作锁

    // [注意] 以下属性请使用对应的 getter 和 setter. 除非你知道你在做什么, 否则不要直接读写他们的值
    std::string nickname;                   bool nickname_cache = false;            // 昵称 (请使用对应的 getter 或 setter)
    LL signInCount;                         bool signInCount_cache = false;         // 签到次数 (请使用对应的 getter 或 setter)
    LL signInCountCont;                     bool signInCountCont_cache = false;     // 连续签到次数 (请使用对应的 getter 或 setter)
    LL lastFight;                           bool lastFight_cache = false;           // 上一次打怪时间. 为 UNIX 时间戳, 单位为秒 (请使用对应的 getter 或 setter)
    LL lastSignIn;                          bool lastSignIn_cache = false;          // 上一次签到时间. 为 UNIX 时间戳, 单位为秒 (请使用对应的 getter 或 setter)
    LL coins;                               bool coins_cache = false;               // 硬币数 (请使用对应的 getter 或 setter)
    LL heroCoin;                            bool heroCoin_cache = false;            // 英雄币数 (请使用对应的 getter 或 setter)
    LL level;                               bool level_cache = false;               // 等级 (请使用对应的 getter 或 setter)
    LL blessing;                            bool blessing_cache = false;            // 祝福 (请使用对应的 getter 或 setter)
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
    bool set_nickname(const std::string &val);

    DEF_LL_GET_SET_INC(signInCount);
    DEF_LL_GET_SET_INC(signInCountCont);
    DEF_LL_GET_SET_INC(lastFight);
    DEF_LL_GET_SET_INC(lastSignIn);
    DEF_LL_GET_SET_INC(coins);
    DEF_LL_GET_SET_INC(heroCoin);
    DEF_LL_GET_SET_INC(level);
    DEF_LL_GET_SET_INC(blessing);
    DEF_LL_GET_SET_INC(energy);
    DEF_LL_GET_SET_INC(exp);
    DEF_LL_GET_SET_INC(invCapacity);
    DEF_LL_GET_SET_INC(vip);

    // 获取玩家属性
    bool atk_cache = false;
    double get_atk();           // 攻

    bool def_cache = false;
    double get_def();           // 防

    bool brk_cache = false;
    double get_brk();           // 破

    bool agi_cache = false;
    double get_agi();           // 敏

    bool hp_cache = false;
    double get_hp();            // 血

    bool mp_cache = false;
    double get_mp();            // 魔

    bool crt_cache = false;
    double get_crt();           // 暴

    LL get_exp_needed();        // 升级所需经验
    LL get_cd();                // 冷却时间

    void resetCache();

    // 获取整个背包列表
    std::list<inventoryData> get_inventory(const bool &use_cache = true);
    // 获取背包装备数量
    LL get_inventory_size(const bool &use_cache = true);
    // 按照指定序号移除背包物品. 如果指定序号无效, 则返回 false. 注意, 指定序号必须从 0 开始
    bool remove_at_inventory(const LL &index);
    // 按照指定的序号列表移除背包物品. 指定的序号不得重复. 如果指定序号无效, 则返回 false. 注意, 指定序号必须从 0 开始
    bool remove_at_inventory(const std::vector<LL> &indexes);
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

    // 获取整个已装备的装备表
    std::unordered_map<EqiType, inventoryData> get_equipments(const bool &use_cache = true);
    // 获取某个类型的装备
    inventoryData get_equipments_item(const EqiType &type, const bool &use_cache = true);
    // 设置某个类型的装备. 如果要移除, 则把 item 的 id 设置为 -1
    bool set_equipments_item(const EqiType &type, const inventoryData &item);

    // 获取整个已装备的一次性物品表
    std::list<inventoryData> get_equipItems(const bool &use_cache = true);
    // 获取已装备的一次性物品数量
    LL get_equipItems_size(const bool &use_cache = true);
    // 移除某个已装备的一次性物品. 如果指定序号无效, 则返回 false. 注意, 指定序号必须从 0 开始
    bool remove_at_equipItems(const LL &index);
    // 清空已装备的一次性物品
    bool clear_equipItems();
    // 添加新物品到已装备的一次性物品列表末尾
    bool add_equipItems_item(const inventoryData &item);

    bool confirmInProgress = false;                                                 // 是否有待确认的强化
    // 取消强化确认
    void abortUpgrade();
    // 确认强化确认
    void confirmUpgrade();
    // 等待强化确认. 如果玩家确认了强化, 就返回 true; 否则返回 false
    bool waitUpgradeConfirm();
    // 等待确认完成
    void waitConfirmComplete();
};

extern std::unordered_map<std::int64_t, player>     allPlayers;
extern std::mutex                                   mutexAllPlayers;

bool bg_player_exist(const LL &id);
bool bg_player_add(const LL &id);
bool bg_get_all_players_from_db();

bool bg_all_player_inc_coins(const LL &val);
bool bg_all_player_inc_heroCoin(const LL &val);
bool bg_all_player_inc_level(const LL &val);
bool bg_all_player_inc_blessing(const LL &val);
bool bg_all_player_inc_energy(const LL &val);
bool bg_all_player_inc_exp(const LL &val);
bool bg_all_player_inc_invCapacity(const LL &val);
bool bg_all_player_inc_vip(const LL &val);
