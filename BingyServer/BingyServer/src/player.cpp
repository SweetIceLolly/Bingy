/*
描述: Bingy 玩家相关操作
作者: 冰棍
文件: player.cpp
*/

#include "player.hpp"
#include "database.hpp"
#include <chrono>               // 用于确认线程超时

#define LOCK_CURR_PLAYER std::scoped_lock<std::mutex> __lock(this->mutexPlayer);

std::unordered_map<LL, player>  allPlayers;         // 注意: 读取的时候可以不用加锁, 但是不要使用[], 需要使用 at(). 多线程写入的时候必须加锁
std::mutex                      mutexAllPlayers;

template <typename T> void invListFromBson(const bsoncxx::document::element &elem, T &container);
void eqiMapFromBson(const bsoncxx::document::element &elem, std::unordered_map<EqiType, inventoryData> &container);

// --------------------------------------------------
// 构造函数

player::player() {
    throw std::exception("创建 player 对象时必须指定 QQ 号!");
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

    if (!dbInsertDocument(DB_COLL_USERDATA, doc))
        return false;

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
        throw std::exception("获取玩家属性失败");

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
            SET_LL_PROP(level);
            SET_LL_PROP(blessing);
            SET_LL_PROP(energy);
            SET_LL_PROP(exp);
            SET_LL_PROP(invCapacity);
            SET_LL_PROP(vip);

            // 读取背包内容
            tmp = doc["inventory"];
            if (tmp)
                invListFromBson(tmp, p.inventory);
            else
                throw std::exception(("获取玩家" + std::to_string(id) + "的 inventory 属性失败").c_str());

            // 读取玩家装备
            tmp = doc["equipments"];
            if (tmp)
                eqiMapFromBson(tmp, p.equipments);
            else
                throw std::exception(("获取玩家" + std::to_string(id) + "的 equipments 属性失败").c_str());

            // 读取一次性装备
            tmp = doc["equipItems"];
            if (tmp)
                invListFromBson(tmp, p.equipItems);
            else
                throw std::exception(("获取玩家" + std::to_string(id) + "的 equipItems 属性失败").c_str());

            // todo
            //p.buyCount

            allPlayers.insert(std::make_pair(id, p));
        }
        return true;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
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
#define LL_GET_SET_INC(propName)                                                \
    LL player::get_##propName##(const bool &use_cache) {                        \
        if (##propName##_cache && use_cache)                                    \
            return this->##propName##;                                          \
                                                                                \
        auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, #propName);   \
        if (!result)                                                            \
            throw std::exception("找不到对应的玩家 ID");                         \
        auto field = result->view()[#propName];                                 \
        if (!field.raw())                                                       \
            throw std::exception("没有找到 " #propName " field");                \
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
        if (!##propName##_cache) {                                              \
            get_##propName##();                                                 \
            if (!##propName##_cache) {                                          \
                return false;                                                   \
            }                                                                   \
        }                                                                       \
                                                                                \
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
LL_GET_SET_INC(blessing);
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
        throw std::exception("找不到对应的玩家 ID");
    auto field = result->view()["nickname"];
    if (!field.raw())
        throw std::exception("没有找到 nickname field");
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

// 处理背包的 BSON 数组, 并把内容添加到指定的容器中
// 注意, 该函数假设 elem 是合法的且存有装备数据
template <typename T>
void invListFromBson(const bsoncxx::document::element &elem, T &container) {
    bsoncxx::array::view tmpArray{ elem.get_array().value };   // 处理 BSON 数组

    // 数组格式: [{id: x, level: x, wear: x}, {id: x, level: x, wear: x}, ...]
    inventoryData invItem;
    for (const auto &item : tmpArray) {
        auto tmpObj = item.get_document().view();

        invItem.id = tmpObj["id"].get_int64().value;            // 装备 ID
        invItem.level = tmpObj["level"].get_int64().value;      // 装备等级
        invItem.wear = tmpObj["wear"].get_int64().value;        // 装备磨损度
        container.push_back(invItem);
    }
}

// 获取整个背包列表
std::list<inventoryData> player::get_inventory(const bool &use_cache) {
    if (inventory_cache && use_cache)
        return inventory;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "inventory");
    if (!result)
        throw std::exception("找不到对应玩家 ID");
    auto field = result->view()["inventory"];
    if (!field.raw())
        throw std::exception("没有找到 inventory field");

    std::list<inventoryData> rtn;
    invListFromBson(field, rtn);
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
    if (!inventory_cache) {
        get_inventory();
        if (!inventory_cache)
            return false;
    }

    // 检查序号是否有效
    if (index >= inventory.size())
        return false;

    // 更新数据库, 成功后再更新本地缓存
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "inventory." + std::to_string(index) << bsoncxx::types::b_null()
        << bsoncxx::builder::stream::finalize)) {

        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$pu",
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
    bsoncxx::builder::basic::document doc;
    for (const auto &index : indexes) {
        if (index >= inventory.size())
            return false;
        doc.append(bsoncxx::builder::basic::kvp("inventory." + std::to_string(index), bsoncxx::types::b_null()));
    }

    // 更新数据库, 成功后再更新本地缓存
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set", doc)) {
        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$pu",
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

// 处理装备的 BSON 数据, 并把内容添加到指定的容器中
// 注意, 该函数假设 elem 是合法的且存有已装备的装备数据. 本函数不会获取已装备的一次性装备
void eqiMapFromBson(const bsoncxx::document::element &elem, std::unordered_map<EqiType, inventoryData> &container) {
    // 数据格式: equipments: {1: {id: x, level: x, wear: x}, ...}
    auto tmp = elem.get_document().view();
    for (const auto &it : tmp) {
        if (it.type() == bsoncxx::type::k_document) {         // 只处理 object 类型的元素
            inventoryData invData;
            invData.id = it.get_document().value["id"].get_int64().value;
            invData.level = it.get_document().value["level"].get_int64().value;
            invData.wear = it.get_document().value["wear"].get_int64().value;
            container.insert({ static_cast<EqiType>(std::stoll(it.key().data())), invData });
        }
    }
}

// 获取整个已装备的装备表
std::unordered_map<EqiType, inventoryData> player::get_equipments(const bool &use_cache) {
    if (equipments_cache && use_cache)
        return equipments;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "equipments");
    if (!result)
        throw std::exception("找不到对应玩家 ID");
    auto field = result->view()["equipments"];
    if (!field.raw())
        throw std::exception("没有找到 equipments field");

    std::unordered_map<EqiType, inventoryData> rtn;
    eqiMapFromBson(field, rtn);
    LOCK_CURR_PLAYER;
    this->equipments = rtn;
    this->equipments_cache = true;
    return rtn;
}

// 获取某个类型的装备
inventoryData player::get_equipments_item(const EqiType &type, const bool &use_cache) {
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
        return true;
    }
    return false;
}

