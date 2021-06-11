/*
描述: Bingy 玩家相关操作
作者: 冰棍
文件: player.cpp
*/

#include "player.hpp"
#include "database.hpp"

#define LOCK_CURR_PLAYER std::scoped_lock<std::mutex> __lock(this->mutexPlayer);

std::unordered_map<LL, player>  allPlayers;         // 注意: 读取的时候可以不用加锁, 但是不要使用[], 需要使用 at(). 多线程写入的时候必须加锁
std::mutex                      mutexAllPlayers;

// --------------------------------------------------
// 构造函数

player::player() {
    throw "创建 player 对象时必须指定 QQ 号!";
}

player::player(const player &p) {
    this->id = p.id;
    this->nickname = p.nickname;
	this->nickname_cache = p.nickname_cache;
    this->signInCount = p.signInCount;
	this->signInCount_cache = p.signInCount_cache;
    this->signInCountCont = p.signInCountCont;
	this->signInCountCont_cache = p.signInCountCont_cache;
    this->lastFight = p.lastFight;
	this->lastFight_cache = p.lastFight_cache;
    this->lastSignIn = p.lastSignIn;
	this->lastSignIn_cache = p.lastSignIn_cache;
    this->coins = p.coins;
	this->coins_cache = p.coins_cache;
    this->heroCoin = p.heroCoin;
	this->heroCoin_cache = p.heroCoin_cache;
    this->level = p.level;
	this->level_cache = p.level_cache;
    this->energy = p.energy;
	this->energy_cache = p.energy_cache;
    this->exp = p.exp;
	this->exp_cache = p.exp_cache;
    this->invCapacity = p.invCapacity;
	this->invCapacity_cache = p.invCapacity_cache;
    this->inventory = p.inventory;
    this->inventory_cache = p.inventory_cache;
    this->vip = p.vip;
	this->vip_cache = p.vip_cache;
    this->equipments = p.equipments;
    this->equipments_cache = p.equipments_cache;
    this->equipItems = p.equipItems;
    this->equipItems_cache = p.equipItems_cache;
}

player::player(const LL &qq) {
    this->id = qq;
    this->nickname = "";
    this->signInCount = 0;
    this->signInCountCont = 0;
    this->lastFight = 0;
    this->lastSignIn = 0;
    this->coins = 0;
    this->heroCoin = 0;
    this->level = 1;
    this->energy = 0;
    this->exp = 0;
    this->invCapacity = INV_DEFAULT_CAPACITY;
    this->vip = 0;
}

// --------------------------------------------------
// 玩家操作函数

// 检查玩家是否存在
bool bg_player_exist(const LL &id) {
    auto val = dbFindOne(DB_COLL_USERDATA, "id", id);
    return allPlayers.end() != allPlayers.find(id);
}

// 懒人宏
// 把玩家属性中 LL 类型的数据设置为0, 并把对应的缓存标识设置为 true
#define SET_LL_PROP_ZERO(prop) p.##prop = 0; p.##prop##_cache = true;

