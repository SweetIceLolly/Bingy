/*
描述: 签到活动类, 以及相关接口
作者: 冰棍
文件: signin_event.hpp
*/

#pragma once

#include <vector>
#include <string>

using LL = long long;

class signInEvent {
public:
    int             year, month, day;       // 年月日 (若年份为 -1, 则每年都有; 若月份为 -1, 则每月都有; 天数不得为 -1)
    char            hour, minute;           // 时分 (若时为 -1, 则在规定的分钟签到就有; 若分为 -1, 则在规定的小时签到就有)
    LL              firstN;                 // 前 N 个玩家才能获得奖励 (若为 -1, 则所有在规定时间内签到的玩家都能获取奖励)
    double          coinFactor;             // 硬币系数
    double          energyFactor;           // 体力系数
    std::vector<LL> items;                  // 赠送物品 (若只有一个 ID 为 -1 的项目, 则说明没有赠送物品)
    std::string     message;                // 活动消息
};

extern std::vector<signInEvent> allSignInEvents;

void matchSignInEvent(LL &coin, LL &energy);
