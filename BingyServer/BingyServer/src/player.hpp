/*
����: Bingy �����ز����Ľӿ�
����: ����
�ļ�: player.hpp
*/

#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <list>
#include "inventory.hpp"
#include "equipment.hpp"

#define INV_DEFAULT_CAPACITY 50                                                     // Ĭ�ϱ�������

// ���˺�
// ���� LL ���͵����Ե� getter, setter, �� inc (������ֵ)�ĺ���ԭ��
#define DEF_LL_GET_SET_INC(propName)                            \
    LL get_##propName##(const bool &use_cache = true);          \
    bool set_##propName##(const LL &val);                       \
    bool inc_##propName##(const LL &val);                       \

using LL = long long;

class player {
private:
    LL id;                                                                          // QQ ��

    // ���ǿ����ȷ��״̬
    std::mutex mutexStatus;                                                         // �߳�״̬��
    bool upgrading;                                                                 // ����Ƿ�ȷ��Ҫǿ��
    std::condition_variable cvStatusChange;                                         // ״̬�����仯
    std::condition_variable cvPrevConfirmCompleted;                                 // ��һ�β����Ƿ����

public:
    std::mutex mutexPlayer;                                                         // ��Ҳ�����

    // [ע��] ����������ʹ�ö�Ӧ�� getter �� setter. ������֪��������ʲô, ����Ҫֱ�Ӷ�д���ǵ�ֵ
    std::string nickname;                   bool nickname_cache = false;            // �ǳ� (��ʹ�ö�Ӧ�� getter �� setter)
    LL signInCount;                         bool signInCount_cache = false;         // ǩ������ (��ʹ�ö�Ӧ�� getter �� setter)
    LL signInCountCont;                     bool signInCountCont_cache = false;     // ����ǩ������ (��ʹ�ö�Ӧ�� getter �� setter)
    LL lastFight;                           bool lastFight_cache = false;           // ��һ�δ��ʱ��. Ϊ UNIX ʱ���, ��λΪ�� (��ʹ�ö�Ӧ�� getter �� setter)
    LL lastSignIn;                          bool lastSignIn_cache = false;          // ��һ��ǩ��ʱ��. Ϊ UNIX ʱ���, ��λΪ�� (��ʹ�ö�Ӧ�� getter �� setter)
    LL coins;                               bool coins_cache = false;               // Ӳ���� (��ʹ�ö�Ӧ�� getter �� setter)
    LL heroCoin;                            bool heroCoin_cache = false;            // Ӣ�۱��� (��ʹ�ö�Ӧ�� getter �� setter)
    LL level;                               bool level_cache = false;               // �ȼ� (��ʹ�ö�Ӧ�� getter �� setter)
    LL blessing;                            bool blessing_cache = false;            // ף�� (��ʹ�ö�Ӧ�� getter �� setter)
    LL energy;                              bool energy_cache = false;              // ���� (��ʹ�ö�Ӧ�� getter �� setter)
    LL exp;                                 bool exp_cache = false;                 // ������ (��ʹ�ö�Ӧ�� getter �� setter)
    LL invCapacity;                         bool invCapacity_cache = false;         // �������� (��ʹ�ö�Ӧ�� getter �� setter)
    std::list<inventoryData> inventory;     bool inventory_cache = false;           // ���� (��ʹ�ö�Ӧ�� getter �� setter)
    std::unordered_map<LL, LL> buyCount;    bool buyCount_cache = false;            // ��Ʒ������� (��Ʒ ID -> ����) (��ʹ�ö�Ӧ�Ĳ�������)
    std::unordered_map<EqiType,
        inventoryData> equipments;          bool equipments_cache = false;          // ��װ����װ�� (װ������ -> inventoryData) (��ʹ�ö�Ӧ�Ĳ�������)
    std::list<inventoryData> equipItems;    bool equipItems_cache = false;          // ��װ����һ������Ʒ (��ʹ�ö�Ӧ�Ĳ�������)
    LL vip;                                 bool vip_cache = false;                 // VIP (��ʹ�ö�Ӧ�� getter �� setter)

    // ---------------------------------------------------------

    // Ĭ�Ϲ��캯��
    player();

    // ���ƹ��캯��
    player(const player &p);

    // ָ�� QQ �ŵĹ��캯��
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