// 无条件添加玩家到数据库和字典中. 抛出的异常需要由外部处理
bool bg_player_add(const LL &id) {
    bsoncxx::document::value doc = bsoncxx::builder::stream::document{}
        << "id" << id
        << "nickname" << ""
        << "signInCount" << (LL)0
        << "signInCountCont" << (LL)0
        << "lastFight" << (LL)0
        << "lastSignIn" << (LL)0
        << "coins" << (LL)0
        << "heroCoin" << (LL)0
        << "level" << (LL)0
        << "energy" << (LL)0
        << "exp" << (LL)0
        << "invCapacity" << (LL)INV_DEFAULT_CAPACITY
        << "inventory" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
        << "vip" << (LL)0
        << "equipments" << bsoncxx::builder::stream::open_document << bsoncxx::builder::stream::close_document
        << "equipItems" << bsoncxx::builder::stream::open_document << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize;

    if (!dbInsertDocument(DB_COLL_USERDATA, doc))
        return false;

    player p(id);
    p.nickname = "";                        p.nickname_cache = true;
    SET_LL_PROP_ZERO(signInCount);
    SET_LL_PROP_ZERO(signInCountCont);
    SET_LL_PROP_ZERO(lastFight);
    SET_LL_PROP_ZERO(lastSignIn);
    SET_LL_PROP_ZERO(coins);
    SET_LL_PROP_ZERO(heroCoin);
    SET_LL_PROP_ZERO(level);
    SET_LL_PROP_ZERO(energy);
    SET_LL_PROP_ZERO(exp);
    p.invCapacity = INV_DEFAULT_CAPACITY;   p.invCapacity_cache = true;
    SET_LL_PROP_ZERO(vip);

    std::scoped_lock<std::mutex> lock(mutexAllPlayers);
    allPlayers.insert(std::make_pair(id, p));
    return true;
}

// 懒人宏
// 把玩家属性中 LL 类型的数据设置为从数据库中读取到的内容, 并把对应的缓存标识设置为 true
// 顺便把可能发生的异常处理一下
#define SET_LL_PROP(prop)                       \
    tmp = doc[#prop];                           \
    if (tmp) {                                  \
        p.##prop = tmp.get_int64().value;       \
        p.##prop##_cache = true;                \
    }                                           \
    else                                        \
        throw std::string("获取玩家") + std::to_string(id) + "的 " + #prop + std::string(" 属性失败");

// 从数据库读取所有玩家
bool bg_get_allplayers_from_db() {
    try {
        std::scoped_lock<std::mutex> lock(mutexAllPlayers);

        for (const auto &doc : dbFindAll(DB_COLL_USERDATA)) {
            auto tmp = doc["id"];
            LL id;

            if (tmp)
                id = tmp.get_int64().value;
            player p(id);
            
            p.nickname = doc["nickname"].get_utf8().value;  p.nickname_cache = true;
            SET_LL_PROP(signInCount);
            SET_LL_PROP(signInCountCont);
            SET_LL_PROP(lastFight);
            SET_LL_PROP(lastSignIn);
            SET_LL_PROP(coins);
            SET_LL_PROP(heroCoin);
            SET_LL_PROP(level);;
            SET_LL_PROP(energy);
            SET_LL_PROP(exp);
            SET_LL_PROP(invCapacity);
            SET_LL_PROP(vip);
            // todo
            //p.inventory
            //p.equipments
            //p.equipItems
            //p.buyCount

            allPlayers.insert(std::make_pair(id, p));
        }
        return true;
    }
    catch (...) {
        return false;
    }
}

// --------------------------------------------------
// 各种属性的 getter 和 setter

// 懒人宏
// 方便编写 LL 类型属性的 getter 和 setter. 其中包括:
// 1. 获取某个属性; 2. 设置某个属性; 3. 为某个属性添加指定数值
#define LL_GET_SET_INC(propName)                                                \
    LL player::get_##propName##(const bool &use_cache) {                        \
        if (##propName##_cache && use_cache)                                    \
            return this->##propName##;                                          \
                                                                                \
        auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, #propName);   \
        if (!result)                                                            \
            throw "找不到对应的玩家 ID";                                         \
        auto field = result->view()[#propName];                                 \
        if (!field.raw())                                                       \
            throw "没有找到 " #propName " field";                               \
        auto tmp = field.get_int64().value;                                     \
                                                                                \
        LOCK_CURR_PLAYER;                                                       \
        this->##propName## = tmp;                                               \
        this->##propName##_cache = true;                                        \
        return tmp;                                                             \
    }                                                                           \
                                                                                \
    bool player::set_##propName##(const LL &val) {                              \
        LOCK_CURR_PLAYER;                                                       \
        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",               \
            bsoncxx::builder::stream::document{} << #propName << val            \
            << bsoncxx::builder::stream::finalize)) {                           \
                                                                                \
            this->##propName## = val;                                           \
            this->##propName##_cache = true;                                    \
            return true;                                                        \
        }                                                                       \
        return false;                                                           \
    }                                                                           \
                                                                                \
    bool player::inc_##propName##(const LL &val) {                              \
        LOCK_CURR_PLAYER;                                                       \
        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$inc",               \
            bsoncxx::builder::stream::document{} << #propName << val            \
            << bsoncxx::builder::stream::finalize)) {                           \
                                                                                \
            this->##propName## += val;                                          \
            return true;                                                        \
        }                                                                       \
        return false;                                                           \
    }

