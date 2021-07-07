/*
����: Bingy �����ز���
����: ����
�ļ�: player.cpp
*/

#include "player.hpp"
#include "database.hpp"
#include <chrono>               // ����ȷ���̳߳�ʱ

#define LOCK_CURR_PLAYER std::scoped_lock<std::mutex> __lock(this->mutexPlayer);

std::unordered_map<LL, player>  allPlayers;         // ע��: ��ȡ��ʱ����Բ��ü���, ���ǲ�Ҫʹ��[], ��Ҫʹ�� at(). ���߳�д���ʱ��������
std::mutex                      mutexAllPlayers;

template <typename T> void invListFromBson(const bsoncxx::document::element &elem, T &container);
void eqiMapFromBson(const bsoncxx::document::element &elem, std::unordered_map<EqiType, inventoryData> &container);

// --------------------------------------------------
// ���캯��

player::player() {
    throw std::exception("���� player ����ʱ����ָ�� QQ ��!");
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
// ��Ҳ�������

// �������Ƿ����
bool bg_player_exist(const LL &id) {
    auto val = dbFindOne(DB_COLL_USERDATA, "id", id);
    return allPlayers.end() != allPlayers.find(id);
}

// ���˺�
// ����������� LL ���͵���������Ϊ0, ���Ѷ�Ӧ�Ļ����ʶ����Ϊ true
#define SET_LL_PROP_ZERO(prop) p.##prop = 0; p.##prop##_cache = true;

// �����������ҵ����ݿ���ֵ���. �׳����쳣��Ҫ���ⲿ����
bool bg_player_add(const LL &id) {
    // ������װ��������Ϊ -1 (��)
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

    // ��ʼ���������
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
    
    // ��ʼ�����װ��
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

// ���˺�
// ����������� LL ���͵���������Ϊ�����ݿ��ж�ȡ��������, ���Ѷ�Ӧ�Ļ����ʶ����Ϊ true
// ˳��ѿ��ܷ������쳣����һ��
#define SET_LL_PROP(prop)                       \
    tmp = doc[#prop];                           \
    if (tmp) {                                  \
        p.##prop = tmp.get_int64().value;       \
        p.##prop##_cache = true;                \
    }                                           \
    else                                        \
        throw std::exception("��ȡ�������ʧ��");

// �����ݿ��ȡ�������
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

            // ��ȡ��������
            tmp = doc["inventory"];
            if (tmp)
                invListFromBson(tmp, p.inventory);
            else
                throw std::exception(("��ȡ���" + std::to_string(id) + "�� inventory ����ʧ��").c_str());

            // ��ȡ���װ��
            tmp = doc["equipments"];
            if (tmp)
                eqiMapFromBson(tmp, p.equipments);
            else
                throw std::exception(("��ȡ���" + std::to_string(id) + "�� equipments ����ʧ��").c_str());

            // ��ȡһ����װ��
            tmp = doc["equipItems"];
            if (tmp)
                invListFromBson(tmp, p.equipItems);
            else
                throw std::exception(("��ȡ���" + std::to_string(id) + "�� equipItems ����ʧ��").c_str());

            // todo
            //p.buyCount

            allPlayers.insert(std::make_pair(id, p));
        }
        return true;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
    }
    catch (...) {
        return false;
    }
}

// --------------------------------------------------
// �������Ե� getter �� setter

// ���˺�
// �����д LL �������Ե� getter �� setter. ���а���:
// 1. ��ȡĳ������; 2. ����ĳ������; 3. Ϊĳ���������ָ����ֵ
#define LL_GET_SET_INC(propName)                                                \
    LL player::get_##propName##(const bool &use_cache) {                        \
        if (##propName##_cache && use_cache)                                    \
            return this->##propName##;                                          \
                                                                                \
        auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, #propName);   \
        if (!result)                                                            \
            throw std::exception("�Ҳ�����Ӧ����� ID");                         \
        auto field = result->view()[#propName];                                 \
        if (!field.raw())                                                       \
            throw std::exception("û���ҵ� " #propName " field");                \
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

// ��� ID
LL player::get_id() {
    return this->id;
}

// ��ȡ�ǳ�
std::string player::get_nickname(const bool &use_cache) {
    if (nickname_cache && use_cache)
        return this->nickname;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "nickname");
    if (!result)
        throw std::exception("�Ҳ�����Ӧ����� ID");
    auto field = result->view()["nickname"];
    if (!field.raw())
        throw std::exception("û���ҵ� nickname field");
    auto tmp = std::string(result->view()["nickname"].get_utf8().value);

    LOCK_CURR_PLAYER;
    this->nickname = tmp;
    this->nickname_cache = true;
    return tmp;
}

// �����ǳ�
bool player::set_nickname(const std::string &val) {
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "nickname", val)) {
        this->nickname = val;
        this->nickname_cache = true;
        return true;
    }
    return false;
}

