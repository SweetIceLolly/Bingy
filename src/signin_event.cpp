/*
描述: 签到活动列表相关的操作
作者: 冰棍
文件: signin_event.cpp
*/

#include "signin_event.hpp"
#include <chrono>
#include <ctime>

std::vector<signInEvent>    allSignInEvents;

// 获取当前系统时间, 自动匹配对应的活动, 然后修改 coin 和 energy 对应的数值
void matchSignInEvent(LL &coin, LL &energy) {
    time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto time = std::localtime(&t);
    // todo
}
