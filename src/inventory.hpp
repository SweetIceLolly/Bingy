/*
描述: Bingy 背包类
作者: 冰棍
文件: inventory.hpp
*/

#pragma once

using LL = long long;

class inventoryData {
public:
    LL      id;             // 物品 ID, -1 代表空
    LL      level;          // 等级, -1 代表一次性物品
    LL      wear;           // 磨损, -1 代表一次性物品
};