    // ��ȡ�������
    bool atk_cache = false;
    double get_atk();           // ��

    bool def_cache = false;
    double get_def();           // ��

    bool brk_cache = false;
    double get_brk();           // ��

    bool agi_cache = false;
    double get_agi();           // ��

    bool hp_cache = false;
    double get_hp();            // Ѫ

    bool mp_cache = false;
    double get_mp();            // ħ

    bool crt_cache = false;
    double get_crt();           // ��

    LL get_exp_needed();        // �������辭��
    LL get_cd();                // ��ȴʱ��

    void resetCache();

    // ��ȡ���������б�
    std::list<inventoryData> get_inventory(const bool &use_cache = true);
    // ��ȡ����װ������
    LL get_inventory_size(const bool &use_cache = true);
    // ����ָ������Ƴ�������Ʒ. ���ָ�������Ч, �򷵻� false. ע��, ָ����ű���� 0 ��ʼ
    bool remove_at_inventory(const LL &index);
    // ����ָ��������б��Ƴ�������Ʒ. ָ������Ų����ظ�. ���ָ�������Ч, �򷵻� false. ע��, ָ����ű���� 0 ��ʼ
    bool remove_at_inventory(const std::vector<LL> &indexes);
    // �������Ʒ������ĩβ
    bool add_inventory_item(const inventoryData &item);
    // �������������б�
    bool set_inventory(const std::list<inventoryData> &val);

    // ��ȡ�������������
    std::unordered_map<LL, LL> get_buyCount(const bool &use_cache = true);
    // ��ȡ�����������ĳ����Ʒ�Ĺ������. ����Ҳ�����Ӧ����Ʒ�����¼, �򷵻� 0
    LL get_buyCount_item(const LL &id, const bool &use_cache = true);
    // ���ù����������ĳ����Ʒ�Ĺ������. �����Ӧ��Ʒ�Ĺ����¼������, ��ᴴ��
    bool set_buyCount_item(const LL &id, const LL &count);

    // ��ȡ������װ����װ����
    std::unordered_map<EqiType, inventoryData> get_equipments(const bool &use_cache = true);
    // ��ȡĳ�����͵�װ��
    inventoryData get_equipments_item(const EqiType &type, const bool &use_cache = true);
    // ����ĳ�����͵�װ��. ���Ҫ�Ƴ�, ��� item �� id ����Ϊ -1
    bool set_equipments_item(const EqiType &type, const inventoryData &item);

    // ��ȡ������װ����һ������Ʒ��
    std::list<inventoryData> get_equipItems(const bool &use_cache = true);
    // ��ȡ��װ����һ������Ʒ����
    LL get_equipItems_size(const bool &use_cache = true);
    // �Ƴ�ĳ����װ����һ������Ʒ. ���ָ�������Ч, �򷵻� false. ע��, ָ����ű���� 0 ��ʼ
    bool remove_at_equipItems(const LL &index);
    // �����װ����һ������Ʒ
    bool clear_equipItems();
    // �������Ʒ����װ����һ������Ʒ�б�ĩβ
    bool add_equipItems_item(const inventoryData &item);

    bool confirmInProgress = false;                                                 // �Ƿ��д�ȷ�ϵ�ǿ��
    // ȡ��ǿ��ȷ��
    void abortUpgrade();
    // ȷ��ǿ��ȷ��
    void confirmUpgrade();
    // �ȴ�ǿ��ȷ��. ������ȷ����ǿ��, �ͷ��� true; ���򷵻� false
    bool waitUpgradeConfirm();
    // �ȴ�ȷ�����
    void waitConfirmComplete();
};

extern std::unordered_map<long long, player>   allPlayers;
extern std::mutex                              mutexAllPlayers;

bool bg_player_exist(const LL &id);
bool bg_player_add(const LL &id);
bool bg_get_allplayers_from_db();

bool bg_all_player_inc_coins(const LL &val);
bool bg_all_player_inc_heroCoin(const LL &val);
bool bg_all_player_inc_level(const LL &val);
bool bg_all_player_inc_blessing(const LL &val);
bool bg_all_player_inc_energy(const LL &val);
bool bg_all_player_inc_exp(const LL &val);
bool bg_all_player_inc_invCapacity(const LL &val);
bool bg_all_player_inc_vip(const LL &val);
