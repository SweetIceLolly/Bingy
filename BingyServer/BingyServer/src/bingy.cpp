/*
描述: Bingy 服务器的入口点, 负责调用初始化函数和分发事件
作者: 冰棍
文件: bingy.cpp
*/

#include "bingy.hpp"
#include "utils.hpp"
#include "init.hpp"
#include "http_router.hpp"

int main() {
    rest_server server;

    // 初始化 HTTP 服务器路由
    init_server_router(server);

    // 初始化 Bingy
    auto startTime = std::chrono::high_resolution_clock::now();
    if (bg_init()) {
        std::chrono::duration<double> timeDiff = std::chrono::high_resolution_clock::now() - startTime;
        console_log("Bingy 加载成功, 用时" + std::to_string(timeDiff.count() * 1000) + "ms");
    }
    else {
        console_log("Bingy 初始化失败!", LogType::error);
        return 1;
    }
    
    // 启动 HTTP 服务器
    console_log("正在启动 HTTP 服务器...");
    server.startServer("127.0.0.1:8000", nullptr);

    return 0;
}