// 获取整个已装备的一次性物品表
std::list<inventoryData> player::get_equipItems(const bool &use_cache) {
    if (equipItems_cache && use_cache)
        return equipItems;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "equipItems");
    if (!result)
        throw std::exception("找不到对应玩家 ID");
    auto field = result->view()["equipItems"];
    if (!field.raw())
        throw std::exception("没有找到 equipItems field");

    std::list<inventoryData> rtn;
    invListFromBson(field, rtn);
    LOCK_CURR_PLAYER;
    this->equipItems = rtn;
    this->equipItems_cache = true;
    return rtn;
}

// 获取已装备的一次性物品数量
LL player::get_equipItems_size(const bool &use_cache) {
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
    if (index >= equipItems.size())
        return false;

    // 更新数据库, 成功后再更新本地缓存
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "equipItems." + std::to_string(index) << bsoncxx::types::b_null()
        << bsoncxx::builder::stream::finalize)) {

        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$pu",
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

// 攻 = 20 + 所有装备攻总和 + 等级 * 1.1 + 祝福 * 2
double player::get_atk() {
    static double calc_result = 20;
    if (!atk_cache) {
        // -----------------------------------
        // 重新计算
        calc_result = 20;
        for (auto &item : equipments) {
            if (item.first != EqiType::single_use) {
                calc_result += item.second.calc_atk();
            }
        }
        calc_result += level * 1.1 + blessing * 2;
        // -----------------------------------
        atk_cache = true;
    }
    return calc_result;
}

// 防 = 20 + 所有装备防总和 + 等级 * 0.6 + 祝福 * 1.5
double player::get_def() {
    static double calc_result = 20;
    if (!def_cache) {
        // -----------------------------------
        // 重新计算
        calc_result = 20;
        for (auto &item : equipments) {
            if (item.first != EqiType::single_use) {
                calc_result += item.second.calc_def();
            }
        }
        calc_result += level * 0.6 + blessing * 1.5;
        // -----------------------------------
        def_cache = true;
    }
    return calc_result;
}

// 破 = 所有武器破总和
double player::get_brk() {
    static double calc_result = 0;
    if (!brk_cache) {
        // -----------------------------------
        // 重新计算
        calc_result = 0;
        for (auto &item : equipments) {
            if (item.first != EqiType::single_use) {
                calc_result += item.second.calc_brk();
            }
        }
        // -----------------------------------
        brk_cache = true;
    }
    return calc_result;
}

// 敏 = 10 + 所有装备敏总和 + 祝福 * 0.2
double player::get_agi() {
    static double calc_result = 0;
    if (!agi_cache) {
        // -----------------------------------
        // 重新计算
        calc_result = 10;
        for (auto &item : equipments) {
            if (item.first != EqiType::single_use) {
                calc_result += item.second.calc_agi();
            }
        }
        calc_result += blessing * 0.2;
        // -----------------------------------
        agi_cache = true;
    }
    return calc_result;
}

// 血 = 100 + 所有装备血总和 + 玩家等级 * 祝福 / 10 + 祝福
double player::get_hp() {
    static double calc_result = 0;
    if (!hp_cache) {
        // -----------------------------------
        // 重新计算
        calc_result = 100;
        for (auto &item : equipments) {
            if (item.first != EqiType::single_use) {
                calc_result += item.second.calc_hp();
            }
        }
        calc_result += level * blessing / 10.0 + blessing;
        // -----------------------------------
        hp_cache = true;
    }
    return calc_result;
}

// 魔 = 所有装备魔总和 + 祝福 * 1.7 + 玩家等级 * 1.7
double player::get_mp() {
    static double calc_result = 0;
    if (!mp_cache) {
        // -----------------------------------
        // 重新计算
        calc_result = 0;
        for (auto &item : equipments) {
            if (item.first != EqiType::single_use) {
                calc_result += item.second.calc_mp();
            }
        }
        calc_result += blessing * 1.7 + level * 1.7;
        // -----------------------------------
        mp_cache = true;
    }
    return calc_result;
}

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
    return (LL)(100.0 + level * 10.0 + 8 * pow(1.18, level));
}

