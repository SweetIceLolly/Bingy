/*
����: Bingy ���׳���ؽӿ�
����: ����
�ļ�: trade.hpp
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
    LL              sellerId;       // ���� QQ ��
    LL              addTime;        // �ϼ�ʱ��
    bool            hasPassword;    // ������
    std::string     password;       // ��������
    LL              price;          // �۸�
};

extern std::map<LL, tradeData>  allTradeItems;      // ע��: ��ȡ��ʱ����Բ��ü���, ���ǲ�Ҫʹ��[], ��Ҫʹ�� at(). ���߳�д���ʱ��������

// ��ȡ���н��׳���Ŀ
std::map<LL, tradeData> bg_trade_get_items(const bool &use_cache = true);

// ��ȡ��ǰ�Ľ��� ID
LL bg_get_tradeId(const bool &use_cache = true);

// ���ý��� ID
bool bg_set_tradeId(const LL &val);

// ʹ���� ID ��ֵ���� 1
bool bg_inc_tradeId();

// �����׳������Ŀ
bool bg_trade_insert_item(const tradeData &itemData);

// �ӽ��׳��Ƴ�һ����Ŀ
bool bg_trade_remove_item(const LL &tradeId);

// �ӽ��׳��Ƴ�һϵ�е���Ŀ
bool bg_trade_remove_item(const std::vector<LL> &tradeIdList);

// ��ȡ���׳������ַ���
std::string bg_trade_get_string(const bool &use_cache = true);
