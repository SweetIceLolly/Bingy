/*
描述: Bingy 装备类
作者: 冰棍
文件: equipment.hpp
*/

#pragma once

#include <string>
#include "inventory.hpp"

#define DEFAULT_EQI_PATH    "equipments.txt"

using LL = long long;

extern std::string eqiConfigPath;

// 装备类型
enum class EqiType {
    // 护甲类
    armor_helmet,       // 头盔
    armor_body,         // 战甲
    armor_leg,          // 护腿
    armor_boot,         // 战靴
    
    // 武器类
    weapon_primary,     // 主武器
    weapon_secondary,   // 服务器

    // 饰品类
    ornament_earrings,  // 耳环
    ornament_rings,     // 戒指
    ornament_necklace,  // 项链
    ornament_jewelry,   // 宝石

    // 一次性
    single_use
};

class equipmentData {
public:
    LL          id;
    std::string name;
    LL          atk, def, brk, agi, hp, mp, crt;    // 攻防破敏血魔暴
    LL          price;                              // 出售价格
};
