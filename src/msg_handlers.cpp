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
        if (pre##callbackName##Callback(ev))                        \
            post##callbackName##Callback(ev);                       \
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
    MATCH("注册", Register);
}

// 查看硬币
CMD(view_coins) {
    MATCH("查看硬币", ViewCoins);
}

// 签到
CMD(sign_in) {
    MATCH("签到", SignIn);
}

// 查看背包
CMD(view_inventory) {
    MATCH("查看背包", ViewInventory);
}

// 出售
CMD(pawn) {
    if (ev.message.length() < 9) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 出售指令格式为: \"bg 出售 背包序号1 背包序号2 ...\"");
        return;
    }
    auto params = str_split(ev.message.substr(9), ' ');         // 去掉命令字符串, 然后以空格分隔开参数
    if (params.size() > 0) {
        std::vector<LL> items;                                  // prePawnCallback 返回的处理之后的序号列表
        if (prePawnCallback(ev, params, items))
            postPawnCallback(ev, items);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 出售指令格式为: \"bg 出售 背包序号1 背包序号2 ...\"");
    }
}

// 查看属性
CMD(view_properties) {
    MATCH("查看属性", ViewProperties);
}

// 查看装备
CMD(view_equipments) {
    MATCH("查看装备", ViewEquipments);
}

// 装备
CMD(equip) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 装备指令格式为: \"bg 装备 背包序号\"");
        return;
    }
    auto param = ev.message.substr(9);                          // 去掉命令字符串, 只保留参数
    LL item = -1;
    if (preEquipCallback(ev, param, item))
        postEquipCallback(ev, item);
}

// 卸下头盔
CMD(unequip_helmet) {
    MATCH("卸下头盔", UnequipHelmet);
}

// 卸下战甲
CMD(unequip_body) {
    MATCH("卸下战甲", UnequipBody);
}

// 卸下护腿
CMD(unequip_leg) {
    MATCH("卸下护腿", UnequipLeg);
}

// 卸下战靴
CMD(unequip_boot) {
    MATCH("卸下战靴", UnequipBoot);
}

// 卸下护甲
CMD(unequip_armor) {
    MATCH("卸下护甲", UnequipArmor);
}

// 卸下主武器
CMD(unequip_primary) {
    MATCH("卸下主武器", UnequipPrimary);
}

// 卸下副武器
CMD(unequip_secondary) {
    MATCH("卸下副武器", UnequipSecondary);
}

// 卸下武器
CMD(unequip_weapon) {
    MATCH("卸下武器", UnequipWeapon);
}

// 卸下耳环
CMD(unequip_earrings) {
    MATCH("卸下耳环", UnequipEarrings);
}

// 卸下戒指
CMD(unequip_rings) {
    MATCH("卸下戒指", UnequipRings);
}

// 卸下项链
CMD(unequip_necklace) {
    MATCH("卸下项链", UnequipNecklace);
}

// 卸下宝石
CMD(unequip_jewelry) {
    MATCH("卸下宝石", UnequipJewelry);
}

// 卸下饰品
CMD(unequip_ornament) {
    MATCH("卸下饰品", UnequipOrnament);
}

// 卸下一次性物品
CMD(unequip_item) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 卸下一次性物品指令格式为: \"bg 卸下 背包序号\"\n"
            "或者卸下指定类型的物品: 例如: bg 卸下头盔 (只卸下头盔); bg 卸下饰品 (卸下所有饰品); bg 卸下所有 (卸下所有装备)");
        return;
    }
    auto param = ev.message.substr(9);                          // 去掉命令字符串, 只保留参数
    LL item = -1;
    if (preUnequipSingleCallback(ev, param, item))
        postUnequipSingleCallback(ev, item);
}

// 卸下一次性物品 (另一命令)
CMD(unequip_item_2) {
    if (ev.message.length() < 14) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 卸下一次性物品指令格式为: \"bg 卸下 背包序号\"\n"
            "或者卸下指定类型的物品: 例如: bg 卸下头盔 (只卸下头盔); bg 卸下饰品 (卸下所有饰品); bg 卸下所有 (卸下所有装备)");
        return;
    }
    auto param = ev.message.substr(13);                         // 去掉命令字符串, 只保留参数
    LL item = -1;
    if (preUnequipSingleCallback(ev, param, item))
        postUnequipSingleCallback(ev, item);
}

