/*
����: Bingy װ����
����: ����
�ļ�: equipment.hpp
*/

#pragma once

#include <string>
#include <unordered_map>

using LL = long long;

// װ������
enum class EqiType : unsigned char {
    // ������
    armor_helmet = 0U,          // ͷ��
    armor_body = 1U,            // ս��
    armor_leg = 2U,             // ����
    armor_boot = 3U,            // սѥ
    
    // ������
    weapon_primary = 4U,        // ������
    weapon_secondary = 5U,      // ������

    // ��Ʒ��
    ornament_earrings = 6U,     // ����
    ornament_rings = 7U,        // ��ָ
    ornament_necklace = 8U,     // ����
    ornament_jewelry = 9U,      // ��ʯ

    // һ����
    single_use = 10U
};

class equipmentData {
public:
    LL              id;
    EqiType         type;                               // װ������
    std::string     name;
    LL              atk, def, brk, agi, hp, mp, crt;    // ԭʼ��������Ѫħ�� (��ʹ�� inventoryData �ж�Ӧ�� getter ����ȡ����������)
    LL              wear;                               // ԭʼĥ��
    LL              price;                              // ���ۼ۸�
};

extern std::string                             eqiConfigPath;
extern std::unordered_map<LL, equipmentData>   allEquipments;   // ע��: ��ȡ��ʱ����Բ��ü���, ���ǲ�Ҫʹ��[], ��Ҫʹ�� at(). ���߳�д���ʱ��������

bool bg_load_equipment_config();
