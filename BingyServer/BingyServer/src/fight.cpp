/*
描述: 对战相关的操作
作者: 冰棍
文件: fight.hpp
*/

#include "fight.hpp"
#include "secrets.hpp"

#define FLOAT_PRECISION 1e-6            // 内部计算的数值精度, 绝对值小于这个的数值视为 0

// 懒人宏
// 定义检查玩家装备和技能相关的函数
#define CHECK_BUFF(type)    \
    inline void check_##type##_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &preMsg, std::string &postMsg, std::vector<std::string> &msg)

/**
 * 检查玩家装备和技能相关的函数
 * 优先级: init > mp > brk = def > atk = crt > dmg > hp
 * 参数说明:
 *  round: 回合计数
 *  a, aOriginal: 当前回合所在的玩家的属性, 及其原始属性
 *  b, bOriginal: 另一方的属性, 及其原始属性
 *  postMsg: 对战结束后的消息. 请使用拼接字符串的形式来修改. 如: postMsg += "xxx"
 *  msg: 当前回合的消息. 请使用 push_back 来修改. 如: msg.push_back("xxx")
 */
inline void check_init_buff(fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &preMsg, std::string &postMsg);
CHECK_BUFF(mp);
CHECK_BUFF(brk);
CHECK_BUFF(def);
CHECK_BUFF(atk);
CHECK_BUFF(crt);
inline void check_dmg_buff(LL round, double &dmg, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &preMsg, std::string &postMsg, std::vector<std::string> &msg);
CHECK_BUFF(hp);
inline void remove_single_items(fightable &a);

fightable::fightable() {
    throw std::runtime_error("必须通过玩家或者怪物创建 fightable!");
}

fightable::fightable(const fightable &f) {
    atk = f.atk;
    def = f.def;
    brk = f.brk;
    agi = f.agi;
    hp = f.hp;
    mp = f.mp;
    crt = f.crt;
    currHp = f.currHp;
    playerId = f.playerId;
    monsterId = f.monsterId;
    equipItems = f.equipItems;
    equipments = f.equipments;
}

fightable::fightable(player &p) {
    atk = p.get_atk();
    def = p.get_def();
    brk = p.get_brk();
    agi = p.get_agi();
    hp = p.get_hp();
    mp = p.get_mp();
    crt = p.get_crt();
    currHp = hp;
    playerId = p.get_id();
    monsterId = -1;

    // 获取玩家的装备和一次性装备
    for (const auto &item : p.get_equipItems())
        equipItems.insert(item.id);
    for (const auto &item : p.get_equipments()) {
        if (item.second.id != -1)
            equipments.insert(item.second.id);
    }
}

fightable::fightable(const monsterData &m) {
    atk = m.atk;
    def = m.def;
    brk = m.brk;
    agi = m.agi;
    hp = m.hp;
    mp = 0;
    crt = m.crt;
    currHp = hp;
    monsterId = m.id;
    playerId = -1;
}

// 把字符串数组展开成字符串
inline std::string flatten_str_vec(const std::vector<std::string> &strVec) {
    if (strVec.size() > 0) {
        std::string rtn;
        for (const auto &str : strVec) {
            rtn += str + ", ";
        }
        rtn.pop_back();         // 去掉结尾多余的 ", "
        rtn.pop_back();
        return rtn;
    }
    else
        return "";
}

