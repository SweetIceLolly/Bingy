/*
描述: 初始化相关代码
作者: 冰棍
文件: init.cpp
*/

#include "init.hpp"
#include "msg_router.hpp"
#include "msg_handlers.hpp"
#include "database.hpp"
#include "utils.hpp"
#include "player.hpp"
#include <iostream>
#include <fstream>

#define CONFIG_FILE_PATH    "bgConfig.txt"

inline void bg_msgrouter_init();
inline bool bg_load_config();

bool bgInitialized = false;     // Bingy 是否成功启动

bool bg_init() {
    // 加载配置文件
    console_log("正在读取配置文件...");
    if (!bg_load_config()) {
        console_log("读取配置文件失败!", LogType::error);
        return false;
    }
    console_log("成功读取配置文件");

    // 连接数据库
    console_log("正在连接数据库...");
    if (!dbInit()) {
        console_log("连接数据库失败!", LogType::error);
        return false;
    }
    console_log("成功连接数据库");

    // 加载怪物数据

    // 加载玩家信息
    console_log("正在读取所有玩家...");
    if (!bg_get_allplayers_from_db()) {
        console_log("读取玩家数据失败!", LogType::error);
        return false;
    }
    console_log("成功读取玩家数据, 共计" + std::to_string(allPlayers.size()) + "个玩家");

    // 注册消息路由
    bg_msgrouter_init();

    // 启动BgKeepAlive

    // 初始化完成
    bgInitialized = true;
    return true;
}

inline bool bg_load_config() {
    std::ifstream file;
    file.open(CONFIG_FILE_PATH);
    if (!file.is_open())
        return false;

    // 逐行读取配置文件
    std::string line;
    int         currLine = 0;
    char        state = 0;          // 0: 一般配置

    while (std::getline(file, line)) {
        ++currLine;
        line = str_trim(line);
        if (line.length() == 0)     // 忽略空行
            continue;

        if (line[0] == '[') {       // 状态设置行
            continue;
        }

        auto tmp = str_split(line, '=');
        str_lcase(tmp[0]);

        if (tmp[0] == "dburi")                              // 数据库 URI
            dbUri = tmp[1] + std::string("?authSource=admin");
        else if (tmp[0] == "dbname")                        // 数据库名
            dbName = tmp[1];
        else
            console_log(std::string("未知的配置名: \"") + tmp[0] + std::string("\", 于行") + std::to_string(currLine), LogType::warning);
    }

    file.close();
    return true;
}

inline void bg_msgrouter_init() {
    // 注册群聊相关命令
    bg_groupmsg_router_add("bg", bg_cmd_bg);
    bg_groupmsg_router_add("bg 注册", bg_cmd_register);
    bg_groupmsg_router_add("bg 查看硬币", bg_cmd_view_coins);
    bg_groupmsg_router_add("bg 查看属性", nullptr);
    bg_groupmsg_router_add("bg 签到", bg_cmd_sign_in);
    bg_groupmsg_router_add("bg 交易场", nullptr);
    bg_groupmsg_router_add("bg 合成", nullptr);
    bg_groupmsg_router_add("bg 挑战", nullptr);
    bg_groupmsg_router_add("bg 挑战森林", nullptr);
    bg_groupmsg_router_add("bg pvp", nullptr);
    bg_groupmsg_router_add("bg 查看装备", nullptr);
    bg_groupmsg_router_add("bg 购买", nullptr);
    bg_groupmsg_router_add("bg vip", nullptr);

    // 注册私聊相关命令
    
}
