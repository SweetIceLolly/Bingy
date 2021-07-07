/*
����: ǩ�����, �Լ���ؽӿ�
����: ����
�ļ�: signin_event.hpp
*/

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include "utils.hpp"

using LL = long long;

class signInEvent {
private:
    // ֻ��ָ����ǰ N ��ǩ����������������������Ż�ʹ�õ�
    LL              signInCount;            // ǩ��������
    bool            signInCount_cache;
    LL              prevActiveTime;         // ��һ�λʱ��
    bool            prevActiveTime_cache;
    std::mutex      mutexSignInCount;

public:
    LL              id;
    int             year, month, day;       // ������ (�����Ϊ -1, ��ÿ�궼��; ���·�Ϊ -1, ������Ϊ -1, ��ÿ�춼��)
    char            hour, minute;           // ʱ�� (��ʱΪ -1, ���ڹ涨�ķ���ǩ������; ����Ϊ -1, ���ڹ涨��Сʱǩ������)
    double          coinFactor;             // Ӳ��ϵ��
    double          energyFactor;           // ����ϵ��
    LL              firstN;                 // ǰ N ��ǩ������Ҳ��ܻ�ȡ����. ͳ������ֻ�ᾫϸ����
    std::vector<LL> items;                  // ������Ʒ (��ֻ��һ�� ID Ϊ -1 ����Ŀ, ��˵��û��������Ʒ)
    std::string     message;                // ���Ϣ

    signInEvent();
    signInEvent(const signInEvent &ev);
    LL get_signInCount(bool use_cache = true);
    bool inc_signInCount(const LL &val);
    bool set_signInCount(const LL &val);
    LL get_prevActiveTime(bool use_cache = true);
    bool set_prevActiveTime(const LL &val);
};

extern std::vector<signInEvent> allSignInEvents;

void bg_match_sign_in_event(const dateTime &time, LL &coin, LL &energy, std::vector<LL> &items, std::string &msg);
