/*
描述: Bingy 玩家相关操作
作者: 冰棍
文件: player.cpp
*/

#include "player.hpp"
#include "database.hpp"
#include <chrono>               // 用于确认线程超时

#define LOCK_CURR_PLAYER std::scoped_lock<std::mutex> __lock(this->mutexPlayer);

std::unordered_map<LL, player>  allPlayers;         // 注意: 无论读写都必须加锁
std::mutex                      mutexAllPlayers;
std::unordered_set<LL>          allAdmins;
std::unordered_set<LL>          blacklist;

template <typename T> void invListFromBson(const bsoncxx::document::element &elem, T &container);
void eqiMapFromBson(const bsoncxx::document::element &elem, std::unordered_map<EqiType, inventoryData> &container);

// --------------------------------------------------
// 构造函数

player::player() {
    throw std::runtime_error("创建 player 对象时必须指定 QQ 号!");
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
    this->blessing = p.blessing;
    this->blessing_cache = p.blessing_cache;
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
    this->upgrading = false;
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
    this->blessing = 0;
    this->energy = 0;
    this->exp = 0;
    this->invCapacity = INV_DEFAULT_CAPACITY;
    this->vip = 0;
    this->upgrading = false;
}

// --------------------------------------------------
// 玩家操作函数

// 懒人宏
// 把玩家属性中 LL 类型的数据设置为0, 并把对应的缓存标识设置为 true
#define SET_LL_PROP_ZERO(prop) p. prop = 0; p. prop## _cache = true;

// 无条件添加玩家到数据库和字典中. 抛出的异常需要由外部处理
bool bg_player_add(const LL &id) {
    // 把所有装备都设置为 -1 (空)
    auto equipments = bsoncxx::builder::stream::document{};
    for (LL i = 0; i <= 9; ++i) {
        equipments << std::to_string(i) << bsoncxx::builder::stream::open_document
            << "id" << (LL)-1
            << "level" << (LL)-1
            << "wear" << (LL)-1
            << bsoncxx::builder::stream::close_document;
    }
    equipments << "10" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array;

    bsoncxx::document::value doc = bsoncxx::builder::stream::document{}
        << "id" << id
        << "nickname" << ""
        << "signInCount" << (LL)0
        << "signInCountCont" << (LL)0
        << "lastFight" << (LL)0
        << "lastSignIn" << (LL)0
        << "coins" << (LL)0
        << "heroCoin" << (LL)0
        << "level" << (LL)1
        << "blessing" << (LL)0
        << "energy" << (LL)0
        << "exp" << (LL)0
        << "invCapacity" << (LL)INV_DEFAULT_CAPACITY
        << "inventory" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
        << "buyCount" << bsoncxx::builder::stream::open_document << bsoncxx::builder::stream::close_document
        << "equipments" << equipments
        << "equipItems" << bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
        << "vip" << (LL)0
        << bsoncxx::builder::stream::finalize;

    // 初始化玩家属性
    player p(id);
    p.nickname = "";                        p.nickname_cache = true;
    SET_LL_PROP_ZERO(signInCount);
    SET_LL_PROP_ZERO(signInCountCont);
    SET_LL_PROP_ZERO(lastFight);
    SET_LL_PROP_ZERO(lastSignIn);
    SET_LL_PROP_ZERO(coins);
    SET_LL_PROP_ZERO(heroCoin);
    p.level = 1;                            p.level_cache = true;
    SET_LL_PROP_ZERO(blessing);
    SET_LL_PROP_ZERO(energy);
    SET_LL_PROP_ZERO(exp);
    p.invCapacity = INV_DEFAULT_CAPACITY;   p.invCapacity_cache = true;
    SET_LL_PROP_ZERO(vip);
    p.inventory_cache = true;
    p.buyCount_cache = true;
    p.equipItems_cache = true;

    // 初始化玩家装备
    p.equipments[EqiType::armor_helmet] = { -1, -1, -1 };
    p.equipments[EqiType::armor_body] = { -1, -1, -1 };
    p.equipments[EqiType::armor_leg] = { -1, -1, -1 };
    p.equipments[EqiType::armor_boot] = { -1, -1, -1 };
    p.equipments[EqiType::weapon_primary] = { -1, -1, -1 };
    p.equipments[EqiType::weapon_secondary] = { -1, -1, -1 };
    p.equipments[EqiType::ornament_earrings] = { -1, -1, -1 };
    p.equipments[EqiType::ornament_rings] = { -1, -1, -1 };
    p.equipments[EqiType::ornament_necklace] = { -1, -1, -1 };
    p.equipments[EqiType::ornament_jewelry] = { -1, -1, -1 };
    p.equipments_cache = true;

    std::scoped_lock<std::mutex> _lock(mutexAllPlayers);
    if (!dbInsertDocument(DB_COLL_USERDATA, doc))
        return false;
    allPlayers.insert(std::make_pair(id, p));

    return true;
}

