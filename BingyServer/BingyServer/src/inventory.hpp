/*
描述: Bingy 背包类
作者: 冰棍
文件: inventory.hpp
*/

#pragma once

#include "equipment.hpp"
#include <cmath>

// 懒人宏
// 装备属性 = 默认属性 * 1.14 ^ 装备等级 * (0.65 + 0.35 * 磨损 / 原始磨损)
// 通过装备的等级来计算对应的属性值
// 计算之后会把结果存入静态变量, 只有在 calc_属性名_cache 为 false 的时候重新计算
#define CALC_EQI_PROP(prop)             \
    if (id == -1)                       \
        return 0;                       \
    if (!calc_##prop##_cache) {         \
        prop##_cache = allEquipments.at(id). prop * pow(1.14, level) * (0.65 + 0.35 * wear / allEquipments[id].wear);   \
        calc_##prop##_cache = true;     \
    }                                   \
    return prop##_cache;
        

using LL = std::int64_t;

class inventoryData {
public:
    LL  id;             // 物品 ID, -1 代表空
    LL  level;          // 等级, -1 代表一次性物品
    LL  wear;           // 磨损, -1 代表一次性物品

    // 装备的攻防破敏血魔暴
    bool calc_atk_cache = false;
    double atk_cache = 0;
    double calc_atk() { CALC_EQI_PROP(atk) }

    bool calc_def_cache = false;
    double def_cache = 0;
    double calc_def() { CALC_EQI_PROP(def) };

    bool calc_brk_cache = false;
    double brk_cache = 0;
    double calc_brk() { CALC_EQI_PROP(brk) };

    bool calc_agi_cache = false;
    double agi_cache = 0;
    double calc_agi() { CALC_EQI_PROP(agi) };

    bool calc_hp_cache = false;
    double hp_cache = 0;
    double calc_hp() { CALC_EQI_PROP(hp) };

    bool calc_mp_cache = false;
    double mp_cache = 0;
    double calc_mp() { CALC_EQI_PROP(mp) };

    bool calc_crt_cache = false;
    double crt_cache = 0;
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
