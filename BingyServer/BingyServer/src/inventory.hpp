/*
����: Bingy ������
����: ����
�ļ�: inventory.hpp
*/

#pragma once

#include "equipment.hpp"
#include <cmath>

// ���˺�
// װ������ = Ĭ������ * 1.14 ^ װ���ȼ� * (1 - 0.35 * (ĥ��� / ԭʼĥ��))
// ͨ��װ���ĵȼ��������Ӧ������ֵ
// ����֮���ѽ�����뾲̬����, ֻ���� calc_������_cache Ϊ false ��ʱ�����¼���
#define CALC_EQI_PROP(prop)             \
    if (id == -1)                       \
        return 0;                       \
    static double calc_result = 0;      \
    if (!calc_##prop##_cache) {         \
        calc_result = allEquipments.at(id).##prop## * pow(1.14, level) * (1 - 0.35 * (wear / allEquipments.at(id).wear));   \
        calc_##prop##_cache = true;     \
    }                                   \
    return calc_result;
        

using LL = long long;

class inventoryData {
public:
    LL  id;             // ��Ʒ ID, -1 �����
    LL  level;          // �ȼ�, -1 ����һ������Ʒ
    LL  wear;           // ĥ��, -1 ����һ������Ʒ

    // װ���Ĺ�������Ѫħ��
    bool calc_atk_cache = false;
    double calc_atk() {
        if (id == -1)
            return 0;
        static double calc_result = 0;
        if (!calc_atk_cache) {
            calc_result = allEquipments.at(id).atk * pow(1.14, level) * (1 - 0.35 * (wear / allEquipments.at(id).wear));
            calc_atk_cache = true;
        }
        return calc_result;
    }

    bool calc_def_cache = false;
    double calc_def() { CALC_EQI_PROP(def) };

    bool calc_brk_cache = false;
    double calc_brk() { CALC_EQI_PROP(brk) };

    bool calc_agi_cache = false;
    double calc_agi() { CALC_EQI_PROP(agi) };

    bool calc_hp_cache = false;
    double calc_hp() { CALC_EQI_PROP(hp) };

    bool calc_mp_cache = false;
    double calc_mp() { CALC_EQI_PROP(mp) };

    bool calc_crt_cache = false;
    double calc_crt() { CALC_EQI_PROP(crt) };

    void resetCache() {
        calc_atk_cache = false;
        calc_def_cache = false;
        calc_brk_cache = false;
        calc_agi_cache= false;
        calc_hp_cache = false;
        calc_mp_cache = false;
        calc_crt_cache = false;
    };
};
