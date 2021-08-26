/*
描述: 处理 Bingy 的指令, 然后呼叫对应的函数
作者: 冰棍
文件: msg_handler.cpp
*/

#include "msg_handlers.hpp"
#include "game.hpp"
#include "utils.hpp"

// 懒人宏
// 命令必须要全字匹配才呼叫对应的 pre 和 post 回调函数
#define MATCH(str, callbackName)                                    \
    if (ev.message == "bg " str) {                                  \
        ##callbackName## Callback(ev);                              \
    }                                                               \
    else {                                                          \
        cq::send_group_message(GROUP_ID, bg_at(ev) +                \
            "命令格式不对哦! 要" str "的话发送\"bg " str "\"即可");  \
    }

// ping
CMD(bg) {
    if (ev.message == "bg")
        cq::send_group_message(ev.target.group_id.value(), "我在呀!");
}

// 注册
CMD(register) {
    MATCH("注册", register);
}

// 查看硬币
CMD(view_coins) {
    if (ev.message == "bg 查看硬币" || ev.message == "bg 硬币") {
        viewCoinsCallback(ev);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) +
            "命令格式不对哦! 要查看硬币的话发送\"bg 查看硬币\"即可");
    }
}

// 签到
CMD(sign_in) {
    MATCH("签到", signIn);
}

// 查看背包
CMD(view_inventory) {
    if (ev.message == "bg 查看背包" || ev.message == "bg 背包") {
        viewInventoryCallback(ev);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) +
            "命令格式不对哦! 要查看背包的话发送\"bg 查看背包\"即可");
    }
}

// 出售
CMD(pawn) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 出售指令格式为: \"bg 出售 背包序号1 背包序号2 ...\"");
        return;
    }
    auto params = str_split(ev.message.substr(9), ' ');         // 去掉命令字符串, 然后以空格分隔开参数
    if (params.size() > 0) {
        pawnCallback(ev, params);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 出售指令格式为: \"bg 出售 背包序号1 背包序号2 ...\"");
    }
}

// 查看属性
CMD(view_properties) {
    if (ev.message == "bg 查看属性" || ev.message == "bg 属性") {
        viewPropertiesCallback(ev);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) +
            "命令格式不对哦! 要查看属性的话发送\"bg 查看属性\"即可");
    }
}

// 查看装备
CMD(view_equipments) {
    if (ev.message == "bg 查看装备" || ev.message == "bg 我的装备") {
        viewEquipmentsCallback(ev);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) +
            "命令格式不对哦! 要查看装备的话发送\"bg 查看装备\"即可");
    }
}

// 查找装备
CMD(search_equipments) {
    if (ev.message.length() < 16) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 查找装备指令格式为: \"bg 查找装备 关键字\"");
        return;
    }
    auto param = ev.message.substr(16);
    if (param.size() > 0) {
        searchEquipmentsCallback(ev, param);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 查找装备指令格式为: \"bg 查找装备 关键字\"");
    }
}

// 装备
CMD(equip) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 装备指令格式为: \"bg 装备 背包序号\"。"
            "或者你可以发送\"bg 查看装备\"来查看身上的装备。");
        return;
    }
    auto param = ev.message.substr(9);
    equipCallback(ev, param);
}

// 卸下头盔
CMD(unequip_helmet) {
    MATCH("卸下头盔", unequipHelmet);
}

// 卸下战甲
CMD(unequip_body) {
    MATCH("卸下战甲", unequipBody);
}

// 卸下护腿
CMD(unequip_leg) {
    MATCH("卸下护腿", unequipLeg);
}

// 卸下战靴
CMD(unequip_boot) {
    MATCH("卸下战靴", unequipBoot);
}

// 卸下护甲
CMD(unequip_armor) {
    MATCH("卸下护甲", unequipArmor);
}

// 卸下主武器
CMD(unequip_primary) {
    MATCH("卸下主武器", unequipPrimary);
}

// 卸下副武器
CMD(unequip_secondary) {
    MATCH("卸下副武器", unequipSecondary);
}

// 卸下武器
CMD(unequip_weapon) {
    MATCH("卸下武器", unequipWeapon);
}

// 卸下耳环
CMD(unequip_earrings) {
    MATCH("卸下耳环", unequipEarrings);
}