// 从数据库读取所有玩家
bool bg_get_all_players_from_db() {
    LL id = 0;

    try {
        std::scoped_lock<std::mutex> lock(mutexAllPlayers);
        for (const auto &doc : dbFindAll(DB_COLL_USERDATA)) {
            player *p = nullptr;

            for (const auto &entry : doc) {
                if (entry.key().compare("id") == 0) {
                    id = entry.get_int64().value;
                    p = new player(id);
                }
                else if (entry.key().compare("nickname") == 0) {
                    p->nickname = entry.get_utf8().value.data();
                    p->nickname_cache = true;
                }
                else if (entry.key().compare("signInCount") == 0) {
                    p->signInCount = entry.get_int64().value;
                    p->signInCount_cache = true;
                }
                else if (entry.key().compare("signInCountCont") == 0) {
                    p->signInCountCont = entry.get_int64().value;
                    p->signInCountCont_cache = true;
                }
                else if (entry.key().compare("lastFight") == 0) {
                    p->lastFight = entry.get_int64().value;
                    p->lastFight_cache = true;
                }
                else if (entry.key().compare("lastSignIn") == 0) {
                    p->lastSignIn = entry.get_int64().value;
                    p->lastSignIn_cache = true;
                }
                else if (entry.key().compare("coins") == 0) {
                    p->coins = entry.get_int64().value;
                    p->coins_cache = true;
                }
                else if (entry.key().compare("heroCoin") == 0) {
                    p->heroCoin = entry.get_int64().value;
                    p->heroCoin_cache = true;
                }
                else if (entry.key().compare("level") == 0) {
                    p->level = entry.get_int64().value;
                    p->level_cache = true;
                }
                else if (entry.key().compare("blessing") == 0) {
                    p->blessing = entry.get_int64().value;
                    p->blessing_cache = true;
                }
                else if (entry.key().compare("energy") == 0) {
                    p->energy = entry.get_int64().value;
                    p->energy_cache = true;
                }
                else if (entry.key().compare("exp") == 0) {
                    p->exp = entry.get_int64().value;
                    p->exp_cache = true;
                }
                else if (entry.key().compare("invCapacity") == 0) {
                    p->invCapacity = entry.get_int64().value;
                    p->invCapacity_cache = true;
                }
                else if (entry.key().compare("vip") == 0) {
                    p->vip = entry.get_int64().value;
                    p->vip_cache = true;
                }
                else if (entry.key().compare("inventory") == 0) {
                    invListFromBson(entry, p->inventory);
                    p->inventory_cache = true;
                }
                else if (entry.key().compare("equipments") == 0) {
                    eqiMapFromBson(entry, p->equipments);
                    p->equipments_cache = true;
                }
                else if (entry.key().compare("equipItems") == 0) {
                    invListFromBson(entry, p->equipItems);
                    p->equipItems_cache = true;
                }
                else if (entry.key().compare("buyCount") == 0) {
                    // todo
                }
                else {
                    throw std::runtime_error("未知的玩家属性: " + std::string(entry.key().data()));
                }
            }
            allPlayers.insert(std::make_pair(id, *p));
            delete p;
        }
        return true;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
        console_log("读取玩家" + std::to_string(id) + "失败", LogType::error);
        return false;
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
#define LL_GET_SET_INC(propName, cleanCache)                                    \
    LL player::get_ ##propName (bool use_cache) {                               \
        LOCK_CURR_PLAYER;                                                       \
        if ( propName## _cache && use_cache)                                    \
            return this-> propName ;                                            \
                                                                                \
        auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, #propName);   \
        if (!result)                                                            \
            throw std::runtime_error("找不到对应的玩家 ID");                     \
        auto field = result->view()[#propName];                                 \
        if (!field.raw())                                                       \
            throw std::runtime_error("没有找到 " #propName " field");            \
        auto tmp = field.get_int64().value;                                     \
                                                                                \
        this-> propName  = tmp;                                                 \
        this-> propName## _cache = true;                                        \
        return tmp;                                                             \
    }                                                                           \
                                                                                \
    bool player::set_ ##propName (const LL &val) {                              \
        LOCK_CURR_PLAYER;                                                       \
        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",               \
            bsoncxx::builder::stream::document{} << #propName << val            \
            << bsoncxx::builder::stream::finalize)) {                           \
                                                                                \
            this-> propName = val;                                              \
            this-> propName## _cache = true;                                    \
            if ( cleanCache )                                                   \
                resetCache();                                                   \
            return true;                                                        \
        }                                                                       \
        return false;                                                           \
    }                                                                           \
                                                                                \
    bool player::inc_ ##propName (const LL &val) {                              \
        if (! propName## _cache) {                                              \
            get_ ##propName ();                                                 \
            if (! propName## _cache) {                                          \
                return false;                                                   \
            }                                                                   \
        }                                                                       \
                                                                                \
        LOCK_CURR_PLAYER;                                                       \
        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$inc",               \
            bsoncxx::builder::stream::document{} << #propName << val            \
            << bsoncxx::builder::stream::finalize)) {                           \
                                                                                \
            this-> propName += val;                                             \
            if ( cleanCache )                                                   \
                resetCache();                                                   \
            return true;                                                        \
        }                                                                       \
        return false;                                                           \
    }

LL_GET_SET_INC(signInCount, false);
LL_GET_SET_INC(signInCountCont, false);
LL_GET_SET_INC(lastFight, false);
LL_GET_SET_INC(lastSignIn, false);
LL_GET_SET_INC(coins, false);
LL_GET_SET_INC(heroCoin, false);
LL_GET_SET_INC(level, true);
LL_GET_SET_INC(blessing, true);
LL_GET_SET_INC(energy, false);
LL_GET_SET_INC(exp, false);
LL_GET_SET_INC(invCapacity, false);
LL_GET_SET_INC(vip, false);

// 玩家添加经验值后检查升级
bool player::check_exp_upgrade() {
    while (get_exp() >= get_exp_needed()) {
        if (!inc_exp(-get_exp_needed()))
            return false;
        if (!inc_level(1))
            return false;
    }
    return true;
}

// 玩家 ID
LL player::get_id() {
    return this->id;
}

// 获取昵称
std::string player::get_nickname(bool use_cache) {
    LOCK_CURR_PLAYER;
    if (nickname_cache && use_cache)
        return this->nickname;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "nickname");
    if (!result)
        throw std::runtime_error("找不到对应的玩家 ID");
    auto field = result->view()["nickname"];
    if (!field.raw())
        throw std::runtime_error("没有找到 nickname field");
    auto tmp = std::string(result->view()["nickname"].get_utf8().value.data());
    
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

// 处理背包的 BSON 数组, 并把内容添加到指定的容器中
// 注意, 该函数假设 elem 是合法的且存有装备数据
template <typename T>
void invListFromBson(const bsoncxx::document::element &elem, T &container) {
    bsoncxx::array::view tmpArray{ elem.get_array().value };   // 处理 BSON 数组

    // 数组格式: [{id: x, level: x, wear: x}, {id: x, level: x, wear: x}, ...]
    inventoryData invItem;
    for (const auto &item : tmpArray) {
        for (const auto &entry : item.get_document().view()) {
            if (entry.key().compare("id") == 0) {                          // 装备 ID
                invItem.id = entry.get_int64().value;
            }
            else if (entry.key().compare("level") == 0) {                  // 装备等级
                invItem.level = entry.get_int64().value;
            }
            else if (entry.key().compare("wear") == 0) {                   // 装备磨损度
                invItem.wear = entry.get_int64().value;
            }
            else {
                throw std::runtime_error("未知的玩家背包装备属性: " + std::string(entry.key().data()));
            }
        }
        container.push_back(invItem);
    }
}

// 获取整个背包列表
std::list<inventoryData> player::get_inventory(bool use_cache) {
    LOCK_CURR_PLAYER;
    if (inventory_cache && use_cache)
        return inventory;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "inventory");
    if (!result)
        throw std::runtime_error("找不到对应玩家 ID");
    auto field = result->view()["inventory"];
    if (!field.raw())
        throw std::runtime_error("没有找到 inventory field");

    std::list<inventoryData> rtn;
    invListFromBson(field, rtn);
    this->inventory = rtn;
    this->inventory_cache = true;
    return rtn;
}

