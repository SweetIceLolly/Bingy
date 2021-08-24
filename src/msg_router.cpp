/*
描述: 消息路由, 负责消息分发到对应函数
作者: 冰棍
文件: msg_router.cpp
*/

#include "msg_router.hpp"
#include "trie.hpp"
#include "utils.hpp"

typedef std::function<void(const cq::MessageEvent &ev)> handlerFunc;

void bg_msg_parse_and_dispatch(cq::MessageEvent &ev, const bg_trie<handlerFunc> &router);

bg_trie<handlerFunc> gm_router, pm_router;

// 添加群聊消息路由
void bg_groupmsg_router_add(const std::string &msg, const handlerFunc &handler) {
    gm_router.addMsg(msg, handler);
}

// 添加私聊消息路由
void bg_privatemsg_router_add(const std::string &msg, const handlerFunc &handler) {
    pm_router.addMsg(msg, handler);
}

// 群聊消息分发
void bg_groupmsg_dispatch(const cq::MessageEvent &ev) {
    bg_msg_parse_and_dispatch((cq::MessageEvent)ev, gm_router);
}

// 私聊消息分发
void bg_privatemsg_dispatch(const cq::MessageEvent &ev) {
    bg_msg_parse_and_dispatch((cq::MessageEvent)ev, pm_router);
}

// 处理命令, 然后通过指定的路由分发消息
void bg_msg_parse_and_dispatch(cq::MessageEvent &ev, const bg_trie<handlerFunc> &router) {
    // 命令必须多于或等于两个字
    if (ev.message.length() < 2)
        return;

    // 去掉开头多余的空格和换行符
    size_t pos = 0;
    for (pos; pos < ev.message.length(); ++pos) {
        if (ev.message[pos] != ' ' && ev.message[pos] != '\r' && ev.message[pos] != '\n' && ev.message[pos] != '\0')
            break;
    }
    if (ev.message.length() - pos < 2)
        return;

    // 命令必须以"bg"开头
    if ((ev.message[pos] != 'b' && ev.message[pos] != 'B') || (ev.message[pos + 1] != 'g' && ev.message[pos + 1] != 'G'))
        return;

    // 把多余的换行符和 '\0' 替换为空格, 方便接下来处理
    for (size_t i = pos; i < ev.message.length(); ++i) {
        if (ev.message[i] == '\r' || ev.message[i] == '\n' || ev.message[i] == '\0')
            ev.message[i] = ' ';
    }

    // 获取命令字串, 在第二个空格处截断命令
    // 这样就不用一下子把整个消息字符串处理掉, 节省CPU资源
    bool spaceFound = false;
    std::string cmd = "";       // 处理完成后的命令
    for (pos; pos < ev.message.length(); ++pos) {
        if (ev.message[pos] == ' ' && pos + 1 < ev.message.length() && ev.message[pos + 1] != ' ') {
            if (!spaceFound) {
                cmd += ' ';
                spaceFound = true;
            }
            else
                break;
        }
        else if (ev.message[pos] != ' ') {
            if (ev.message[pos] >= '0' && ev.message[pos] <= '9')       // 是数字就马上截断. 因为命令中不可以有数字
                break;
            else if (ev.message[pos] == '[')                            // 是中括号则马上截断. 因为这可能是艾特的 CQ 码
                break;
            else if ((ev.message[pos] >= 'A' && ev.message[pos] <= 'Z') || (ev.message[pos] >= 'a' && ev.message[pos] <= 'z'))
                cmd += ev.message[pos] | 32;    // 转换为小写字母
            else
                cmd += ev.message[pos];
        }
    }

    // 查找对应的命令处理函数
    auto handler = router.getMsgHandler(cmd);

    // 移除字符串中所有多余的空格和换行符
    for (pos; pos < ev.message.length(); ++pos) {
        if (ev.message[pos] != ' ' || (ev.message[pos] == ' ' && pos + 1 < ev.message.length() && ev.message[pos + 1] != ' ')) {
            if ((ev.message[pos] >= 'A' && ev.message[pos] <= 'Z') || (ev.message[pos] >= 'a' && ev.message[pos] <= 'z'))
                cmd += ev.message[pos] | 32;    // 转换为小写字母
            else
                cmd += ev.message[pos];
        }
    }
    ev.message = cmd;

    // 有对应的命令处理函数则调用对应的函数, 否则调用聊骚接口
    if (handler)
        handler(ev);
    else
        bg_cmd_chat(ev);
}
