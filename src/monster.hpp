/*
描述: Bingy 怪物类
作者: 冰棍
文件: monster.hpp
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

using LL = long long;

extern std::string monsterConfigPath;

class monsterDrop {
public:
    LL          id;
    double      chance;
};

class monsterData {
public:
    LL                          id;
    std::string                 name;
    LL                          atk, def, agi, hp;      // 攻防敏血
    LL                          coin;                   // 掉落硬币
    LL                          exp;                    // 奖励经验
    std::string                 message;                // 出场消息
    std::vector<monsterDrop>    drop;                   // 掉落
};

extern std::unordered_map<LL, monsterData> allMonsters; // 注意: 读取的时候可以不用加锁, 但是不要使用[], 需要使用 at(). 多线程写入的时候必须加锁

bool bg_load_monster_config();