// �������� BSON ����, ����������ӵ�ָ����������
// ע��, �ú������� elem �ǺϷ����Ҵ���װ������
template <typename T>
void invListFromBson(const bsoncxx::document::element &elem, T &container) {
    bsoncxx::array::view tmpArray{ elem.get_array().value };   // ���� BSON ����

    // �����ʽ: [{id: x, level: x, wear: x}, {id: x, level: x, wear: x}, ...]
    inventoryData invItem;
    for (const auto &item : tmpArray) {
        auto tmpObj = item.get_document().view();

        invItem.id = tmpObj["id"].get_int64().value;            // װ�� ID
        invItem.level = tmpObj["level"].get_int64().value;      // װ���ȼ�
        invItem.wear = tmpObj["wear"].get_int64().value;        // װ��ĥ���
        container.push_back(invItem);
    }
}

// ��ȡ���������б�
std::list<inventoryData> player::get_inventory(const bool &use_cache) {
    if (inventory_cache && use_cache)
        return inventory;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "inventory");
    if (!result)
        throw std::exception("�Ҳ�����Ӧ��� ID");
    auto field = result->view()["inventory"];
    if (!field.raw())
        throw std::exception("û���ҵ� inventory field");

    std::list<inventoryData> rtn;
    invListFromBson(field, rtn);
    LOCK_CURR_PLAYER;
    this->inventory = rtn;
    this->inventory_cache = true;
    return rtn;
}

// ��ȡ����װ������
LL player::get_inventory_size(const bool &use_cache) {
    if (inventory_cache && use_cache)
        return inventory.size();
    return get_inventory().size();
}

