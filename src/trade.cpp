/*
描述: Bingy 交易场相关操作
作者: 冰棍
文件: inventory.hpp
*/

#include "trade.hpp"
#include "database.hpp"
#include <sstream>

std::mutex              mutexAllTradeItems;             // 交易场锁
std::map<LL, tradeData> allTradeItems;                  // 所有交易场条目
bool                    allTradeItems_cache = false;    // 交易场条目缓存

std::mutex              mutexTradeId;                   // 交易 ID 锁
LL                      currTradeId;                    // 当前交易 ID
bool                    currTradeId_cache = false;      // 交易 ID 缓存

std::mutex              mutexTradeStr;                  // 交易场字符串缓存锁
std::string             tradeStr;                       // 交易场字符串缓存, 以节省每次生成交易场字符串的开销
bool                    tradeStr_cache = false;         // 交易场字符串缓存标记

bool bg_init_tradeId();                                 // 在数据库中初始化交易 ID 记录

LL bg_get_tradeId(const bool &use_cache) {
    if (currTradeId_cache && use_cache)
        return currTradeId;

    auto result = dbFindOne(DB_COLL_TRADE, "type", "tradeId", "value");
    if (!result) {
        console_log("没有在交易场数据库中找到 tradeId 记录, 尝试重新创建...", LogType::warning);
        if (!bg_init_tradeId())
            throw std::exception("尝试初始化交易 ID 失败");
        console_log("tradeId 已重置为 0", LogType::warning);
        return 0;
    }
    auto field = result->view()["value"];
    if (!field.raw()) {
        console_log("没有在交易场数据库中找到 tradeId 记录, 尝试重新创建...", LogType::warning);
        if (!bg_init_tradeId())
            throw std::exception("尝试初始化交易 ID 失败");
        console_log("tradeId 已重置为 0", LogType::warning);
        return 0;
    }
    auto tmp = field.get_int64().value;

    std::scoped_lock lock(mutexTradeId);
    currTradeId = tmp;
    currTradeId_cache = true;
    return tmp;
}

bool bg_set_tradeId(const LL &val) {
    std::scoped_lock lock(mutexTradeId);
    if (dbUpdateOne(DB_COLL_TRADE, "type", "tradeId", "$set",
        bsoncxx::builder::stream::document{} << "value" << val << bsoncxx::builder::stream::finalize)) {

        currTradeId = val;
        currTradeId_cache = true;
        return true;
    }
    return false;
}

bool bg_init_tradeId() {
    std::scoped_lock lock(mutexTradeId);
    if (dbInsertDocument(DB_COLL_TRADE, bsoncxx::builder::stream::document{}
        << "value" << (LL)0
        << "type" << "tradeId"
        << bsoncxx::builder::stream::finalize
    )) {

        currTradeId = 0;
        currTradeId_cache = true;
        return true;
    }
    return false;
}

bool bg_inc_tradeId() {
    std::scoped_lock lock(mutexTradeId);
    if (dbUpdateOne(DB_COLL_TRADE, "type", "tradeId", "$inc",
        bsoncxx::builder::stream::document{} << "value" << 1 << bsoncxx::builder::stream::finalize)) {

        ++currTradeId;
        return true;
    }
    return false;
}

// 处理交易场项目的 BSON 数据, 并把内容添加到指定的容器中
// 注意, 该函数假设 elem 是合法的且存有交易场项目数据
void tradeMapFromBson(const bsoncxx::document::view &elem, std::map<LL, tradeData> &container) {
    if (elem.find("type") != elem.end() && elem["type"].get_utf8().value == "tradeId")      // 忽略掉下一个交易 ID 的记录
        return;

    tradeData       tItem;
    inventoryData   invItem;

    invItem.id = elem["item"].get_document().value["id"].get_int64().value;
    invItem.level = elem["item"].get_document().value["level"].get_int64().value;
    invItem.wear = elem["item"].get_document().value["wear"].get_int64().value;

    tItem.tradeId = elem["tradeId"].get_int64().value;
    tItem.item = invItem;
    tItem.sellerId = elem["sellerId"].get_int64().value;
    tItem.addTime = elem["addTime"].get_int64().value;
    tItem.hasPassword = elem["hasPassword"].get_bool().value;
    tItem.password = elem["password"].get_utf8().value.data();
    tItem.price = elem["price"].get_int64().value;
    container.insert({ tItem.tradeId, tItem });
}

std::map<LL, tradeData> bg_trade_get_items(const bool &use_cache) {
    if (allTradeItems_cache && use_cache)
        return allTradeItems;

    std::scoped_lock lock(mutexAllTradeItems);
    for (const auto &doc : dbFindAll(DB_COLL_TRADE)) {
        tradeMapFromBson(doc, allTradeItems);
    }
    allTradeItems_cache = true;
    return allTradeItems;
}