// 获取背包中指定序号的物品
inventoryData player::get_inventory_item(LL index, bool use_cache) {
    LOCK_CURR_PLAYER;
    if (inventory_cache && use_cache) {
        auto it = inventory.begin();
        std::advance(it, index);
        return *it;
    }

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "inventory");
    if (!result)
        throw std::runtime_error("找不到对应玩家 ID");
    auto field = result->view()["inventory"];
    if (!field.raw())
        throw std::runtime_error("没有找到 inventory field");

    std::list<inventoryData> rtn;
    invListFromBson(field, rtn);
    this->inventory = rtn;
    this->inventory_cache = true;

    auto it = rtn.begin();
    std::advance(it, index);
    return *it;
}

// 获取背包装备数量
LL player::get_inventory_size(bool use_cache) {
    if (inventory_cache && use_cache)
        return inventory.size();
    return get_inventory().size();
}

// 按照指定序号移除背包物品. 如果指定序号无效, 则返回 false
bool player::remove_at_inventory(const LL &index) {
    LOCK_CURR_PLAYER;
    // 必须有缓存才能继续
    if (!inventory_cache) {
        get_inventory();
        if (!inventory_cache)
            return false;
    }

    // 检查序号是否有效
    if (index >= inventory.size())
        return false;

    // 更新数据库, 成功后再更新本地缓存
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
    if (!inventory_cache) {
        get_inventory();
        if (!inventory_cache)
            return false;
    }

    // 检查序号是否有效, 并添加到 document 中
    LOCK_CURR_PLAYER;
    bsoncxx::builder::basic::document doc;
    for (const auto &index : indexes) {
        if (index >= inventory.size())
            return false;
        doc.append(bsoncxx::builder::basic::kvp("inventory." + std::to_string(index), bsoncxx::types::b_null()));
    }

    // 更新数据库, 成功后再更新本地缓存
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set", doc)) {
        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$pull",
            bsoncxx::builder::stream::document{} << "inventory" << bsoncxx::types::b_null()
            << bsoncxx::builder::stream::finalize)) {

            std::vector<LL> sortedIndexes = indexes;
            std::sort(sortedIndexes.rbegin(), sortedIndexes.rend());    // 把序号从大到小排序

            LL      prevIndex = static_cast<LL>(this->inventory.size()) - 1;
            auto    it = this->inventory.end();
            --it;
            for (const auto &index : sortedIndexes) {
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
    // 必须有缓存才能继续
    if (!inventory_cache) {
        get_inventory();
        if (!inventory_cache)
            return false;
    }

    // 更新数据库, 成功后再更新本地缓存
    // inventory: [{id: id, level: level, wear: wear}, ...]
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$push",
        bsoncxx::builder::stream::document{} << "inventory"
        << bsoncxx::builder::stream::open_document
        << "id" << item.id
        << "level" << item.level
        << "wear" << item.wear
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize)) {

        this->inventory.push_back(item);
        return true;
    }
    return false;
}

