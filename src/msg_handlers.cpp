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
    if (ev.message.length() < 10) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 出售指令格式为: \"bg 出售 背包序号1 背包序号2 ...\"");
        return;
    }
    auto params = str_split(ev.message.substr(10), ' ');         // 去掉命令字符串, 然后以空格分隔开参数
    if (params.size() > 0) {
        std::vector<LL> items;                                  // prePawnCallback 返回的处理之后的序号列表
        if (prePawnCallback(ev, params, items))
            postPawnCallback(ev, items);
    }
    else {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 出售指令格式为: \"bg 出售 背包序号1 背包序号2 ...\"");
    }
}
