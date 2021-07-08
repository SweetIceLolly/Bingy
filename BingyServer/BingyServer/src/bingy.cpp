/*
描述: Bingy 服务器的入口点, 负责调用初始化函数和分发事件
作者: 冰棍
文件: bingy.cpp
*/

#include "bingy.hpp"
#include "utils.hpp"
#include "init.hpp"
#include "http_router.hpp"
#include <csignal>

rest_server gameServer;

int main() {
    auto startTime = std::chrono::high_resolution_clock::now();

    // 初始化 HTTP 服务器路由
    if (!init_server_router(gameServer)) {
        console_log("Bingy 初始化 HTTP 服务器路由失败!", LogType::error);
        return 1;
    }

    // 初始化 Bingy
    if (bg_init()) {
        std::chrono::duration<double> timeDiff = std::chrono::high_resolution_clock::now() - startTime;
        console_log("Bingy 加载成功, 用时" + std::to_string(timeDiff.count() * 1000) + "ms");
    }
    else {
        console_log("Bingy 初始化失败!", LogType::error);
        return 1;
    }

    // 拦截中断信号
    signal(SIGINT,
        [](int signum) {
            console_log("接收到退出信号, 正在关闭...\n");
            // 停止 HTTP 服务器
            gameServer.stopServer();
        }
    );
    
    // 启动 HTTP 服务器
    console_log("正在启动 HTTP 服务器...");
    gameServer.startServer("127.0.0.1:8000", 1, nullptr);

    // 最后收尾
    console_log("收尾完毕, 拜拜!\n");

    return 0;
}