// 设置整个背包列表
bool player::set_inventory(const std::list<inventoryData> &val) {
    // 把背包内容添加到 document 中
    auto doc = bsoncxx::builder::basic::array{};
    for (const auto &item : val) {
        doc.append(
            bsoncxx::builder::stream::document{}
            << "id" << item.id
            << "level" << item.level
            << "wear" << item.wear
            << bsoncxx::builder::stream::finalize
        );
    }
    
    // 更新数据库, 成功后再更新本地缓存
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "inventory" << doc
        << bsoncxx::builder::stream::finalize)) {

        this->inventory = val;
        this->inventory_cache = true;
        return true;
    }
    return false;
}

// 获取整个购买次数表
std::unordered_map<LL, LL> player::get_buyCount(bool use_cache) {
    return buyCount;
}

// 获取购买次数表中某个商品的购买次数. 如果找不到对应的商品购买记录, 则返回 0
LL player::get_buyCount_item(const LL &id, bool use_cache) {
    return 0;
}

// 设置购买次数表中某个商品的购买次数. 如果对应商品的购买记录不存在, 则会创建
bool player::set_buyCount_item(const LL &id, const LL &count) {
    return false;
}

// 处理装备的 BSON 数据, 并把内容添加到指定的容器中
// 注意, 该函数假设 elem 是合法的且存有已装备的装备数据. 本函数不会获取已装备的一次性装备
void eqiMapFromBson(const bsoncxx::document::element &elem, std::unordered_map<EqiType, inventoryData> &container) {
    // 数据格式: equipments: {1: {id: x, level: x, wear: x}, ...}
    inventoryData invData;

    for (const auto &it : elem.get_document().view()) {
        if (it.type() == bsoncxx::type::k_document) {         // 只处理 object 类型的元素
            for (const auto &entry : it.get_document().view()) {
                if (entry.key().compare("id") == 0) {
                    invData.id = entry.get_int64().value;
                }
                else if (entry.key().compare("level") == 0) {
                    invData.level = entry.get_int64().value;
                }
                else if (entry.key().compare("wear") == 0) {
                    invData.wear = entry.get_int64().value;
                }
                else {
                    throw std::runtime_error("未知的玩家装备属性: " + std::string(entry.key().data()));
                }
            }
            container.insert({ static_cast<EqiType>(std::stoll(it.key().data())), invData });
        }
    }
}

