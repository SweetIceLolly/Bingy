/*
描述: Bingy 游戏相关的函数. 整个游戏的主要逻辑
作者: 冰棍
文件: game.cpp
*/

#include "game.hpp"
#include "player.hpp"

std::unordered_set<LL> allAdmins;
std::unordered_set<LL>   blacklist;

bool accountCheck(const bgGameHttpReq& req) {
    return false;
}
