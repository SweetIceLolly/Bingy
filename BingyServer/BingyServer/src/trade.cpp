/*
����: Bingy ���׳���ز���
����: ����
�ļ�: inventory.hpp
*/

#include "trade.hpp"
#include "database.hpp"
#include <sstream>

std::mutex              mutexAllTradeItems;             // ���׳���
std::map<LL, tradeData> allTradeItems;                  // ���н��׳���Ŀ
bool                    allTradeItems_cache = false;    // ���׳���Ŀ����

std::mutex              mutexTradeId;                   // ���� ID ��
LL                      currTradeId;                    // ��ǰ���� ID
bool                    currTradeId_cache = false;      // ���� ID ����

std::mutex              mutexTradeStr;                  // ���׳��ַ���������
std::string             tradeStr;                       // ���׳��ַ�������, �Խ�ʡÿ�����ɽ��׳��ַ����Ŀ���
bool                    tradeStr_cache = false;         // ���׳��ַ���������

bool bg_init_tradeId();                                 // �����ݿ��г�ʼ������ ID ��¼

LL bg_get_tradeId(const bool &use_cache) {
    if (currTradeId_cache && use_cache)
        return currTradeId;

    auto result = dbFindOne(DB_COLL_TRADE, "type", "tradeId", "value");
    if (!result) {
        console_log("û���ڽ��׳����ݿ����ҵ� tradeId ��¼, �������´���...", LogType::warning);
        if (!bg_init_tradeId())
            throw std::exception("���Գ�ʼ������ ID ʧ��");
        console_log("tradeId ������Ϊ 0", LogType::warning);
        return 0;
    }
    auto field = result->view()["value"];
    if (!field.raw()) {
        console_log("û���ڽ��׳����ݿ����ҵ� tradeId ��¼, �������´���...", LogType::warning);
        if (!bg_init_tradeId())
            throw std::exception("���Գ�ʼ������ ID ʧ��");
        console_log("tradeId ������Ϊ 0", LogType::warning);
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

// �����׳���Ŀ�� BSON ����, ����������ӵ�ָ����������
// ע��, �ú������� elem �ǺϷ����Ҵ��н��׳���Ŀ����
void tradeMapFromBson(const bsoncxx::document::view &elem, std::map<LL, tradeData> &container) {
    if (elem.find("type") != elem.end() && elem["type"].get_utf8().value == "tradeId")      // ���Ե���һ������ ID �ļ�¼
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
    // �����л�����ܼ���
    if (!allTradeItems_cache) {
        bg_trade_get_items();
        if (!allTradeItems_cache)
            return false;
    }

    // ����ظ� ID
    if (allTradeItems.find(itemData.tradeId) != allTradeItems.end())
        return false;

    // �������ݿ�, �ɹ����ٸ��±��ػ���
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

        std::scoped_lock lock(mutexTradeStr);       // �������
        tradeStr_cache = false;

        return true;
    }
    return false;
}

bool bg_trade_remove_item(const LL &tradeId) {
    // �����л�����ܼ���
    if (!allTradeItems_cache) {
        bg_trade_get_items();
        if (!allTradeItems_cache)
            return false;
    }

    // ��� ID �Ƿ����
    if (allTradeItems.find(tradeId) == allTradeItems.end())
        return false;

    // �������ݿ�, �ɹ����ٸ��±��ػ���
    std::scoped_lock lock(mutexAllTradeItems);
    if (dbDeleteOne(DB_COLL_TRADE, "tradeId", tradeId)) {
        allTradeItems.erase(tradeId);

        std::scoped_lock lock(mutexTradeStr);       // �������
        tradeStr_cache = false;

        return true;
    }
    return false;
}

bool bg_trade_remove_item(const std::vector<LL> &tradeIdList) {
    // �����л�����ܼ���
    if (!allTradeItems_cache) {
        bg_trade_get_items();
        if (!allTradeItems_cache)
            return false;
    }

    // ��� ID �Ƿ����, �����ɹ�������
    bsoncxx::builder::basic::array conditions;
    for (const auto &id : tradeIdList) {
        if (allTradeItems.find(id) != allTradeItems.end())
            return false;
        conditions.append(bsoncxx::builder::stream::document{} << "tradeId" << id << bsoncxx::builder::stream::finalize);
    }

    // �������ݿ�, �ɹ����ٸ��±��ػ���
    std::scoped_lock lock(mutexAllTradeItems);
    if (dbDeleteAll(DB_COLL_TRADE, "tradeId",
        bsoncxx::builder::stream::document{} << "$or" << conditions << bsoncxx::builder::stream::finalize)) {
        for (const auto &id : tradeIdList) {
            allTradeItems.erase(id);
        }

        std::scoped_lock lock(mutexTradeStr);       // �������
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
        rtn = "Ŀǰ���׳���û�ж���!";

        std::scoped_lock lock(mutexTradeStr);       // �浽������
        tradeStr = rtn;
        tradeStr_cache = true;
        return rtn;
    }

    rtn = "---������Ʒ (��" + std::to_string(tradeItems.size()) +"��)---\n";
    for (const auto &item : tradeItems) {
        // ����Ƿ�Ϊһ������Ʒ
        rtn += "ID" + std::to_string(item.first) + ": ";
        if (allEquipments.at(item.second.item.id).type == EqiType::single_use) {
            rtn += "[" + allEquipments.at(item.second.item.id).name + "]";
        }
        else {
            rtn += allEquipments.at(item.second.item.id).name + "+" + std::to_string(item.second.item.level) + ", " +
                std::to_string(item.second.item.wear) + "/" + std::to_string(allEquipments.at(item.second.item.id).wear);
        }
        
        // ���ϼ۸�, ����Ƿ�Ϊ˽�ܽ���
        rtn += " $" + std::to_string(item.second.price);
        if (item.second.hasPassword) {
            rtn += " (˽)";
        }
        rtn += "\n";
    }
    rtn.pop_back();                             // ȥ��ĩβ����� '\n'

    std::scoped_lock lock(mutexTradeStr);       // �浽������
    tradeStr = rtn;
    tradeStr_cache = true;

    return rtn;
}