// 获取整个已装备的装备表
std::unordered_map<EqiType, inventoryData> player::get_equipments(bool use_cache) {
    LOCK_CURR_PLAYER;
    if (equipments_cache && use_cache)
        return equipments;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "equipments");
    if (!result)
        throw std::runtime_error("找不到对应玩家 ID");
    auto field = result->view()["equipments"];
    if (!field.raw())
        throw std::runtime_error("没有找到 equipments field");

    std::unordered_map<EqiType, inventoryData> rtn;
    eqiMapFromBson(field, rtn);
    this->equipments = rtn;
    this->equipments_cache = true;
    return rtn;
}

// 获取某个类型的装备
inventoryData player::get_equipments_item(const EqiType &type, bool use_cache) {
    return get_equipments(use_cache)[type];
}

// 设置某个类型的装备. 如果想要移除某个类型的装备, 则把 item 的 id 设置为 -1
bool player::set_equipments_item(const EqiType &type, const inventoryData &item) {
    // 必须有缓存才能继续
    if (!equipments_cache) {
        get_equipments();
        if (!equipments_cache)
            return false;
    }

    // 更新数据库, 成功后再更新本地缓存
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",
        bsoncxx::builder::stream::document{}
        << "equipments." + std::to_string(static_cast<LL>(type))
        << bsoncxx::builder::stream::open_document
            << "id" << item.id
            << "level" << item.level
            << "wear" << item.wear
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize)) {

        this->equipments[type] = item;
        resetCache();
        return true;
    }
    return false;
}

// 获取整个已装备的一次性物品表
std::list<inventoryData> player::get_equipItems(bool use_cache) {
    LOCK_CURR_PLAYER;
    if (equipItems_cache && use_cache)
        return equipItems;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "equipItems");
    if (!result)
        throw std::runtime_error("找不到对应玩家 ID");
    auto field = result->view()["equipItems"];
    if (!field.raw())
        throw std::runtime_error("没有找到 equipItems field");

    std::list<inventoryData> rtn;
    invListFromBson(field, rtn);
    this->equipItems = rtn;
    this->equipItems_cache = true;
    return rtn;
}

// 获取已装备的一次性物品数量
LL player::get_equipItems_size(bool use_cache) {
    if (equipItems_cache && use_cache)
        return equipItems.size();
    return get_equipItems().size();
}

