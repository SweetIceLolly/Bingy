/*
描述: Bingy 装备类
作者: 冰棍
文件: equipment.hpp
*/

#pragma once

#include <string>
#include <unordered_map>

using LL = std::int64_t;

// 装备类型
enum class EqiType : unsigned char {
    // 护甲类
    armor_helmet = 0U,          // 头盔
    armor_body = 1U,            // 战甲
    armor_leg = 2U,             // 护腿
    armor_boot = 3U,            // 战靴
    
    // 武器类
    weapon_primary = 4U,        // 主武器
    weapon_secondary = 5U,      // 副武器

    // 饰品类
    ornament_earrings = 6U,     // 耳环
    ornament_rings = 7U,        // 戒指
    ornament_necklace = 8U,     // 项链
    ornament_jewelry = 9U,      // 宝石

    // 一次性
    single_use = 10U
};

class equipmentData {
public:
    LL              id;
    EqiType         type;                               // 装备类型
    std::string     name;
    LL              atk, def, brk, agi, hp, mp, crt;    // 原始攻防破敏血魔暴 (请使用 inventoryData 中对应的 getter 来获取计算后的数据)
    LL              wear;                               // 原始磨损
    LL              price;                              // 出售价格
};

extern std::string                             eqiConfigPath;
extern std::unordered_map<LL, equipmentData>   allEquipments;

bool bg_load_equipment_config();

// 根据字符串来局部匹配装备名称
std::vector<std::tuple<LL, std::string, EqiType>> bg_search_equipment(const std::string &str);