bool bg_trade_insert_item(const tradeData &itemData) {
    // 必须有缓存才能继续
    if (!allTradeItems_cache) {
        bg_trade_get_items();
        if (!allTradeItems_cache)
            return false;
    }

    // 检查重复 ID
    if (allTradeItems.find(itemData.tradeId) != allTradeItems.end())
        return false;

    // 更新数据库, 成功后再更新本地缓存
    bsoncxx::document::value doc = bsoncxx::builder::stream::document{}
        << "tradeId" << itemData.tradeId
        << "item" << bsoncxx::builder::stream::open_document
            << "id" << itemData.item.id
            << "level" << itemData.item.level
            << "wear" << itemData.item.wear
        << bsoncxx::builder::stream::close_document
        << "sellerId" << itemData.sellerId
        << "addTime" << itemData.addTime
        << "hasPassword" << itemData.hasPassword
        << "password" << itemData.password
        << "price" << itemData.price
        << bsoncxx::builder::stream::finalize;

    std::scoped_lock lock(mutexAllTradeItems);
    if (dbInsertDocument(DB_COLL_TRADE, doc)) {
        allTradeItems.insert({ itemData.tradeId, itemData });

        std::scoped_lock lock(mutexTradeStr);       // 清除缓存
        tradeStr_cache = false;

        return true;
    }
    return false;
}

bool bg_trade_remove_item(const LL &tradeId) {
    // 必须有缓存才能继续
    if (!allTradeItems_cache) {
        bg_trade_get_items();
        if (!allTradeItems_cache)
            return false;
    }

    // 检查 ID 是否存在
    if (allTradeItems.find(tradeId) == allTradeItems.end())
        return false;

    // 更新数据库, 成功后再更新本地缓存
    std::scoped_lock lock(mutexAllTradeItems);
    if (dbDeleteOne(DB_COLL_TRADE, "tradeId", tradeId)) {
        allTradeItems.erase(tradeId);

        std::scoped_lock lock(mutexTradeStr);       // 清除缓存
        tradeStr_cache = false;

        return true;
    }
    return false;
}

bool bg_trade_remove_item(const std::vector<LL> &tradeIdList) {
    // 必须有缓存才能继续
    if (!allTradeItems_cache) {
        bg_trade_get_items();
        if (!allTradeItems_cache)
            return false;
    }

    // 检查 ID 是否存在, 并生成过滤条件
    bsoncxx::builder::basic::array conditions;
    for (const auto &id : tradeIdList) {
        if (allTradeItems.find(id) != allTradeItems.end())
            return false;
        conditions.append(bsoncxx::builder::stream::document{} << "tradeId" << id << bsoncxx::builder::stream::finalize);
    }

    // 更新数据库, 成功后再更新本地缓存
    std::scoped_lock lock(mutexAllTradeItems);
    if (dbDeleteAll(DB_COLL_TRADE, "tradeId",
        bsoncxx::builder::stream::document{} << "$or" << conditions << bsoncxx::builder::stream::finalize)) {
        for (const auto &id : tradeIdList) {
            allTradeItems.erase(id);
        }

        std::scoped_lock lock(mutexTradeStr);       // 清除缓存
        tradeStr_cache = false;

        return true;
    }
    return false;
}

std::string bg_trade_get_string(const bool &use_cache) {
    if (tradeStr_cache && use_cache)
        return tradeStr;

    auto        tradeItems = bg_trade_get_items();
    std::string rtn;

    if (tradeItems.size() == 0) {
        rtn = "目前交易场中没有东西!";

        std::scoped_lock lock(mutexTradeStr);       // 存到缓存中
        tradeStr = rtn;
        tradeStr_cache = true;
        return rtn;
    }

    rtn = "---交易物品 (共" + std::to_string(tradeItems.size()) +"个)---\n";
    for (const auto &item : tradeItems) {
        // 检查是否为一次性物品
        rtn += "ID" + std::to_string(item.first) + ": ";
        if (allEquipments.at(item.second.item.id).type == EqiType::single_use) {
            rtn += "[" + allEquipments.at(item.second.item.id).name + "]";
        }
        else {
            rtn += allEquipments.at(item.second.item.id).name + "+" + std::to_string(item.second.item.level) + ", " +
                std::to_string(item.second.item.wear) + "/" + std::to_string(allEquipments.at(item.second.item.id).wear);
        }
        
        // 加上价格, 检查是否为私密交易
        rtn += " $" + std::to_string(item.second.price);
        if (item.second.hasPassword) {
            rtn += " (私)";
        }
        rtn += "\n";
    }
    rtn.pop_back();                             // 去掉末尾多余的 '\n'

    std::scoped_lock lock(mutexTradeStr);       // 存到缓存中
    tradeStr = rtn;
    tradeStr_cache = true;

    return rtn;
}
