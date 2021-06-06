/*
描述: Bingy 交易场相关接口
作者: 冰棍
文件: trade.hpp
*/

#pragma once

#include "inventory.hpp"
#include <string>

using LL = long long;

class tradeData {
    LL              tradeId;
    inventoryData   item;
    LL              sellerId;       // 卖方 QQ 号
    LL              addTime;        // 上架时间
    bool            hasPassword;    // 有密码
    std::string     password;       // 购买密码
    LL              price;          // 价格
};
