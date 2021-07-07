/*
����: Bingy װ���ϳ���ؽӿ�
����: ����
�ļ�: synthesis.hpp
*/

#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

using LL = long long;

class synthesisInfo {
public:
    std::unordered_multiset<LL> requirements;       // ����װ��
    LL                          coins;              // ����Ӳ��
    LL                          targetId;           // �ϳ�Ŀ�� ID
};

extern std::unordered_multimap<LL, synthesisInfo>   allSyntheses;

// �������ṩ����������Կ��õĺϳ�. ����з��������ĺϳ�, �򷵻� true, coins Ϊ����Ӳ��; ���򷵻� false; coins ��ֵ�����޸�
bool bg_match_synthesis(const LL &targetId, const std::unordered_multiset<LL> &materials, LL &coins);