LL_GET_SET_INC(signInCount);
LL_GET_SET_INC(signInCountCont);
LL_GET_SET_INC(lastFight);
LL_GET_SET_INC(lastSignIn);
LL_GET_SET_INC(coins);
LL_GET_SET_INC(heroCoin);
LL_GET_SET_INC(level);
LL_GET_SET_INC(energy);
LL_GET_SET_INC(exp);
LL_GET_SET_INC(invCapacity);
LL_GET_SET_INC(vip);

// 玩家 ID
LL player::get_id() {
    return this->id;
}

// 获取昵称
std::string player::get_nickname(const bool &use_cache) {
    if (nickname_cache && use_cache)
        return this->nickname;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "nickname");
    if (!result)
        throw "找不到对应的玩家 ID";
    auto field = result->view()["nickname"];
    if (!field.raw())
        throw "没有找到 nickname field";
    auto tmp = std::string(result->view()["nickname"].get_utf8().value);

    LOCK_CURR_PLAYER;
    this->nickname = tmp;
    this->nickname_cache = true;
    return tmp;
}

// 设置昵称
bool player::set_nickname(const std::string &val) {
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "nickname", val)) {
        this->nickname = val;
        this->nickname_cache = true;
        return true;
    }
    return false;
}

// 获取整个背包列表
std::list<inventoryData> player::get_inventory(const bool &use_cache) {
    if (inventory_cache && use_cache)
        return inventory;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "inventory");
    if (!result)
        throw "找不到对应玩家 ID";
    auto field = result->view()["inventory"];
    if (!field.raw())
        throw "没有找到 inventory field";
    bsoncxx::array::view tmpArray{ field.get_array().value };   // 处理 BSON 数组

    // 数组格式: [{id: x, level: x, wear: x}, {id: x, level: x, wear: x}, ...]
    std::list<inventoryData> rtn;
    inventoryData invItem;
    for (const auto &item : tmpArray) {
        auto tmpObj = item.get_document().view();

        invItem.id = tmpObj["id"].get_int64().value;            // 装备 ID
        invItem.level = tmpObj["level"].get_int64().value;      // 装备等级
        invItem.wear = tmpObj["wear"].get_int64().value;        // 装备磨损度
        rtn.push_back(invItem);
    }

    LOCK_CURR_PLAYER;
    this->inventory = rtn;
    this->inventory_cache = true;
    return rtn;
}

// 获取背包装备数量
LL player::get_inventory_size(const bool &use_cache) {
    if (inventory_cache && use_cache)
        return inventory.size();
    return get_inventory().size();
}

// 按照指定序号移除背包物品. 如果指定序号无效, 则返回 false
bool player::remove_at_inventory(const LL &index) {
    // 必须有缓存才能继续
    if (!inventory_cache)
        get_inventory();
    if (!inventory_cache)
        return false;

    // 检查序号是否有效
    if (index >= inventory.size())
        return false;

    // 更新数据库, 成功后再更新本地缓存
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "inventory." + std::to_string(index) << bsoncxx::types::b_null()
        << bsoncxx::builder::stream::finalize)) {

        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$pull",
            bsoncxx::builder::stream::document{} << "inventory" << bsoncxx::types::b_null()
            << bsoncxx::builder::stream::finalize)) {

            auto it = this->inventory.begin();
            std::advance(it, index);
            this->inventory.erase(it);
            return true;
        }
        return false;
    }
    return false;
}