// 冷却时间 = Min(200 - 玩家等级 * 1.2, 40)
LL player::get_cd() {
    LL cd = (LL)(200.0 - level * 1.2);
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
#define ALL_PLAYER_INC(field)                                           \
    bool bg_all_player_inc_##field##(const LL &val) {                   \
        /* 更新数据库, 成功后再更新本地缓存 */                            \
        if (dbUpdateAll(DB_COLL_USERDATA, "$inc",                       \
            bsoncxx::builder::stream::document{} << #field << val       \
            << bsoncxx::builder::stream::finalize)) {                   \
                                                                        \
            /* 为所有玩家添加硬币 */                                     \
            for (auto &p : allPlayers) {                                \
                if (!p.second.##field##_cache) {                        \
                    p.second.get_##field##();                           \
                    if (!p.second.##field##_cache) {                    \
                        return false;                                   \
                    }                                                   \
                }                                                       \
                                                                        \
                std::unique_lock lock(p.second.mutexPlayer);            \
                p.second.##field## += val;                              \
                lock.unlock();                                          \
            }                                                           \
            return true;                                                \
        }                                                               \
        return false;                                                   \
    }

ALL_PLAYER_INC(coins);
ALL_PLAYER_INC(heroCoin);
ALL_PLAYER_INC(level);
ALL_PLAYER_INC(blessing);
ALL_PLAYER_INC(energy);
ALL_PLAYER_INC(exp);
ALL_PLAYER_INC(invCapacity);
ALL_PLAYER_INC(vip);