// 卸下戒指
CMD(unequip_rings) {
    MATCH("卸下戒指", unequipRings);
}

// 卸下项链
CMD(unequip_necklace) {
    MATCH("卸下项链", unequipNecklace);
}

// 卸下宝石
CMD(unequip_jewelry) {
    MATCH("卸下宝石", unequipJewelry);
}

// 卸下饰品
CMD(unequip_ornament) {
    MATCH("卸下饰品", unequipOrnament);
}

// 卸下一次性物品
CMD(unequip_item) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 卸下一次性物品指令格式为: \"bg 卸下 背包序号\"\n"
            "或者卸下指定类型的物品: 例如: bg 卸下头盔 (只卸下头盔); bg 卸下饰品 (卸下所有饰品); bg 卸下所有 (卸下所有装备)");
        return;
    }
    auto param = ev.message.substr(9);
    unequipSingleCallback(ev, param);
}

// 卸下一次性物品 (另一命令)
CMD(unequip_item_2) {
    if (ev.message.length() < 14) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 卸下一次性物品指令格式为: \"bg 卸下 背包序号\"\n"
            "或者卸下指定类型的物品: 例如: bg 卸下头盔 (只卸下头盔); bg 卸下饰品 (卸下所有饰品); bg 卸下所有 (卸下所有装备)");
        return;
    }
    auto param = ev.message.substr(13);
    unequipSingleCallback(ev, param);
}

// 卸下所有
CMD(unequip_all) {
    if (ev.message == "bg 卸下所有" || ev.message == "bg 卸下全部" || ev.message == "bg 卸下所有装备" || ev.message == "bg 卸下全部装备") {
        unequipAllCallback(ev);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) +
            "命令格式不对哦! 要卸下所有装备的话可以发送: \"bg 卸下所有\"");
    }
}

// 懒人宏
// 定义强化装备相关的命令规则, 并调用强化回调函数
#define UPGRADE_CMD(name, type, cmdLen)                                                                 \
    CMD(upgrade_##name##) {                                                                             \
        if (ev.message.length() < cmdLen)                   /* 无参数 */                                \
            upgradeCallback(ev, EqiType::##type##, "1");                                                \
        else                                                /* 有参数 */                                \
            upgradeCallback(ev, EqiType::##type##, ev.message.substr(cmdLen - 1));                      \
    }

UPGRADE_CMD(helmet, armor_helmet, 16);
UPGRADE_CMD(body, armor_body, 16);
UPGRADE_CMD(leg, armor_leg, 16);
UPGRADE_CMD(boot, armor_boot, 16);
UPGRADE_CMD(primary, weapon_primary, 20);
UPGRADE_CMD(secondary, weapon_secondary, 20);
UPGRADE_CMD(earrings, ornament_earrings, 16);
UPGRADE_CMD(rings, ornament_rings, 16);
UPGRADE_CMD(necklace, ornament_necklace, 16);
UPGRADE_CMD(jewelry, ornament_jewelry, 16);

// 确认强化
CMD(confirm_upgrade) {
    MATCH("确认", confirmUpgrade);
}

// 强化装备帮助
CMD(upgrade_help) {
    cq::send_group_message(ev.target.group_id.value(), "请指定需要强化的装备类型, 命令后面可以跟需要强化的次数。例如: \"bg 强化主武器\", \"bg 强化战甲 5\"");
}

// 升级祝福
CMD(upgrade_blessing) {
    if (ev.message.length() < 16)               // 无参数
        upgradeBlessingCallback(ev, "1");
    else                                        // 有参数
        upgradeBlessingCallback(ev, ev.message.substr(15));
}

// 祝福帮助
CMD(blessing_help) {
    cq::send_group_message(ev.target.group_id.value(), "升级祝福等级能够提升你的战斗力，发送\"bg 升级祝福\"升级。命令后面可以跟需要升级的次数，例如：\"bg 升级祝福 5\"");
}

// 查看交易场
CMD(view_trade) {
    if (ev.message == "bg 交易场" || ev.message == "bg 查看交易场" || ev.message == "bg 交易") {
        viewTradeCallback(ev);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 要查看交易场的话可以发送: \"bg 交易场\"");
    }
}

// 购买交易场项目
CMD(buy_trade) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 购买指令格式为: \"bg 购买 交易ID\"。如果交易有密码, 则为: \"bg 购买 交易ID 密码\"");
        return;
    }
    auto params = str_split(str_trim(ev.message.substr(9)), ' ');
    if (params.size() > 2) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 购买指令格式为: \"bg 购买 交易ID\"。如果交易有密码, 则为: \"bg 购买 交易ID 密码\"");
        return;
    }
    buyTradeCallback(ev, params);
}