std::vector<std::tuple<LL, LL, std::string>> bg_fight(const fightable &obj_a, const fightable &obj_b, bool &a_wins, bool &a_first, std::string &preMsg, std::string &postMsg) {
    std::vector<std::tuple<LL, LL, std::string>> rounds;        // 格式为: [[A打出的伤害, B的剩余血量, 附加信息], [B打出的伤害, A的剩余血量, 附加信息], ...]
    fightable a = obj_a, b = obj_b;                             // 复制一份, 以便之后访问玩家原本的属性
    
    // 开始时检查玩家的装备和技能
    check_init_buff(a, obj_a, b, obj_b, preMsg, postMsg);
    check_init_buff(b, obj_b, a, obj_a, preMsg, postMsg);

    a_first = a.agi >= b.agi;                                   // 检查是否为 a 先手
    bool a_round = a_first;                                     // 是否为 a 的回合
    LL round = 0;

    while (a.currHp > 0 && b.currHp > 0) {
        std::vector<std::string> msg;                           // 技能消息

        // 把攻防破魔恢复成原本的数值
        a.atk = obj_a.atk;  b.atk = obj_b.atk;
        a.def = obj_a.def;  b.def = obj_b.def;
        a.brk = obj_a.brk;  b.brk = obj_b.brk;
        a.mp = obj_a.mp;    b.mp = obj_b.mp;

        // 若当前回合等于先手的回合, 那么回合数 + 1
        if (a_round == a_first)
            ++round;
        
        if (a_round) {
            // 检查 a 的 mp 技能, a 的 brk 技能, b 的 def 技能
            check_mp_buff(round, a, obj_a, b, obj_b, preMsg, postMsg, msg);
            check_brk_buff(round, a, obj_a, b, obj_b, preMsg, postMsg, msg);
            check_def_buff(round, b, obj_b, a, obj_a, preMsg, postMsg, msg);

            // 计算最终防护
            double bFinalDef = std::max(b.def - a.brk / 4, 0.0);

            // 检查 a 的 atk 和 crt 技能
            check_atk_buff(round, a, obj_a, b, obj_b, preMsg, postMsg, msg);
            check_crt_buff(round, a, obj_a, b, obj_b, preMsg, postMsg, msg);

            // 根据暴击计算最终攻
            // 若暴击为 0, 则最终攻 = 原始攻
            // 若暴击不为 0, 则有 (暴击 mod 100) 的机会打出更高的攻击
            double aFinalAtk = a.atk;
            if (a.crt > FLOAT_PRECISION) {
                if (rndRange(100) <= static_cast<LL>(a.crt) % 100)
                    aFinalAtk *= (1.3 + 0.1 * (1 + floor(a.crt / 100.0)) + a.crt / (a.atk * 2.5));
                else
                    aFinalAtk *= (1.3 + 0.1 * floor(a.crt / 100.0) + a.crt / (a.atk * 2.5));
            }

            // 计算实际扣血
            double aDmg = aFinalAtk * (1.0 - bFinalDef / (bFinalDef + 100));

            // 检查 a 的 dmg 技能
            check_dmg_buff(round, aDmg, a, obj_a, b, obj_b, preMsg, postMsg, msg);

            b.currHp -= aDmg;

            // 检查 b 的 hp 技能
            check_hp_buff(round, b, obj_b, a, obj_a, preMsg, postMsg, msg);

            if (b.currHp <= FLOAT_PRECISION)
                b.currHp = 0;
            rounds.push_back(std::make_tuple(aDmg, b.currHp, flatten_str_vec(msg)));
            if (b.currHp <= FLOAT_PRECISION) {
                a_wins = true;
                break;
            }
        }
        else {
            // 检查 b 的 mp 技能,  b 的 brk 技能和 a 的 def 技能
            check_mp_buff(round, b, obj_b, a, obj_a, preMsg, postMsg, msg);
            check_brk_buff(round, b, obj_b, a, obj_a, preMsg, postMsg, msg);
            check_def_buff(round, a, obj_a, b, obj_b, preMsg, postMsg, msg);

            // 计算最终防护
            double aFinalDef = std::max(a.def - b.brk / 4, 0.0);

            // 检查 b 的 atk 和 crt 技能
            check_atk_buff(round, b, obj_b, a, obj_a, preMsg, postMsg, msg);
            check_crt_buff(round, b, obj_b, a, obj_a, preMsg, postMsg, msg);

            // 根据暴击计算最终攻
            double bFinalAtk = b.atk;
            if (b.crt > FLOAT_PRECISION) {
                if (rndRange(100) <= static_cast<LL>(b.crt) % 100)
                    bFinalAtk *= (1.3 + 0.1 * (1 + floor(b.crt / 100.0)) + b.crt / (b.atk * 2.5));
                else
                    bFinalAtk *= (1.3 + 0.1 * floor(b.crt / 100.0) + b.crt / (b.atk * 2.5));
            }

            // 计算实际扣血
            double bDmg = bFinalAtk * (1.0 - aFinalDef / (aFinalDef + 100));

            // 检查 b 的 dmg 技能
            check_dmg_buff(round, bDmg, b, obj_b, a, obj_a, preMsg, postMsg, msg);

            a.currHp -= bDmg;

            // 检查 a 的 hp 技能
            check_hp_buff(round, a, obj_a, b, obj_b, preMsg, postMsg, msg);

            if (a.currHp <= FLOAT_PRECISION)
                a.currHp = 0;
            rounds.push_back(std::make_tuple(bDmg, a.currHp, flatten_str_vec(msg)));
            if (a.currHp <= FLOAT_PRECISION) {
                a_wins = false;
                break;
            }
        }

        // 切换回合
        a_round = !a_round;
    }

    // 检查彩蛋
    postFightEasterEgg(b.monsterId, a.playerId, a_wins, preMsg, postMsg);

    // 去掉多余的换行符
    if (preMsg.length() > 0) {
        if (preMsg.back() == '\n')
            preMsg.pop_back();
    }

    // 移除使用过的一次性物品
    remove_single_items(a);
    remove_single_items(b);

    return rounds;
}

