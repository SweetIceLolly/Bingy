/*
描述: Bingy 交易场相关接口
作者: 冰棍
文件: trade.hpp
*/

#pragma once

#include "inventory.hpp"
#include <string>
#include <map>
#include <mutex>
#include <vector>

using LL = long long;

class tradeData {
public:
    LL              tradeId;
    inventoryData   item;
    LL              sellerId;       // 卖方 QQ 号
    LL              addTime;        // 上架时间
    bool            hasPassword;    // 有密码
    std::string     password;       // 购买密码
    LL              price;          // 价格
};

extern std::map<LL, tradeData>  allTradeItems;      // 注意: 读取的时候可以不用加锁, 但是不要使用[], 需要使用 at(). 多线程写入的时候必须加锁

// 获取所有交易场条目
std::map<LL, tradeData> bg_trade_get_items(const bool &use_cache = true);

// 获取当前的交易 ID
LL bg_get_tradeId(const bool &use_cache = true);

// 设置交易 ID
bool bg_set_tradeId(const LL &val);

// 使交易 ID 的值增加 1
bool bg_inc_tradeId();

// 往交易场添加项目
bool bg_trade_insert_item(const tradeData &itemData);

// 从交易场移除一个项目
bool bg_trade_remove_item(const LL &tradeId);

// 从交易场移除一系列的项目
bool bg_trade_remove_item(const std::vector<LL> &tradeIdList);

// 获取交易场内容字符串
std::string bg_trade_get_string(const bool &use_cache = true);
