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
#define UPGRADE_CMD(name, type)                                                                         \
    CMD(upgrade_##name##) {                                                                             \
        LL upgradeTimes = 0;                        /* 强化次数 */                                      \
        LL coinsNeeded = 0;                         /* 需要硬币 */                                      \
                                                                                                        \
        if (ev.message.length() < 16) {             /* 无参数 */                                        \
            if (preUpgradeCallback(ev, EqiType::##type##, "1", upgradeTimes, coinsNeeded))              \
                postUpgradeCallback(ev, EqiType::##type##, upgradeTimes, coinsNeeded);                  \
        }                                                                                               \
        else {                                      /* 有参数 */                                        \
            auto param = ev.message.substr(15);     /* 去掉命令字符串, 只保留参数 */                     \
            if (preUpgradeCallback(ev, EqiType::##type##, param, upgradeTimes, coinsNeeded))            \
                postUpgradeCallback(ev, EqiType::##type##, upgradeTimes, coinsNeeded);                  \
        }                                                                                               \
    }

UPGRADE_CMD(helmet, armor_helmet);
UPGRADE_CMD(body, armor_body);
UPGRADE_CMD(leg, armor_leg);
UPGRADE_CMD(boot, armor_boot);
UPGRADE_CMD(primary, weapon_primary);
UPGRADE_CMD(secondary, weapon_secondary);
UPGRADE_CMD(earrings, ornament_earrings);
UPGRADE_CMD(rings, ornament_rings);
UPGRADE_CMD(necklace, ornament_necklace);
UPGRADE_CMD(jewelry, ornament_jewelry);

// 确认强化
CMD(confirm_upgrade) {
    MATCH("确认", ConfirmUpgrade);
}