// 火攻
inline void fire_atk(LL round, double &dmgDelta, fightable &b, std::vector<std::string> &msg) {
    if (b.equipments.find(18) != b.equipments.end() || b.equipments.find(19) != b.equipments.end()) {
        // 如果对方有红色耳环或者火焰宝石, 则不受烧伤影响
        dmgDelta = 0;
        if (round == 1)
            msg.push_back("对方不受烧伤影响");
        return;
    }
    if (b.equipments.find(22) != b.equipments.end()) {
        // 如果对方有寒冰宝石, 则多受 10 点伤害
        dmgDelta += 10;
        if (round == 1)
            msg.push_back("对方烧伤且拥有寒冰宝石,每回合-(" + std::to_string(static_cast<LL>(dmgDelta - 10)) + "+10)血");
    }
    else {
        // 没有寒冰宝石
        if (round == 1)
            msg.push_back("对方烧伤,每回合-" + std::to_string(static_cast<LL>(dmgDelta)) + "血");
    }
}

// 冰攻
inline void ice_atk(LL round, double &dmgDelta, fightable &b, std::vector<std::string> &msg) {
    if (b.equipments.find(21) != b.equipments.end() || b.equipments.find(22) != b.equipments.end()) {
        // 如果对方有蓝色耳环或者寒冰宝石, 则不受冻伤影响
        dmgDelta = 0;
        if (round == 1)
            msg.push_back("对方不受冻伤影响");
        return;
    }
    if (b.equipments.find(19) != b.equipments.end()) {
        // 如果对方有火焰宝石, 则多受 10 点伤害
        dmgDelta += 10;
        if (round == 1)
            msg.push_back("对方冻伤且拥有火焰宝石,每回合-(" + std::to_string(static_cast<LL>(dmgDelta - 10)) + "+10)血");
    }
    else {
        // 没有火焰宝石
        if (round == 1)
            msg.push_back("对方冻伤,每回合-" + std::to_string(static_cast<LL>(dmgDelta)) + "血");
    }
}

// 开始的时候检查 a 的技能
inline void check_init_buff(fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &preMsg, std::string &postMsg) {
    // 怪物相关
    switch (a.monsterId) {
    case -1:
        // 不是怪物, 继续去检查装备
        break;
    }

    // 一次性装备相关
    if (a.equipItems.size() > 0) {
        if (a.equipItems.find(16) != a.equipItems.end()) {
            // 哥布林护符: 有 30% 的概率令哥布林不战自退
            if (b.monsterId != -1 && (b.monsterId == 6 || b.monsterId == 8 || b.monsterId == 9 || b.monsterId == 11)) {
                // 移除这个物品
                a.equipItems.erase(16);
                bg_player_get(a.playerId).remove_equipItem_by_id(16);

                if (rndRange(99) < 50) {
                    // 成功
                    preMsg += "护符成功令哥布林以为你是他们中的一员! 敌人不战自退了。";
                    b.currHp = 0;
                    return;
                }
                else {
                    // 不成功
                    preMsg += "哥布林认出了这个哥布林护符不是你的并向你发起了攻击!\n";
                }
            }
        }
        if (a.equipItems.find(11) != a.equipItems.end()) {
            // 移除这个物品
            a.equipItems.erase(11);
            bg_player_get(a.playerId).remove_equipItem_by_id(11);

            // 轻盈之羽: + 20 敏
            a.agi += 20;
            preMsg += std::to_string(a.playerId) + ": 轻盈之羽, +20敏\n";
        }
    }

    // 装备相关
    if (a.equipments.size() > 0) {
        
    }
}

// 检查 a 的魔法值相关技能
CHECK_BUFF(mp) {

}

// 检查 a 的破甲值相关技能
CHECK_BUFF(brk) {

}

// 检查 a 防护值相关技能
CHECK_BUFF(def) {
    // 怪物相关
    switch (a.monsterId) {
    case -1:
        // 不是怪物, 继续去检查装备
        break;

    case 14:
        // 末狮: 狂狮怒吼, 玩家失去 10 点防护
        b.def -= 10;
        if (round == 1) {
            preMsg += "末狮: 狂狮怒吼, 玩家失去10点防护\n";
        }
        return;
    }
}

