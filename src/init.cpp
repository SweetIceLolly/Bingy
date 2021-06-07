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
#include "monster.hpp"
#include "game.hpp"
#include "signin_event.hpp"
#include "equipment.hpp"
#include "synthesis.hpp"
#include <iostream>
#include <fstream>

#define CONFIG_FILE_PATH    "bgConfig.txt"

inline void bg_msgrouter_init();
inline bool bg_load_config();

bool bgInitialized = false;     // Bingy 是否成功启动

// 初始化主函数
bool bg_init() {
    // 加载配置文件
    console_log("正在读取配置文件...");
    if (!bg_load_config()) {
        console_log("读取配置文件失败!", LogType::error);
        return false;
    }
    console_log("成功读取配置文件");
    console_log("共计" + std::to_string(allSignInEvents.size()) + "个签到活动, " + std::to_string(allSynthesises.size()) + "个装备合成");

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

    // 启动 BgKeepAlive

    // 初始化完成
    bgInitialized = true;
    return true;
}

// 读取游戏配置
inline bool bg_load_config() {
    std::ifstream file;
    file.open(CONFIG_FILE_PATH);
    if (!file.is_open())
        return false;

    // 逐行读取配置文件
    std::string     line;
    int             currLine = 0;
    char            state = 0;              // 0: 一般配置; 1: 签到活动; 2: 装备合成
    signInEvent     *signInEv = nullptr;    // 签到活动临时变量
    synthesisInfo   *synInfo = nullptr;     // 合成信息临时变量

    while (std::getline(file, line)) {
        ++currLine;
        line = str_trim(line);
        if (line.length() == 0)         // 忽略空行
            continue;

        if (line[0] == '[') {           // 状态设置行
            if (line == "[签到活动]")
                state = 1;
            else if (line == "[装备合成]")
                state = 2;
            else
                state = 0;
            continue;
        }

        auto tmp = str_split(line, '=');
        str_lcase(tmp[0]);

        // 处理一般配置
        if (state == 0) {
            if (tmp[0] == "dburi")                              // 数据库 URI
                dbUri = tmp[1] + std::string("?authSource=admin");
            else if (tmp[0] == "dbname")                        // 数据库名
                dbName = tmp[1];
            else if (tmp[0] == "monsters")                      // 怪物配置路径
                monsterConfigPath = tmp[1];
            else if (tmp[0] == "equipments")                    // 装备配置路径
                eqiConfigPath = tmp[1];
            else if (tmp[0] == "admin") {                       // 管理员
                try {
                    LL qq = std::stoll(tmp[1]);
                    if (allAdmins.insert(qq).second)
                        console_log("成功添加管理员: " + tmp[1]);
                    else
                        console_log("把管理员" + tmp[1] + "添加到管理员列表时发生错误, 可能是因为重复了? 于行" + std::to_string(currLine), LogType::warning);
                }
                catch (...) {
                    console_log("无法把\"" + tmp[1] + "\"添加到管理员列表, 勤检查是否为有效数值! 于行" + std::to_string(currLine), LogType::warning);
                }
            }
            else
                console_log(std::string("未知的配置名: \"") + tmp[0] + std::string("\", 于行") + std::to_string(currLine), LogType::warning);
        }

        // 处理签到活动
        else if (state == 1) {
            if (line == "begin")                                // 开始标记
                signInEv = new signInEvent();
            else if (line == "end") {                           // 结束标记
                allSignInEvents.push_back(*signInEv);
                delete signInEv;
            }
            else {                                              // 配置
                try {
                    if (tmp[0] == "year")
                        signInEv->year = std::stoi(tmp[1]);
                    else if (tmp[0] == "month")
                        signInEv->month = std::stoi(tmp[1]);
                    else if (tmp[0] == "day")
                        signInEv->day = std::stoi(tmp[1]);
                    else if (tmp[0] == "hour")
                        signInEv->hour = (char)std::stoi(tmp[1]);
                    else if (tmp[0] == "minute")
                        signInEv->minute = (char)std::stoi(tmp[1]);
                    else if (tmp[0] == "first")
                        signInEv->firstN = std::stoll(tmp[1]);
                    else if (tmp[0] == "coinfactor")
                        signInEv->coinFactor = std::stod(tmp[1]);
                    else if (tmp[0] == "energyfactor")
                        signInEv->energyFactor = std::stod(tmp[1]);
                    else if (tmp[0] == "items") {
                        for (const auto &item : str_split(tmp[1], ',')) {
                            auto itemId = std::stoll(item);
                            if (itemId == -1)
                                break;
                            else
                                signInEv->items.push_back(itemId);
                        }
                    }
                    else if (tmp[0] == "message")
                        signInEv->message = tmp[1];
                    else
                        console_log(std::string("未知的配置名: \"") + tmp[0] + std::string("\", 于行") + std::to_string(currLine), LogType::warning);
                }
                catch (...) {
                    console_log("添加签到活动失败, 处理配置时发生错误: 于行" + std::to_string(currLine), LogType::warning);
                }
            }
        }

        // 处理装备合成信息
        else if (state == 2) {
            if (line == "begin")                                // 开始标记
                synInfo = new synthesisInfo();
            else if (line == "end") {                           // 结束标记
                allSynthesises.push_back(*synInfo);
                delete synInfo;
            }
            else {
                try {
                    if (tmp[0] == "requirements") {
                        for (const auto &item : str_split(tmp[1], ',')) {
                            auto itemId = std::stoll(item);
                            if (itemId == -1)
                                break;
                            else
                                synInfo->requirements.push_back(itemId);
                        }
                    }
                    else if (tmp[0] == "coins")
                        synInfo->coins = std::stoll(tmp[1]);
                    else if (tmp[0] == "target")
                        synInfo->targetId = std::stoll(tmp[1]);
                }
                catch (...) {
                    console_log("添加合成信息失败, 处理配置时发生错误: 于行" + std::to_string(currLine), LogType::warning);
                }
            }
        }
    }

    file.close();
    return true;
}

// 注册所有命令
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
