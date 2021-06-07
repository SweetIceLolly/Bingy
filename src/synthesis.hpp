/*
描述: Bingy 装备合成相关接口
作者: 冰棍
文件: synthesis.hpp
*/

#pragma once

#include <vector>

using LL = long long;

class synthesisInfo {
public:
    std::vector<LL> requirements;       // 所需装备
    LL              coins;              // 所需硬币
    LL              targetId;           // 合成目标 ID
};

extern std::vector<synthesisInfo>   allSynthesises;
