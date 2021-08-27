/*
描述: 初始化相关代码
作者: 冰棍
文件: init.cpp
*/

#include "init.hpp"
#include "database.hpp"
#include "utils.hpp"
#include "player.hpp"
#include "monster.hpp"
#include "game.hpp"
#include "signin_event.hpp"
#include "equipment.hpp"
#include "synthesis.hpp"
#include "config_parser.hpp"
#include "trade.hpp"
#include "http_auth.hpp"
#include "secrets.hpp"
#include <iostream>
#include <fstream>

#define CONFIG_FILE_PATH    "bgConfig.txt"

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
    console_log("成功读取配置文件: 共计" + std::to_string(allSignInEvents.size()) + "个签到活动, " + std::to_string(allSyntheses.size()) + "个装备合成");

    // 加载怪物数据
    console_log("正在读取怪物数据...");
    if (!bg_load_monster_config()) {
        console_log("读取怪物数据文件失败!", LogType::error);
        return false;
    }
    bg_init_monster_chances();
    console_log("成功读取怪物数据: 共计" + std::to_string(allMonsters.size()) + "个怪物");

    // 加载装备数据
    console_log("正在读取装备数据...");
    if (!bg_load_equipment_config()) {
        console_log("读取装备数据文件失败!", LogType::error);
        return false;
    }
    console_log("成功读取装备数据: 共计" + std::to_string(allEquipments.size()) + "个装备");

    // 连接数据库
    console_log("正在连接数据库...");
    if (!dbInit()) {
        console_log("连接数据库失败!", LogType::error);
        return false;
    }
    console_log("成功连接数据库");

    // 加载玩家信息
    console_log("正在读取所有玩家数据...");
    if (!bg_get_all_players_from_db()) {
        console_log("读取玩家数据失败!", LogType::error);
        return false;
    }
    console_log("成功读取玩家数据: 共计" + std::to_string(allPlayers.size()) + "个玩家");

    // 加载交易场信息
    console_log("正在读取交易场数据...");
    bg_trade_get_items();
    console_log("成功读取交易场数据: 共计" + std::to_string(allTradeItems.size()) + "个条目, 下一个交易 ID 为" + std::to_string(bg_get_tradeId()));

#ifdef BINGY_ENABLE_SECRETS
    // 加载聊骚配置文件
    console_log("正在读取聊骚配置文件...");
    if (!bg_load_chat_config()) {
        console_log("读取聊骚配置文件失败!", LogType::error);
        return false;
    }
    console_log("成功读取聊骚配置文件");

    // 初始化所有彩蛋
    console_log("正在初始化彩蛋...");
    if (!bg_init_easter_eggs()) {
        console_log("彩蛋初始化失败!", LogType::error);
        return false;
    }
    console_log("彩蛋初始化完毕");
#endif

    // 启动 BgKeepAlive

    // 初始化完成
    bgInitialized = true;
    return true;
}

