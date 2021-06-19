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
CMD(view_coins);
CMD(sign_in);
CMD(view_inventory);
CMD(pawn);
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
