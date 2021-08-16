/*
描述: 对战接口
作者: 冰棍
文件: fight.hpp
*/

#pragma once

#include "player.hpp"
#include "monster.hpp"

// 能够进行对战的对象
class fightable {
public:
    double atk, def, brk, agi, hp, mp, crt;                     // 攻防破敏血魔暴
    double currHp;                                              // 当前血量
    
    // 只有玩家才会有以下内容
    LL playerId;                                                // 玩家 ID

    // 只有怪物才会有以下内容
    LL monsterId;                                               // 怪物 ID

    // 默认构造函数
    fightable();

    // 复制构造函数
    fightable(const fightable &f);

    // 从玩家信息获取对战对象
    // 这里 player 不加 const 是因为计算玩家属性的过程中可能会更新玩家的属性缓存以加速计算, 因此不是 const 操作
    fightable(player &p);

    // 从怪物信息获取对战对象
    fightable(const monsterData &m);
};

// 令两个对象对战, 返回对战的回合信息
// 格式为: [[A打出的伤害, B的剩余血量, 附加信息], [B打出的伤害, A的剩余血量, 附加信息], ...]
std::vector<std::tuple<LL, LL, std::string>> bg_fight(fightable a, fightable b, bool &a_wins, bool &a_first);