// 检查 a 的攻击值相关技能
CHECK_BUFF(atk) {
    // 怪物相关
    switch (a.monsterId) {
    case -1:
        // 不是怪物, 继续去检查装备
        break;
    }

    // 一次性装备相关
    if (a.equipItems.size() > 0) {
        if (a.equipItems.find(8) != a.equipItems.end()) {
            // 巨熊之胆: 加强 10 点攻击
            a.atk += 10;
            if (round == 1)
                msg.push_back("巨熊之胆:每回合+10攻击");
        }
    }

    // 装备相关
    if (a.equipments.size() > 0) {
        
    }
    if (b.equipments.size() > 0) {
        if (b.equipments.find(36) != b.equipments.end()) {
            // 如果对方有狮毛护腿: 前三回合攻击减弱 5% - 25%
            if (round <= 3) {
                double atkPercent = rndRangeFloat(0.75, 0.95);
                msg.push_back("攻击减弱" + std::to_string(static_cast<LL>(100 - atkPercent * 100)) + "％");
                a.atk *= atkPercent;
            }
        }
    }
}

// 检查 a 的暴击值相关技能
CHECK_BUFF(crt) {

}

// 检查 a 的最终攻击相关技能
inline void check_dmg_buff(LL round, double &dmg, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &preMsg, std::string &postMsg, std::vector<std::string> &msg) {
    // 怪物相关
    switch (a.monsterId) {
    case -1:
        // 不是怪物, 继续去检查装备
        break;

    case 2:
        // 喰毒巨蟒: 中毒, 每回合 + 7 伤害
        dmg += 7;
        if (round == 1)
            msg.push_back("玩家中毒,每回合-7血");
        return;

    case 8: {
        // 火焰哥布林: 烧伤, 每回合 + 5 伤害
        double dmgDelta = 5;
        fire_atk(round, dmgDelta, b, msg);
        dmg += dmgDelta;
        return;
    }

    case 9: {
        // 寒冰哥布林: 冻伤, 每回合 + 5 伤害
        double dmgDelta = 5;
        ice_atk(round, dmgDelta, b, msg);
        dmg += dmgDelta;
        return;
    }
    }

    // 一次性装备相关
    if (a.equipItems.size() > 0) {
        if (a.equipItems.find(5) != a.equipItems.end()) {
            // 蟒毒: 给对方造成中毒, 每回合 + 7 伤害
            dmg += 5;
            if (round == 1)
                msg.push_back("蟒毒:对方中毒,每回合-7血");
        }
    }

    // 装备相关
    if (a.equipments.size() > 0) {
        if (a.equipments.find(17) != a.equipments.end()) {
            // 火弓: 给对方造成烧伤, 每回合 + 10 伤害
            double dmgDelta = 10;
            fire_atk(round, dmgDelta, b, msg);
            dmg += dmgDelta;
        }
        if (a.equipments.find(20) != a.equipments.end()) {
            // 冰弩: 给对方造成冻伤, 每回合 + 10 伤害
            double dmgDelta = 10;
            ice_atk(round, dmgDelta, b, msg);
            dmg += dmgDelta;
        }
        if (a.equipments.find(19) != a.equipments.end()) {
            // 火焰宝石: 给对方造成烧伤, 每回合 + 10 伤害
            double dmgDelta = 10;
            fire_atk(round, dmgDelta, b, msg);
            dmg += dmgDelta;
        }
        if (a.equipments.find(22) != a.equipments.end()) {
            // 寒冰宝石: 给对方造成冻伤, 每回合 + 10 伤害
            double dmgDelta = 10;
            ice_atk(round, dmgDelta, b, msg);
            dmg += dmgDelta;
        }
        if (a.equipments.find(27) != a.equipments.end()) {
            // 嗜血之刃: 每回合吸取打出攻击的 5% - 10%血量
            double hpBonus = rndRangeFloat(dmg * 0.05, dmg * 0.1);
            a.currHp += hpBonus;
            msg.push_back("吸血" + std::to_string(static_cast<LL>(hpBonus)) + "点");
        }
    }
}

// 检查 a 的血值相关技能
CHECK_BUFF(hp) {

}

// 移除目标玩家的一次性用品
inline void remove_single_items(fightable &a) {
    if (a.equipItems.size() > 0) {
        // 蟒毒
        if (a.equipItems.find(5) != a.equipItems.end())
            bg_player_get(a.playerId).remove_equipItem_by_id(5);

        // 巨熊之胆
        if (a.equipItems.find(8) != a.equipItems.end())
            bg_player_get(a.playerId).remove_equipItem_by_id(8);
    }
}
