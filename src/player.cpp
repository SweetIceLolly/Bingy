/*
描述: Bingy 玩家相关操作
作者: 冰棍
文件: player.cpp
*/

#include "player.hpp"
#include "database.hpp"

#define LOCK_CURR_PLAYER std::scoped_lock<std::mutex> __lock(this->mutexPlayer);

std::unordered_map<long long, player>   allPlayers;
std::mutex                              mutexAllPlayers;

// --------------------------------------------------
// 构造函数

player::player() {
    throw "创建 player 对象时必须指定 QQ 号!";
}

player::player(const player &p) {
    this->id = p.id;
    this->nickname = p.nickname;
    this->signInCount = p.signInCount;
    this->signInCountCont = p.signInCountCont;
    this->lastFight = p.lastFight;
    this->lastSignIn = p.lastSignIn;
    this->coins = p.coins;
    this->heroCoin = p.heroCoin;
    this->level = p.level;
    this->energy = p.energy;
    this->exp = p.exp;
    this->invCapacity = p.invCapacity;
    //this->inventory = p.inventory;
    this->vip = p.vip;
    //this.equipments = p.equipments;
    //this.equipItems = p.equipItems;
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

// 玩家 ID
LL player::getId() {
    return this->id;
}

// 获取昵称
std::string player::getNickname(bool use_cache) {
    if (nickname_cache && use_cache)
        return this->nickname;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id);
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
bool player::setNickname(const std::string &val) {
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "nickname", val)) {
        return true;

        LOCK_CURR_PLAYER;
        this->nickname = val;
        this->nickname_cache = true;
    }
    return false;
}

// 获取硬币
LL player::getCoins(bool use_cache) {
    if (coins_cache && use_cache)
        return this->coins;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id);
    if (!result)
        throw "找不到对应的玩家 ID";
    auto field = result->view()["coins"];
    if (!field.raw())
        throw "没有找到 coins field";
    auto tmp = field.get_int64().value;

    LOCK_CURR_PLAYER;
    this->coins = tmp;
    this->coins_cache = true;
    return tmp;
}

// 设置硬币
bool player::setCoins(const LL &val) {
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "coins", val)) {
        LOCK_CURR_PLAYER;
        this->coins = val;
        this->nickname_cache = true;
        return true;
    }
    return false;
}

// 添加硬币
bool player::incCoins(const LL &val) {
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$inc",
        bsoncxx::builder::stream::document{} << "i" << val << bsoncxx::builder::stream::finalize)) {

        LOCK_CURR_PLAYER;
        this->coins += val;
        this->nickname_cache = true;
        return true;
    }
    return false;
}
