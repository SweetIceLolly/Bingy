/*
描述: Bingy 的入口点, 负责调用初始化函数和分发事件
作者: 冰棍
文件: bingy.cpp
*/

#include <cqcppsdk/cqcppsdk.hpp>
#include "init.hpp"
#include "msg_router.hpp"
#include "utils.hpp"

using namespace cq;

CQ_INIT{
    // 初始化 Bingy
    auto startTime = std::chrono::high_resolution_clock::now();
    if (bg_init()) {
        std::chrono::duration<double> timeDiff = std::chrono::high_resolution_clock::now() - startTime;
        console_log("Bingy 启动成功, 启动用时" + std::to_string(timeDiff.count() * 1000) + "ms");
    }
    else {
        console_log("Bingy 初始化失败!", LogType::error);
        return;
    }

    // 处理群消息
    on_group_message([](const auto &event) {
        try {
            bg_groupmsg_dispatch(event);
        }
        catch (const std::exception &e) {
            console_log(std::string("处理群聊消息时发生错误!\n") + std::string(e.what()), LogType::error);
        }
        catch (...) {
            console_log("处理群聊消息时发生错误, 无法获取错误信息", LogType::error);
        }
    });

    // 处理私聊
    on_private_message([](const auto &event) {
        try {
            bg_privatemsg_dispatch(event);
        }
        catch (const std::exception &e) {
            console_log(std::string("处理私聊消息时发生错误!\n") + std::string(e.what()), LogType::error);
        }
        catch (...) {
            console_log("处理私聊消息时发生错误, 无法获取错误信息!", LogType::error);
        }
    });
}
