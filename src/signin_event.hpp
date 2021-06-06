/*
描述: 签到活动类
作者: 冰棍
文件: signin_event.hpp
*/

#pragma once

#include <vector>
#include <string>

using LL = long long;

enum class signInEventType {
    date_precision,
    minute_precision,
    first_n
};

class signInEvent {
public:
    signInEventType type;                   // 签到活动类型
    int             year, month, day;       // 年月日
    char            hour, minute, second;   // 时分秒
    LL              firstN;                 // 前 N 个玩家才能获取奖励
    double          coinFactor;             // 硬币系数
    double          energyFactor;           // 体力系数
    std::vector<LL> items;                  // 赠送物品
    std::string     message;                // 活动消息
};
