/*
描述: Bingy 怪物类
作者: 冰棍
文件: monster.hpp
*/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "utils.hpp"

using LL = std::int64_t;

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
    LL                          atk, def, brk, agi, hp; // 攻防破敏血
    LL                          coin;                   // 掉落硬币
    LL                          exp;                    // 奖励经验
    LL                          dungeonWeight;          // 在副本中的出现概率权重
    LL                          forestWeight;           // 在森林中的出现概率权重
    std::string                 message;                // 出场消息
    std::vector<monsterDrop>    drop;                   // 掉落
    luckyDraw                   dropDraw;               // 掉落抽取器

    void initDropDraw();                                // 根据当前的掉落列表重新生成掉落抽取器
};

class dungeonData {
public:
    LL                              level;              // 副本号
    std::vector<LL>                 monsters;           // 副本中的怪物 ID 列表
    luckyDraw                       monstersDraw;       // 副本怪物抽取器
};

extern std::unordered_map<LL, monsterData> allMonsters; // 注意: 读取的时候可以不用加锁, 但是不要使用[], 需要使用 at(). 多线程写入的时候必须加锁
extern std::unordered_map<LL, dungeonData> allDungeons; // 注意: 读取的时候可以不用加锁, 但是不要使用[], 需要使用 at(). 多线程写入的时候必须加锁
extern luckyDraw                           forestDraw;  // 森林抽奖机

bool bg_load_monster_config();                          // 读取怪物配置
void bg_init_monster_chances();                         // 初始化怪物出现和掉落概率
