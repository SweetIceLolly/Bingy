/*
描述: 提供 Bingy 指令的处理函数的接口
作者: 冰棍
文件: msg_handlers.hpp
*/

#pragma once

#include <cqcppsdk/cqcppsdk.hpp>

// 懒人宏
// 定义命令处理函数
#define CMD(cmd) void bg_cmd_##cmd##(const cq::MessageEvent &ev)

CMD(bg);
CMD(register);
CMD(sign_in);
CMD(pawn);
CMD(synthesis);
CMD(fight);

CMD(view_coins);
CMD(view_inventory);
CMD(view_properties);
CMD(view_equipments);

CMD(equip);
CMD(unequip_helmet);
CMD(unequip_body);
CMD(unequip_leg);
CMD(unequip_boot);
CMD(unequip_armor);
CMD(unequip_primary);
CMD(unequip_secondary);
CMD(unequip_weapon);
CMD(unequip_earrings);
CMD(unequip_rings);
CMD(unequip_necklace);
CMD(unequip_jewelry);
CMD(unequip_ornament);
CMD(unequip_item);
CMD(unequip_item_2);
CMD(unequip_all);

CMD(upgrade_helmet);
CMD(upgrade_body);
CMD(upgrade_leg);
CMD(upgrade_boot);
CMD(upgrade_primary);
CMD(upgrade_secondary);
CMD(upgrade_earrings);
CMD(upgrade_rings);
CMD(upgrade_necklace);
CMD(upgrade_jewelry);
CMD(confirm_upgrade);
CMD(upgrade_help);

CMD(view_trade);
CMD(buy_trade);
CMD(sell_trade);
CMD(recall_trade);

CMD(admin_add_coins);
CMD(admin_add_heroCoin);
CMD(admin_add_level);
CMD(admin_add_blessing);
CMD(admin_add_energy);
CMD(admin_add_exp);
CMD(admin_add_invCapacity);
CMD(admin_add_vip);
