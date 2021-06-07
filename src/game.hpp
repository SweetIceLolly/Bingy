/*
描述: Bingy 游戏相关函数的接口
作者: 冰棍
文件: game.hpp
*/

#pragma once

#include <cqcppsdk/cqcppsdk.hpp>
#include <unordered_set>

using LL = long long;

extern std::unordered_set<LL>  blacklist;
extern std::unordered_set<LL>  allAdmins;

std::string bg_at(const cq::MessageEvent &ev);
bool accountCheck(const cq::MessageEvent &ev);

// 懒人宏
// 定义 pre*Callback 和 post*Callback
#define PRE_POST(name)                                      \
    bool pre##name##Callback(const cq::MessageEvent &ev);   \
    void post##name##Callback(const cq::MessageEvent &ev);  \

// 懒人宏
#define GROUP_ID    ev.target.group_id.value()      // 从 ev 获取群号
#define USER_ID     ev.target.user_id.value()       // 从 ev 获取玩家 QQ 号

PRE_POST(Register);
PRE_POST(ViewCoins);
PRE_POST(SignIn);