// ����ָ������Ƴ�������Ʒ. ���ָ�������Ч, �򷵻� false
bool player::remove_at_inventory(const LL &index) {
    // �����л�����ܼ���
    if (!inventory_cache) {
        get_inventory();
        if (!inventory_cache)
            return false;
    }

    // �������Ƿ���Ч
    if (index >= inventory.size())
        return false;

    // �������ݿ�, �ɹ����ٸ��±��ػ���
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

// ����ָ��������б��Ƴ�������Ʒ. ָ������Ų����ظ�. ���ָ�������Ч, �򷵻� false
bool player::remove_at_inventory(const std::vector<LL> &indexes) {
    // �����л�����ܼ���
    if (!inventory_cache) {
        get_inventory();
        if (!inventory_cache)
            return false;
    }

    // �������Ƿ���Ч, ����ӵ� document ��
    bsoncxx::builder::basic::document doc;
    for (const auto &index : indexes) {
        if (index >= inventory.size())
            return false;
        doc.append(bsoncxx::builder::basic::kvp("inventory." + std::to_string(index), bsoncxx::types::b_null()));
    }

    // �������ݿ�, �ɹ����ٸ��±��ػ���
    LOCK_CURR_PLAYER;
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set", doc)) {
        if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$pu",
            bsoncxx::builder::stream::document{} << "inventory" << bsoncxx::types::b_null()
            << bsoncxx::builder::stream::finalize)) {

            std::vector<LL> sortedIndexes = indexes;
            std::sort(sortedIndexes.rbegin(), sortedIndexes.rend());    // ����ŴӴ�С����

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

// �������Ʒ������ĩβ
bool player::add_inventory_item(const inventoryData &item) {
    // �����л�����ܼ���
    if (!inventory_cache) {
        get_inventory();
        if (!inventory_cache)
            return false;
    }

    // �������ݿ�, �ɹ����ٸ��±��ػ���
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

// �������������б�
bool player::set_inventory(const std::list<inventoryData> &val) {
    // �ѱ���������ӵ� document ��
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
    
    // �������ݿ�, �ɹ����ٸ��±��ػ���
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

// ��ȡ�������������
std::unordered_map<LL, LL> player::get_buyCount(const bool &use_cache) {
    return buyCount;
}

// ��ȡ�����������ĳ����Ʒ�Ĺ������. ����Ҳ�����Ӧ����Ʒ�����¼, �򷵻� 0
LL player::get_buyCount_item(const LL &id, const bool &use_cache) {
    return 0;
}

// ���ù����������ĳ����Ʒ�Ĺ������. �����Ӧ��Ʒ�Ĺ����¼������, ��ᴴ��
bool player::set_buyCount_item(const LL &id, const LL &count) {
    return false;
}

// ����װ���� BSON ����, ����������ӵ�ָ����������
// ע��, �ú������� elem �ǺϷ����Ҵ�����װ����װ������. �����������ȡ��װ����һ����װ��
void eqiMapFromBson(const bsoncxx::document::element &elem, std::unordered_map<EqiType, inventoryData> &container) {
    // ���ݸ�ʽ: equipments: {1: {id: x, level: x, wear: x}, ...}
    auto tmp = elem.get_document().view();
    for (const auto &it : tmp) {
        if (it.type() == bsoncxx::type::k_document) {         // ֻ���� object ���͵�Ԫ��
            inventoryData invData;
            invData.id = it.get_document().value["id"].get_int64().value;
            invData.level = it.get_document().value["level"].get_int64().value;
            invData.wear = it.get_document().value["wear"].get_int64().value;
            container.insert({ static_cast<EqiType>(std::stoll(it.key().data())), invData });
        }
    }
}

// ��ȡ������װ����װ����
std::unordered_map<EqiType, inventoryData> player::get_equipments(const bool &use_cache) {
    if (equipments_cache && use_cache)
        return equipments;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "equipments");
    if (!result)
        throw std::exception("�Ҳ�����Ӧ��� ID");
    auto field = result->view()["equipments"];
    if (!field.raw())
        throw std::exception("û���ҵ� equipments field");

    std::unordered_map<EqiType, inventoryData> rtn;
    eqiMapFromBson(field, rtn);
    LOCK_CURR_PLAYER;
    this->equipments = rtn;
    this->equipments_cache = true;
    return rtn;
}

// ��ȡĳ�����͵�װ��
inventoryData player::get_equipments_item(const EqiType &type, const bool &use_cache) {
    return get_equipments(use_cache)[type];
}

// ����ĳ�����͵�װ��. �����Ҫ�Ƴ�ĳ�����͵�װ��, ��� item �� id ����Ϊ -1
bool player::set_equipments_item(const EqiType &type, const inventoryData &item) {
    // �����л�����ܼ���
    if (!equipments_cache) {
        get_equipments();
        if (!equipments_cache)
            return false;
    }

    // �������ݿ�, �ɹ����ٸ��±��ػ���
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

// ��ȡ������װ����һ������Ʒ��
std::list<inventoryData> player::get_equipItems(const bool &use_cache) {
    if (equipItems_cache && use_cache)
        return equipItems;

    auto result = dbFindOne(DB_COLL_USERDATA, "id", this->id, "equipItems");
    if (!result)
        throw std::exception("�Ҳ�����Ӧ��� ID");
    auto field = result->view()["equipItems"];
    if (!field.raw())
        throw std::exception("û���ҵ� equipItems field");

    std::list<inventoryData> rtn;
    invListFromBson(field, rtn);
    LOCK_CURR_PLAYER;
    this->equipItems = rtn;
    this->equipItems_cache = true;
    return rtn;
}

// ��ȡ��װ����һ������Ʒ����
LL player::get_equipItems_size(const bool &use_cache) {
    if (equipItems_cache && use_cache)
        return equipItems.size();
    return get_equipItems().size();
}

// �Ƴ�ĳ����װ����һ������Ʒ. ���ָ�������Ч, �򷵻� false
bool player::remove_at_equipItems(const LL &index) {
    // �����л�����ܼ���
    if (!equipItems_cache) {
        get_equipItems();
        if (!equipItems_cache)
            return false;
    }

    // �������Ƿ���Ч
    if (index >= equipItems.size())
        return false;

    // �������ݿ�, �ɹ����ٸ��±��ػ���
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

// �����װ����һ������Ʒ
bool player::clear_equipItems() {
    // �����л�����ܼ���
    if (!equipItems_cache) {
        get_equipItems();
        if (!equipItems_cache)
            return false;
    }

    // �������ݿ�, �ɹ����ٸ��±��ػ���
    if (dbUpdateOne(DB_COLL_USERDATA, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "equipItems" <<
        bsoncxx::builder::stream::open_array << bsoncxx::builder::stream::close_array
        << bsoncxx::builder::stream::finalize)) {

        this->equipItems.clear();
        return true;
    }
    return false;
}

// �������Ʒ����װ����һ������Ʒ�б�ĩβ
bool player::add_equipItems_item(const inventoryData &item) {
    // �����л�����ܼ���
    if (!equipItems_cache) {
        get_equipItems();
        if (!equipItems_cache)
            return false;
    }

    // �������ݿ�, �ɹ����ٸ��±��ػ���
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
// ���ս������

// �� = 20 + ����װ�����ܺ� + �ȼ� * 1.1 + ף�� * 2
double player::get_atk() {
    static double calc_result = 20;
    if (!atk_cache) {
        // -----------------------------------
        // ���¼���
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

// �� = 20 + ����װ�����ܺ� + �ȼ� * 0.6 + ף�� * 1.5
double player::get_def() {
    static double calc_result = 20;
    if (!def_cache) {
        // -----------------------------------
        // ���¼���
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

// �� = �����������ܺ�
double player::get_brk() {
    static double calc_result = 0;
    if (!brk_cache) {
        // -----------------------------------
        // ���¼���
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

// �� = 10 + ����װ�����ܺ� + ף�� * 0.2
double player::get_agi() {
    static double calc_result = 0;
    if (!agi_cache) {
        // -----------------------------------
        // ���¼���
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

// Ѫ = 100 + ����װ��Ѫ�ܺ� + ��ҵȼ� * ף�� / 10 + ף��
double player::get_hp() {
    static double calc_result = 0;
    if (!hp_cache) {
        // -----------------------------------
        // ���¼���
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

// ħ = ����װ��ħ�ܺ� + ף�� * 1.7 + ��ҵȼ� * 1.7
double player::get_mp() {
    static double calc_result = 0;
    if (!mp_cache) {
        // -----------------------------------
        // ���¼���
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

// �� = ����װ�����ܺ� * (1 + ��ҵȼ� / 170)
double player::get_crt() {
    static double calc_result = 0;
    if (!crt_cache) {
        // -----------------------------------
        // ���¼���
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

// �������辭�� = 100 + ��ҵȼ� * 10 + 8 * 1.18 ^ ��ҵȼ�
LL player::get_exp_needed() {
    return (LL)(100.0 + level * 10.0 + 8 * pow(1.18, level));
}

// ��ȴʱ�� = Min(200 - ��ҵȼ� * 1.2, 40)
LL player::get_cd() {
    LL cd = (LL)(200.0 - level * 1.2);
    if (cd < 40)
        return 40;
    else
        return cd;
}

// ��ռ��㻺��
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

// ȡ��ǿ��ȷ��
void player::abortUpgrade() {
    upgrading = false;
    cvStatusChange.notify_one();
}

// ȷ��ǿ��ȷ��
void player::confirmUpgrade() {
    upgrading = true;
    cvStatusChange.notify_one();
}

// �ȴ�ǿ��ȷ��. ������ȷ����ǿ��, �ͷ��� true; ���򷵻� false
bool player::waitUpgradeConfirm() {
    confirmInProgress = true;
    upgrading = false;
    std::unique_lock lock(this->mutexStatus);
    cvStatusChange.wait_for(lock, std::chrono::seconds(20));    // 20 ��ȷ�ϳ�ʱ
    confirmInProgress = false;
    cvPrevConfirmCompleted.notify_one();
    return this->upgrading;
}

// �ȴ�ȷ�����
void player::waitConfirmComplete() {
    std::unique_lock lock(this->mutexStatus);
    cvPrevConfirmCompleted.wait(lock);
}

// ���˺�
// Ϊ������ҵ�ָ����������ָ����ֵ
#define ALL_PLAYER_INC(field)                                           \
    bool bg_all_player_inc_##field##(const LL &val) {                   \
        /* �������ݿ�, �ɹ����ٸ��±��ػ��� */                            \
        if (dbUpdateAll(DB_COLL_USERDATA, "$inc",                       \
            bsoncxx::builder::stream::document{} << #field << val       \
            << bsoncxx::builder::stream::finalize)) {                   \
                                                                        \
            /* Ϊ����������Ӳ�� */                                     \
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