// 交易场上架
CMD(sell_trade) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 上架指令格式为: \"bg 上架 背包序号 价格\"。"
            "若要指定为有密码的交易, 则在命令最后加个空格和\"私\"字: \"bg 上架 背包序号 私\"");
        return;
    }

    auto params = str_split(str_trim(ev.message.substr(9)), ' ');
    if (params.size() != 2 && params.size() != 3) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 上架指令格式为: \"bg 上架 背包序号 价格\"。"
            "若要指定为有密码的交易, 则在命令最后加个空格和\"私\"字: \"bg 上架 背包序号 价格 私\"");
        return;
    }
    sellTradeCallback(ev, params);
}

// 交易场下架
CMD(recall_trade) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 下架指令格式为: \"bg 下架 交易ID\"");
        return;
    }
    auto param = ev.message.substr(9);
    recallTradeCallback(ev, param);
}

// 挑战怪物
CMD(fight) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 挑战指令格式为: \"bg 挑战 副本号\"");
        return;
    }
    auto param = ev.message.substr(9);
    fightCallback(ev, param);
}

// PVP
CMD(pvp) {
    if (ev.message.length() < 7) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! PVP指令格式为: \"bg pvp 目标\"");
        return;
    }
    auto param = ev.message.substr(6);
    pvpCallback(ev, param);
}

// 合成装备
CMD(synthesis) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "合成装备指令格式为: \"bg 合成 目标装备ID(或名称) 背包序号1 背包序号2 ...\"。"
            "例如: bg 合成 终极魔剑 1 2 3。发送\"bg 合成 装备ID(或名称)\"可查看可用的合成。");
        return;
    }
    auto params = str_split(str_trim(ev.message.substr(9)), ' ');
    synthesisCallback(ev, params);
}

// 懒人宏
// 定义为指定玩家添加指定属性数值的管理指令
#define CMD_ADMIN_MODIFY_FIELD(funcName, commandStr, fieldStr, callbackFuncName)                                    \
    CMD(admin_add_##funcName##) {                                                                                   \
        if (ev.message.length() < sizeof("bg /add" commandStr)) {                                                   \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式: bg /add" commandStr " qq/all " fieldStr "值");   \
            return;                                                                                                 \
        }                                                                                                           \
        auto param = ev.message.substr(sizeof("bg /add" commandStr) - 1);                                           \
        adminAdd##callbackFuncName##Callback(ev, param);                                                            \
    }                                                                                                               \
                                                                                                                    \
    CMD(admin_set_##funcName##) {                                                                                   \
        if (ev.message.length() < sizeof("bg /set" commandStr)) {                                                   \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式: bg /set" commandStr " qq/all " fieldStr "数");   \
            return;                                                                                                 \
        }                                                                                                           \
        auto param = ev.message.substr(sizeof("bg /set" commandStr) - 1);                                           \
        adminSet##callbackFuncName##Callback(ev, param);                                                            \
    }

CMD_ADMIN_MODIFY_FIELD(coins, "coins", "硬币", Coins);
CMD_ADMIN_MODIFY_FIELD(heroCoin, "herocoin", "英雄币", HeroCoin);
CMD_ADMIN_MODIFY_FIELD(level, "level", "等级", Level);
CMD_ADMIN_MODIFY_FIELD(blessing, "blessing", "祝福", Blessing);
CMD_ADMIN_MODIFY_FIELD(energy, "energy", "体力", Energy);
CMD_ADMIN_MODIFY_FIELD(exp, "exp", "经验", Exp);
CMD_ADMIN_MODIFY_FIELD(invCapacity, "invcapacity", "背包容量", InvCapacity);
CMD_ADMIN_MODIFY_FIELD(vip, "vip", "VIP等级", Vip);

// 和机器人聊骚
CMD(chat) {
    chatCallback(ev);
}