// 卸下所有
CMD(unequip_all) {
    if (ev.message == "bg 卸下所有" || ev.message == "bg 卸下全部" || ev.message == "bg 卸下所有装备" || ev.message == "bg 卸下全部装备") {
        if (preUnequipAllCallback(ev))
            postUnequipAllCallback(ev);
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
        LL upgradeTimes = 0;                                /* 强化次数 */                              \
        LL coinsNeeded = 0;                                 /* 需要硬币 */                              \
                                                                                                        \
        if (ev.message.length() < cmdLen) {                 /* 无参数 */                                \
            if (preUpgradeCallback(ev, EqiType::##type##, "1", upgradeTimes, coinsNeeded))              \
                postUpgradeCallback(ev, EqiType::##type##, upgradeTimes, coinsNeeded);                  \
        }                                                                                               \
        else {                                              /* 有参数 */                                \
            auto param = ev.message.substr(cmdLen - 1);     /* 去掉命令字符串, 只保留参数 */             \
            if (preUpgradeCallback(ev, EqiType::##type##, param, upgradeTimes, coinsNeeded))            \
                postUpgradeCallback(ev, EqiType::##type##, upgradeTimes, coinsNeeded);                  \
        }                                                                                               \
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
    MATCH("确认", ConfirmUpgrade);
}

// 强化装备帮助
CMD(upgrade_help) {
    cq::send_group_message(ev.target.group_id.value(), "请指定需要强化的装备类型, 命令后面可以跟需要强化的次数。例如: \"bg 强化主武器\", \"bg 强化战甲 5\"");
}

// 查看交易场
CMD(view_trade) {
    if (ev.message == "bg 交易场" || ev.message == "bg 查看交易场" || ev.message == "bg 交易") {
        if (preViewTradeCallback(ev))
            postViewTradeCallback(ev);
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
    LL item = -1;
    if (preBuyTradeCallback(ev, params, item))
        postBuyTradeCallback(ev, item);
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
    LL invId = -1;
    bool hasPassword = false;
    LL price = 0;
    if (preSellTradeCallback(ev, params, invId, hasPassword, price))
        postSellTradeCallback(ev, invId, hasPassword, price);
}

// 交易场下架
CMD(recall_trade) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 下架指令格式为: \"bg 下架 交易ID\"");
        return;
    }
    auto param = ev.message.substr(9);                         // 去掉命令字符串, 只保留参数
    LL tradeId = -1;
    if (preRecallTradeCallback(ev, param, tradeId))
        postRecallTradeCallback(ev, tradeId);
}

// 挑战怪物
CMD(fight) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 下架指令格式为: \"bg 挑战 副本号ID\"");
        return;
    }
    auto param = ev.message.substr(9);                         // 去掉命令字符串, 只保留参数
    LL dungeonLevel = -1;
    if (preFightCallback(ev, param, dungeonLevel))
        postFightCallback(ev, dungeonLevel);
}

// 合成装备
CMD(synthesis) {
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "合成装备指令格式为: \"bg 合成 目标装备ID(或名称) 背包序号1 背包序号2 ...\"。"
            "例如: bg 合成 终极魔剑 1 2 3。发送\"bg 合成 装备ID(或名称)\"可查看可用的合成。");
        return;
    }
    auto params = str_split(str_trim(ev.message.substr(9)), ' ');
    LL targetId = -1, coins = 0, level = 0;
    std::set<LL, std::greater<LL>> invList;
    if (preSynthesisCallback(ev, params, invList, targetId, coins, level))
        postSynthesisCallback(ev, invList, targetId, coins, level);
}

// 懒人宏
// 定义为指定玩家添加指定属性数值的管理指令
#define CMD_ADMIN_INC_FIELD(funcName, commandStr, fieldStr, callbackFuncName)                               \
    CMD(admin_add_##funcName##) {                                                                           \
        if (allAdmins.find(USER_ID) == allAdmins.end())                                                     \
            return;                                                                                         \
        if (ev.message.length() < sizeof(commandStr)) {                                                     \
            cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式: " commandStr " qq/all " fieldStr "数");  \
            return;                                                                                         \
        }                                                                                                   \
        auto param = ev.message.substr(sizeof(commandStr) - 1);         /* 去掉命令字符串, 只保留参数 */     \
        adminAdd##callbackFuncName##Callback(ev, param);                                                    \
    }

CMD_ADMIN_INC_FIELD(coins, "bg /addcoins", "硬币", Coins);
CMD_ADMIN_INC_FIELD(heroCoin, "bg /addherocoin", "英雄币", HeroCoin);
CMD_ADMIN_INC_FIELD(level, "bg /addlevel", "等级", Level);
CMD_ADMIN_INC_FIELD(blessing, "bg /addblessing", "祝福", Blessing);
CMD_ADMIN_INC_FIELD(energy, "bg /addenergy", "体力", Energy);
CMD_ADMIN_INC_FIELD(exp, "bg /addexp", "经验", Exp);
CMD_ADMIN_INC_FIELD(invCapacity, "bg /addinvcapacity", "背包容量", InvCapacity);
CMD_ADMIN_INC_FIELD(vip, "bg /addvip", "VIP等级", Vip);
