/*
描述: Bingy 装备合成相关操作
作者: 冰棍
文件: synthesis.cpp
*/

#include "synthesis.hpp"

std::unordered_multimap<LL, synthesisInfo>   allSyntheses;

bool bg_match_synthesis(LL targetId, const std::unordered_multiset<LL> &materials, LL &coins) {
    const auto result = allSyntheses.equal_range(targetId);         // result = (begin, end)
    for (auto it = result.first; it != result.second; ++it) {       // it = (key, content)
        if (materials != it->second.requirements)                   // 所需装备不匹配则略过
            continue;
        coins = it->second.coins;
        return true;
    }
    return false;
}