// 按照指定的序号列表移除背包物品. 指定的序号不得重复. 如果指定序号无效, 则返回 false
bool player::remove_at_inventory(const std::vector<LL> &indexes) {
    // 必须有缓存才能继续
    if (!inventory_cache)
        get_inventory();
    if (!inventory_cache)
        return false;

    // 检查序号是否有效, 并添加到 document 中
    bsoncxx::builder::basic::document doc;
    for (const auto &index : indexes) {
        if (index >= inventory.size())
            return false;
        doc.append(bsoncxx::builder::basic::kvp("inventory." + std::to_string(index), bsoncxx::types::b_null()));
    }

    // 更新数据库, 成功后再更新本地缓存
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set", doc)) {
        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$pull",
            bsoncxx::builder::stream::document{} << "inventory" << bsoncxx::types::b_null()
            << bsoncxx::builder::stream::finalize)) {

            auto sortedIndexes = indexes;
            std::sort(sortedIndexes.rbegin(), sortedIndexes.rend());    // 把序号从大到小排序

            LL      prevIndex = static_cast<LL>(this->inventory.size()) - 1;
            auto    it = this->inventory.end();
            --it;
            for (const auto &index : indexes) {
                std::advance(it, index - prevIndex);
                this->inventory.erase(it++);
                prevIndex = index;
            }
            return true;
        }
        return false;
    }
    return false;
}

// 添加新物品到背包末尾
bool player::add_inventory_item(const inventoryData &item) {
    return false;
}

// 设置整个背包列表
bool player::set_inventory(const std::list<inventoryData> &val) {
    return false;
}

// 获取整个购买次数表
std::unordered_map<LL, LL> player::get_buyCount(const bool &use_cache) {
    return buyCount;
}

// 获取购买次数表中某个商品的购买次数. 如果找不到对应的商品购买记录, 则返回 0
LL player::get_buyCount_item(const LL &id, const bool &use_cache) {
    return 0;
}

// 设置购买次数表中某个商品的购买次数. 如果对应商品的购买记录不存在, 则会创建
bool player::set_buyCount_item(const LL &id, const LL &count) {
    return false;
}

// 设置整个购买次数表
bool player::set_buyCount(const std::unordered_map<LL, LL> &val) {
    return false;
}

// 获取整个已装备的装备表
std::unordered_map<EqiType, inventoryData> player::get_equipments(const bool &use_cache) {
    return equipments;
}

// 获取某个类型的装备
inventoryData player::get_equipments_item(const EqiType &type, const bool &use_cache) {
    return equipments[type];
}

// 设置某个类型的装备. 如果要移除, 则把 item 的 id 设置为 -1
bool player::set_equipments_item(const EqiType &type, const inventoryData &item) {
    return false;
}

// 设置整个已装备的装备表
bool player::set_equipments(const std::unordered_map<EqiType, inventoryData> &val) {
    return false;
}

// 获取整个已装备的一次性物品表
std::list<inventoryData> player::get_equipItems(const bool &use_cache) {
    return equipItems;
}

// 获取某个已装备的一次性物品. 如果指定序号无效, 则返回 false
bool player::get_equipItems_item(const LL &index, const bool &use_cache) {
    return false;
}

// 移除某个已装备的一次性物品. 如果指定序号无效, 则返回 false
bool player::get_equipItems_item(const LL &index, inventoryData &item, const bool &use_cache) {
    return false;
}

// 添加新物品到已装备的一次性物品列表末尾
bool player::add_equipItems_item(const inventoryData &item) {
    return false;
}

// 设置整个已装备的一次性物品列表
bool player::set_equipItems(const std::list<inventoryData> &val) {
    return false;
}

