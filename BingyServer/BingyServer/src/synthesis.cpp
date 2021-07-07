/*
����: Bingy װ���ϳ���ز���
����: ����
�ļ�: synthesis.cpp
*/

#include "synthesis.hpp"

std::unordered_multimap<LL, synthesisInfo>   allSyntheses;

bool bg_match_synthesis(const LL &targetId, const std::unordered_multiset<LL> &materials, LL &coins) {
    const auto result = allSyntheses.equal_range(targetId);       // result = (begin, end)
    for (auto it = result.first; it != result.second; ++it) {       // it = (key, content)
        if (materials != it->second.requirements)                   // ����װ����ƥ�����Թ�
            continue;
        coins = it->second.coins;
        return true;
    }
    return false;
}