// 移除某个已装备的一次性物品. 如果指定序号无效, 则返回 false
bool player::remove_at_equipItems(const LL &index) {
    // 必须有缓存才能继续
    if (!equipItems_cache) {
        get_equipItems();
        if (!equipItems_cache)
            return false;
    }

    // 检查序号是否有效
    LOCK_CURR_PLAYER;
    if (index >= equipItems.size())
        return false;

    // 更新数据库, 成功后再更新本地缓存
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "equipItems." + std::to_string(index) << bsoncxx::types::b_null()
        << bsoncxx::builder::stream::finalize)) {

        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$pull",
            bsoncxx::builder::stream::document{} << "equipItems" << bsoncxx::types::b_null()
            << bsoncxx::builder::stream::finalize)) {

            auto it = this->equipItems.begin();
            std::advance(it, index);
            this->equipItems.erase(it);
            return true;
        }
        return false;
    }
    return false;
}

// 清空已装备的一次性物品
bool player::clear_equipItems() {
    // 必须有缓存才能继续
    if (!equipItems_cache) {
        get_equipItems();
        if (!equipItems_cache)
            return false;
    }

    // 更新数据库, 成功后再更新本地缓存
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "equipItems" <<
        bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
        << bsoncxx::builder::stream::finalize)) {

        this->equipItems.clear();
        return true;
    }
    return false;
}

// 添加新物品到已装备的一次性物品列表末尾
bool player::add_equipItems_item(const inventoryData &item) {
    // 必须有缓存才能继续
    if (!equipItems_cache) {
        get_equipItems();
        if (!equipItems_cache)
            return false;
    }

    // 更新数据库, 成功后再更新本地缓存
    // equipItems: [{id: id, level: level, wear: wear}, ...]
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$push",
        bsoncxx::builder::stream::document{} << "equipItems"
        << bsoncxx::builder::stream::open_document
        << "id" << item.id
        << "level" << item.level
        << "wear" << item.wear
        << bsoncxx::builder::stream::close_document
        << bsoncxx::builder::stream::finalize)) {

        this->equipItems.push_back(item);
        return true;
    }
    return false;
}

// --------------------------------------------------
// 玩家战斗属性

// 懒人宏
// 根据指定参数获取某个属性的最终值
#define PLAYER_GET_PROP(type, init_val, extra_val)                  \
    double player::get_ ##type () {                                 \
        static double calc_result = init_val;                       \
        if (! type##_cache) {                                       \
            /* ----------------------------------- */               \
            /* 重新计算 */                                          \
            calc_result = init_val;                                 \
            for (auto &item : equipments) {                         \
                if (item.first != EqiType::single_use) {            \
                    calc_result += item.second.calc_ ##type ();     \
                }                                                   \
            }                                                       \
            calc_result += extra_val ;                              \
            /* ----------------------------------- */               \
            type## _cache = true;                                   \
        }                                                           \
        return calc_result;                                         \
    }

// 攻 = 20 + 所有装备攻总和 + 等级 * 1.1 + 祝福 * 2
PLAYER_GET_PROP(atk, 20, (level * 1.1 + blessing * 2));

// 防 = 20 + 所有装备防总和 + 等级 * 0.6 + 祝福 * 1.5
PLAYER_GET_PROP(def, 20, level * 0.6 + blessing * 1.5);

// 破 = 所有武器破总和
PLAYER_GET_PROP(brk, 0, 0);

// 敏 = 10 + 所有装备敏总和 + 祝福 * 0.2
PLAYER_GET_PROP(agi, 10, blessing * 0.2);

// 血 = 100 + 所有装备血总和 + 玩家等级 * 祝福 / 4
PLAYER_GET_PROP(hp, 100, level * blessing / 4.0);

// 魔 = 所有装备魔总和 + 祝福 * 1.7 + 玩家等级 * 1.7
PLAYER_GET_PROP(mp, 0, blessing * 1.7 + level * 1.7);

// 暴 = 所有装备暴总和 * (1 + 玩家等级 / 170)
double player::get_crt() {
    static double calc_result = 0;
    if (!crt_cache) {
        // -----------------------------------
        // 重新计算
        calc_result = 0;
        for (auto &item : equipments) {
            if (item.first != EqiType::single_use) {
                calc_result += item.second.calc_crt();
            }
        }
        calc_result *= (1.0 + level / 170.0);
        // -----------------------------------
        crt_cache = true;
    }
    return calc_result;
}

