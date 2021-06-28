/*
描述: Bingy 装备合成相关接口
作者: 冰棍
文件: synthesis.hpp
*/

#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

using LL = long long;

class synthesisInfo {
public:
    std::unordered_multiset<LL> requirements;       // 所需装备
    LL                          coins;              // 所需硬币
    LL                          targetId;           // 合成目标 ID
};

extern std::unordered_multimap<LL, synthesisInfo>   allSynthesises;

// 根据所提供的条件来配对可用的合成. 如果有符合条件的合成, 则返回 true, coins 为所需硬币; 否则返回 false; coins 的值不被修改
bool bg_match_synthesis(const LL &targetId, const std::unordered_multiset<LL> &materials, LL &coins);
