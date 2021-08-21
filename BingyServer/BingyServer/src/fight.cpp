/*
描述: 对战相关的操作
作者: 冰棍
文件: fight.hpp
*/

#include "fight.hpp"

#define FLOAT_PRECISION 1e-6            // 内部计算的数值精度, 绝对值小于这个的数值视为 0

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
inline void check_init_buff(fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg);
inline void check_def_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg);
inline void check_brk_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg);
inline void check_atk_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg);
inline void check_crt_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg);
inline void check_dmg_buff(LL round, double &dmg, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg);
inline void check_hp_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg);
inline void check_mp_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg);

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

std::vector<std::tuple<LL, LL, std::string>> bg_fight(const fightable &obj_a, const fightable &obj_b, bool &a_wins, bool &a_first, std::string &postMsg) {
    std::vector<std::tuple<LL, LL, std::string>> rounds;        // 格式为: [[A打出的伤害, B的剩余血量, 附加信息], [B打出的伤害, A的剩余血量, 附加信息], ...]
    fightable a = obj_a, b = obj_b;                             // 复制一份, 以便之后访问玩家原本的属性
    
    // 开始时检查玩家的装备和技能
    check_init_buff(a, obj_a, b, obj_b, postMsg);
    check_init_buff(b, obj_b, a, obj_a, postMsg);

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
            check_mp_buff(round, a, obj_a, b, obj_b, postMsg, msg);
            check_brk_buff(round, a, obj_a, b, obj_b, postMsg, msg);
            check_def_buff(round, b, obj_b, a, obj_a, postMsg, msg);

            // 计算最终防护
            double bFinalDef = std::max(b.def - a.brk / 4, 0.0);

            // 检查 a 的 atk 和 crt 技能
            check_atk_buff(round, a, obj_a, b, obj_b, postMsg, msg);
            check_crt_buff(round, a, obj_a, b, obj_b, postMsg, msg);

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
            check_dmg_buff(round, aDmg, a, obj_a, b, obj_b, postMsg, msg);

            b.currHp -= aDmg;

            // 检查 b 的 hp 技能
            check_hp_buff(round, b, obj_b, a, obj_a, postMsg, msg);

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
            check_mp_buff(round, b, obj_b, a, obj_a, postMsg, msg);
            check_brk_buff(round, b, obj_b, a, obj_a, postMsg, msg);
            check_def_buff(round, a, obj_a, b, obj_b, postMsg, msg);

            // 计算最终防护
            double aFinalDef = std::max(a.def - b.brk / 4, 0.0);

            // 检查 b 的 atk 和 crt 技能
            check_atk_buff(round, b, obj_b, a, obj_a, postMsg, msg);
            check_crt_buff(round, b, obj_b, a, obj_a, postMsg, msg);

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
            check_dmg_buff(round, bDmg, b, obj_b, a, obj_a, postMsg, msg);

            a.currHp -= bDmg;

            // 检查 a 的 hp 技能
            check_hp_buff(round, a, obj_a, b, obj_b, postMsg, msg);

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

    return rounds;
}

// 开始的时候检查 a 的一次性物品和技能
inline void check_init_buff(fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg) {

}

// 检查 a 防护值相关技能
inline void check_def_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg) {
    // 怪物相关
    switch (a.monsterId) {
    case 14:
        // 末狮: 狂狮怒吼, 玩家失去 10 点防护
        b.def -= 10;
        if (round == 1)
            msg.push_back("狂狮怒吼,玩家失去10点防护");
        return;
    }
}

// 检查 a 的破甲值相关技能
inline void check_brk_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg) {

}

// 检查 a 的攻击值相关技能
inline void check_atk_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg) {
    
}

// 检查 a 的暴击值相关技能
inline void check_crt_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg) {

}

// 检查 a 的最终攻击相关技能
inline void check_dmg_buff(LL round, double &dmg, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg) {
    // 怪物相关
    switch (a.monsterId) {
    case 2:
        // 喰毒巨蟒: 中毒, 每回合 + 7 伤害
        dmg += 7;
        if (round == 1)
            msg.push_back("中毒,每回合+7伤害");
        return;

    case 8:
        // 火焰哥布林: 烧伤, 每回合 + 5 伤害
        dmg += 5;
        if (round == 1)
            msg.push_back("烧伤,每回合+5伤害");
        return;

    case 9:
        // 寒冰哥布林: 冻伤, 每回合 + 5 伤害
        dmg += 5;
        if (round == 1)
            msg.push_back("冻伤,每回合+5伤害");
        return;
    }
}

// 检查 a 的血值相关技能
inline void check_hp_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg) {

}

// 检查 a 的魔法值相关技能
inline void check_mp_buff(LL round, fightable &a, const fightable &aOriginal, fightable &b, const fightable &bOriginal, std::string &postMsg, std::vector<std::string> &msg) {

}