// 升级所需经验 = 100 + 玩家等级 * 10 + 8 * 1.18 ^ 玩家等级
LL player::get_exp_needed() {
    return static_cast<LL>(100.0 + level * 10.0 + 8 * pow(1.18, level));
}

// 冷却时间 = Min(200 - 玩家等级 * 1.2, 40)
LL player::get_cd() {
    LL cd = static_cast<LL>(200.0 - level * 1.2);
    if (cd < 40)
        return 40;
    else
        return cd;
}

// 清空计算缓存
void player::resetCache() {
    atk_cache = false;
    def_cache = false;
    brk_cache = false;
    agi_cache = false;
    hp_cache = false;
    mp_cache = false;
    crt_cache = false;

    for (auto &item : equipments) {
        if (item.first != EqiType::single_use) {
            item.second.resetCache();
        }
    }
}

// 取消强化确认
void player::abortUpgrade() {
    upgrading = false;
    cvStatusChange.notify_one();
}

// 确认强化确认
void player::confirmUpgrade() {
    upgrading = true;
    cvStatusChange.notify_one();
}

// 等待强化确认. 如果玩家确认了强化, 就返回 true; 否则返回 false
bool player::waitUpgradeConfirm() {
    confirmInProgress = true;
    upgrading = false;
    std::unique_lock lock(this->mutexStatus);
    cvStatusChange.wait_for(lock, std::chrono::seconds(20));    // 20 秒确认超时
    confirmInProgress = false;
    cvPrevConfirmCompleted.notify_one();
    return this->upgrading;
}

// 等待确认完成
void player::waitConfirmComplete() {
    std::unique_lock lock(this->mutexStatus);
    cvPrevConfirmCompleted.wait(lock);
}

// 懒人宏
// 为所有玩家的指定属性增加指定数值
#define ALL_PLAYER_MODIFY(field)                                        \
    bool bg_all_player_inc_ ##field (const LL &val) {                   \
        /* 更新数据库, 成功后再更新本地缓存 */                            \
        std::scoped_lock<std::mutex> _lock(mutexAllPlayers);            \
        if (dbUpdateAll(DB_COLL_USERDATA, "$inc",                       \
            bsoncxx::builder::stream::document{} << #field << val       \
            << bsoncxx::builder::stream::finalize)) {                   \
                                                                        \
            /* 为所有玩家添加硬币 */                                     \
            for (auto &p : allPlayers) {                                \
                if (!p.second. field## _cache) {                        \
                    p.second.get_ ##field ();                           \
                    if (!p.second. field## _cache) {                    \
                        return false;                                   \
                    }                                                   \
                }                                                       \
                                                                        \
                std::unique_lock lock(p.second.mutexPlayer);            \
                p.second. field += val;                                 \
                lock.unlock();                                          \
            }                                                           \
            return true;                                                \
        }                                                               \
        return false;                                                   \
    }                                                                   \
                                                                        \
    bool bg_all_player_set_ ##field (const LL &val) {                   \
        /* 更新数据库, 成功后再更新本地缓存 */                            \
        std::scoped_lock<std::mutex> _lock(mutexAllPlayers);            \
        if (dbUpdateAll(DB_COLL_USERDATA, "$set",                       \
            bsoncxx::builder::stream::document{} << #field << val       \
            << bsoncxx::builder::stream::finalize)) {                   \
                                                                        \
            /* 为所有玩家修改硬币 */                                     \
            for (auto &p : allPlayers) {                                \
                std::unique_lock lock(p.second.mutexPlayer);            \
                p.second. field = val;                                  \
                p.second. field## _cache = true;                        \
                lock.unlock();                                          \
            }                                                           \
            return true;                                                \
        }                                                               \
        return false;                                                   \
    }

ALL_PLAYER_MODIFY(coins);
ALL_PLAYER_MODIFY(heroCoin);
ALL_PLAYER_MODIFY(level);
ALL_PLAYER_MODIFY(blessing);
ALL_PLAYER_MODIFY(energy);
ALL_PLAYER_MODIFY(exp);
ALL_PLAYER_MODIFY(invCapacity);
ALL_PLAYER_MODIFY(vip);
