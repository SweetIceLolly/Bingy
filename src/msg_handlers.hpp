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
