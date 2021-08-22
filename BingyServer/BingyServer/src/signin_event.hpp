/*
描述: 签到活动类, 以及相关接口
作者: 冰棍
文件: signin_event.hpp
*/

#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>
#include "utils.hpp"

using LL = std::int64_t;

class signInEvent {
private:
    // 只有指定了前 N 个签到的玩家数量这三个变量才会使用到
    LL              signInCount;            // 签到的人数
    bool            signInCount_cache;
    LL              prevActiveTime;         // 上一次活动时间
    bool            prevActiveTime_cache;
    std::mutex      mutexSignInCount;

public:
    LL              id;
    int             year, month, day;       // 年月日 (若年份为 -1, 则每年都有; 若月份为 -1, 若天数为 -1, 则每天都有)
    char            hour, minute;           // 时分 (若时为 -1, 则在规定的分钟签到就有; 若分为 -1, 则在规定的小时签到就有)
    double          coinFactor;             // 硬币系数
    double          energyFactor;           // 体力系数
    LL              firstN;                 // 前 N 个签到的玩家才能获取奖励. 统计数据只会精细到天
    std::vector<LL> items;                  // 赠送物品 (若只有一个 ID 为 -1 的项目, 则说明没有赠送物品)
    std::string     message;                // 活动消息

    signInEvent();
    signInEvent(const signInEvent &ev);
    LL get_signInCount(bool use_cache = true);
    bool inc_signInCount(LL val);
    bool set_signInCount(LL val);
    LL get_prevActiveTime(bool use_cache = true);
    bool set_prevActiveTime(LL val);
};

extern std::vector<signInEvent> allSignInEvents;

void bg_match_sign_in_event(const dateTime &time, LL &coin, LL &energy, std::vector<LL> &items, std::string &msg);
