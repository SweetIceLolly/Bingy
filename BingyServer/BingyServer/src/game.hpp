/*
描述: Bingy 游戏相关函数的接口
作者: 冰棍
文件: game.hpp
*/

#pragma once

#include <unordered_set>
#include "inventory.hpp"
#include "http_handlers.hpp"

// Bingy 游戏部分的 HTTP 请求内容
typedef struct _bgGameHttpReq {
    http_req    *req;           // 请求
    LL          playerId;       // 玩家 QQ 号
    LL          groupId;        // 群号
} bgGameHttpReq;

extern std::unordered_set<LL>   allAdmins;
extern std::unordered_set<LL>   blacklist;

bool accountCheck(const bgGameHttpReq &req);

// 懒人宏
// 定义 pre*Callback 和 post*Callback
#define PRE_POST(name)                                          \
    bool pre ##name## Callback(const bgGameHttpReq &bgReq);     \
    void post ##name## Callback(const bgGameHttpReq &bgReq);    \

PRE_POST(Register);
PRE_POST(ViewCoins);
PRE_POST(SignIn);
PRE_POST(ViewInventory);
bool prePawnCallback(const bgGameHttpReq &bgReq, const std::vector<LL> &items);
void postPawnCallback(const bgGameHttpReq &bgReq, const std::vector<LL> &items);
PRE_POST(ViewProperties);
PRE_POST(ViewEquipments);
