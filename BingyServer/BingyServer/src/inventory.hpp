/*
描述: Bingy 背包类
作者: 冰棍
文件: inventory.hpp
*/

#pragma once

#include "equipment.hpp"
#include <cmath>

// 懒人宏
// 装备属性 = 默认属性 * 1.14 ^ 装备等级 * (1 - 0.35 * (磨损度 / 原始磨损))
// 通过装备的等级来计算对应的属性值
// 计算之后会把结果存入静态变量, 只有在 calc_属性名_cache 为 false 的时候重新计算
#define CALC_EQI_PROP(prop)             \
    if (id == -1)                       \
        return 0;                       \
    static double calc_result = 0;      \
    if (!calc_##prop##_cache) {         \
        calc_result = allEquipments.at(id). prop * pow(1.14, level) * (1 - 0.35 * (wear / allEquipments.at(id).wear));   \
        calc_##prop##_cache = true;     \
    }                                   \
    return calc_result;
        

using LL = std::int64_t;

class inventoryData {
public:
    LL  id;             // 物品 ID, -1 代表空
    LL  level;          // 等级, -1 代表一次性物品
    LL  wear;           // 磨损, -1 代表一次性物品

    // 装备的攻防破敏血魔暴
    bool calc_atk_cache = false;
    double calc_atk() { CALC_EQI_PROP(atk) }

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
