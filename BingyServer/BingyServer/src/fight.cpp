/*
描述: 对战相关的操作
作者: 冰棍
文件: fight.hpp
*/

#include "fight.hpp"

#define FLOAT_PRECISION 1e-6            // 内部计算的数值精度, 小于这个的数值视为 0

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
    equipItems = p.get_equipItems();
    equipments = p.get_equipments();
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
}

std::vector<std::tuple<LL, LL, std::string>> bg_fight(fightable a, fightable b, bool &a_wins, bool &a_first) {
    std::vector<std::tuple<LL, LL, std::string>> rounds;        // 格式为: [[A打出的伤害, B的剩余血量, 附加信息], [B打出的伤害, A的剩余血量, 附加信息], ...]
    a_first = a.agi >= b.agi;                                   // 检查是否为 a 先手
    bool a_round = a_first;                                     // 是否为 a 的回合

    while (a.currHp > 0 && b.currHp > 0) {
        // 通过对方破甲值计算最终防护
        double aFinalDef = std::max(a.def - b.brk / 4, 0.0);
        double bFinalDef = std::max(b.def - a.brk / 4, 0.0);
        
        double aFinalAtk, bFinalAtk, aDmg, bDmg;
        if (a_round) {
            // 根据暴击计算最终攻
            // 若暴击为 0, 则最终攻 = 原始攻
            // 若暴击不为 0, 则有 (暴击 mod 100) 的机会打出更高的攻击
            aFinalAtk = a.atk;
            // todo 检查a.crt有没有可能<0
            if (a.crt > FLOAT_PRECISION) {
                if (rndRange(100) <= static_cast<LL>(a.crt) % 100)
                    aFinalAtk *= (1.3 + 0.1 * (1 + floor(a.crt / 100.0)) + a.crt / (a.atk * 2.5));
                else
                    aFinalAtk *= (1.3 + 0.1 * floor(a.crt / 100.0) + a.crt / (a.atk * 2.5));
            }

            // 计算实际扣血
            aDmg = aFinalAtk * (1.0 - bFinalDef / (bFinalDef + 100));
            b.currHp -= aDmg;
            if (b.currHp <= FLOAT_PRECISION)
                b.currHp = 0;
            rounds.push_back(std::make_tuple(aDmg, b.currHp, ""));
            if (b.currHp <= FLOAT_PRECISION) {
                a_wins = true;
                break;
            }
        }
        else {
            // 根据暴击计算最终攻
            bFinalAtk = b.atk;
            if (b.crt > FLOAT_PRECISION) {
                if (rndRange(100) <= static_cast<LL>(b.crt) % 100)
                    bFinalAtk *= (1.3 + 0.1 * (1 + floor(b.crt / 100.0)) + b.crt / (b.atk * 2.5));
                else
                    bFinalAtk *= (1.3 + 0.1 * floor(b.crt / 100.0) + b.crt / (b.atk * 2.5));
            }

            // 计算实际扣血
            bDmg = bFinalAtk * (1.0 - aFinalDef / (aFinalDef + 100));
            a.currHp -= bDmg;
            if (a.currHp <= FLOAT_PRECISION)
                a.currHp = 0;
            rounds.push_back(std::make_tuple(bDmg, a.currHp, ""));
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