// 读取游戏配置
inline bool bg_load_config() {
    configParser    parser(CONFIG_FILE_PATH);
    signInEvent     *signInEv = nullptr;            // 签到活动临时变量
    synthesisInfo   *synInfo = nullptr;             // 合成信息临时变量
    dungeonData     *dungeon = nullptr;             // 副本配置临时变量
    std::string     httpAppId, httpAppSecret;       // HTTP 客户端配置临时变量

    return parser.load(
        // 切换 state 回调函数
        [](const std::string &line, char &state) -> bool {
            // state: 0: 一般配置; 1: 签到活动; 2: 装备合成; 3: 副本配置; 4: HTTP 客户端配置
            if (line == "[签到活动]")
                state = 1;
            else if (line == "[装备合成]")
                state = 2;
            else if (line == "[副本配置]")
                state = 3;
            else if (line == "[客户端配置]")
                state = 4;
            else
                state = 0;
            return true;
        },

        // 获取属性值回调函数
        [&](const std::string &propName, const std::string &propValue, char state, unsigned int lineNo) -> bool {
            // 处理一般配置
            if (state == 0) {
                if (propName == "dburi")                            // 数据库 URI
                    dbUri = propValue + std::string("?authSource=admin");
                else if (propName == "dbname")                      // 数据库名
                    dbName = propValue;
                else if (propName == "monsters")                    // 怪物配置路径
                    monsterConfigPath = propValue;
                else if (propName == "equipments")                  // 装备配置路径
                    eqiConfigPath = propValue;
                else if (propName == "chat")                        // 聊骚配置路径
                    chatConfigPath = propValue;
                else if (propName == "admin") {                     // 管理员
                    try {
                        LL qq = std::stoll(propValue);
                        if (allAdmins.insert(qq).second)
                            console_log("成功添加管理员: " + propValue);
                        else
                            console_log("把管理员" + propValue + "添加到管理员列表时发生错误, 可能是因为重复了? 于行" + std::to_string(lineNo), LogType::warning);
                    }
                    catch (...) {
                        console_log("无法把\"" + propValue + "\"添加到管理员列表, 请检查是否为有效数值! 于行" + std::to_string(lineNo), LogType::warning);
                    }
                }
                else
                    console_log(std::string("未知的配置名: \"") + propName +
                        std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
            }

            // 处理签到活动
            else if (state == 1) {
                try {
                    if (propName == "id")
                        signInEv->id = std::stoll(propValue);
                    else if (propName == "year")
                        signInEv->year = std::stoi(propValue);
                    else if (propName == "month")
                        signInEv->month = std::stoi(propValue);
                    else if (propName == "day")
                        signInEv->day = std::stoi(propValue);
                    else if (propName == "hour")
                        signInEv->hour = static_cast<char>(std::stoi(propValue));
                    else if (propName == "minute")
                        signInEv->minute = static_cast<char>(std::stoi(propValue));
                    else if (propName == "coinfactor")
                        signInEv->coinFactor = std::stod(propValue);
                    else if (propName == "energyfactor")
                        signInEv->energyFactor = std::stod(propValue);
                    else if (propName == "first")
                        signInEv->firstN = std::stoll(propValue);
                    else if (propName == "items") {
                        for (const auto &item : str_split(propValue, ',')) {
                            auto itemId = std::stoll(item);
                            if (itemId == -1)
                                break;
                            else
                                signInEv->items.push_back(itemId);
                        }
                    }
                    else if (propName == "message")
                        signInEv->message = propValue;
                    else
                        console_log(std::string("未知的配置名: \"") + propName +
                            std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("处理签到活动配置时发生错误: 于行" + std::to_string(lineNo) + ", 原因: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("处理签到活动配置时发生错误: 于行" + std::to_string(lineNo), LogType::warning);
                }
            }

            // 处理装备合成信息
            else if (state == 2) {
                try {
                    if (propName == "requirements") {
                        for (const auto &item : str_split(propValue, ',')) {
                            auto itemId = std::stoll(item);
                            if (itemId == -1)
                                break;
                            else
                                synInfo->requirements.insert(itemId);
                        }
                    }
                    else if (propName == "coins")
                        synInfo->coins = std::stoll(propValue);
                    else if (propName == "target")
                        synInfo->targetId = std::stoll(propValue);
                    else
                        console_log(std::string("未知的配置名: \"") + propName +
                            std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("添加合成信息失败, 处理配置时发生错误: 于行" + std::to_string(lineNo) + ", 原因: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("添加合成信息失败, 处理配置时发生错误: 于行" + std::to_string(lineNo), LogType::warning);
                }
            }

            // 处理副本配置
            else if (state == 3) {
                try {
                    if (propName == "level")
                        dungeon->level = std::stoll(propValue);
                    else if (propName == "monsters") {
                        for (const auto &idStr : str_split(propValue, ',')) {
                            const auto id = std::stoll(idStr);
                            dungeon->monsters.push_back(id);
                        }
                    }
                    else
                        console_log(std::string("未知的配置名: \"") + propName +
                            std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("添加副本配置失败, 处理配置时发生错误: 于行" + std::to_string(lineNo) + ", 原因: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("添加副本配置失败, 处理配置时发生错误: 于行" + std::to_string(lineNo), LogType::warning);
                }
            }

            // 处理 HTTP 客户端配置
            else if (state == 4) {
                try {
                    if (propName == "appid")
                        httpAppId = propValue;
                    else if (propName == "secret")
                        httpAppSecret = propValue;
                    else
                        console_log(std::string("未知的配置名: \"") + propName +
                            std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
                }
                catch (const std::exception &e) {
                    console_log("添加客户端配置失败, 处理配置时发生错误: 于行" + std::to_string(lineNo) + ", 原因: " + e.what(), LogType::warning);
                }
                catch (...) {
                    console_log("添加客户端配置失败, 处理配置时发生错误: 于行" + std::to_string(lineNo), LogType::warning);
                }
            }

            return true;
        },

        // 开始标记回调函数
        [&](char state) -> bool {
            if (state == 1)
                signInEv = new signInEvent();
            else if (state == 2)
                synInfo = new synthesisInfo();
            else if (state == 3)
                dungeon = new dungeonData();
            else if (state == 4) {
                httpAppSecret = "";
                httpAppId = "";
            }

            return true;
        },

        // 结束标记回调函数
        [&](char state) -> bool {
            if (state == 1) {
                allSignInEvents.push_back(*signInEv);
                delete signInEv;
            }
            else if (state == 2) {
                allSyntheses.insert({ synInfo->targetId, *synInfo });
                delete synInfo;
            }
            else if (state == 3) {
                allDungeons.insert({ dungeon->level, *dungeon });
            }
            else if (state == 4) {
                bg_http_add_app(httpAppId, httpAppSecret);
                console_log("成功添加 HTTP 客户端: " + httpAppId);
            }

            return true;
        }
    );
}
