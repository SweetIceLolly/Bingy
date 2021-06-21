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
#include "config_parser.hpp"
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
    console_log("成功读取配置文件: 共计" + std::to_string(allSignInEvents.size()) + "个签到活动, " + std::to_string(allSynthesises.size()) + "个装备合成");

    // 加载怪物数据
    console_log("正在读取怪物数据...");
    if (!bg_load_monster_config()) {
        console_log("读取怪物数据文件失败!", LogType::error);
        return false;
    }
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
    console_log("正在读取所有玩家...");
    if (!bg_get_allplayers_from_db()) {
        console_log("读取玩家数据失败!", LogType::error);
        return false;
    }
    console_log("成功读取玩家数据: 共计" + std::to_string(allPlayers.size()) + "个玩家");

    // 注册消息路由
    bg_msgrouter_init();

    // 启动 BgKeepAlive

    // 初始化完成
    bgInitialized = true;
    return true;
}

// 注册所有命令
inline void bg_msgrouter_init() {
    // 注册群聊相关命令
    bg_groupmsg_router_add("bg", bg_cmd_bg);
    bg_groupmsg_router_add("bg 注册", bg_cmd_register);
    bg_groupmsg_router_add("bg 签到", bg_cmd_sign_in);

    bg_groupmsg_router_add("bg 查看硬币", bg_cmd_view_coins);
    bg_groupmsg_router_add("bg 查看背包", bg_cmd_view_inventory);
    bg_groupmsg_router_add("bg 查看属性", bg_cmd_view_properties);

    bg_groupmsg_router_add("bg 查看装备", bg_cmd_view_equipments);
    bg_groupmsg_router_add("bg 装备", bg_cmd_equip);

    bg_groupmsg_router_add("bg 卸下头盔", bg_cmd_unequip_helmet);
    bg_groupmsg_router_add("bg 卸下战甲", bg_cmd_unequip_body);
    bg_groupmsg_router_add("bg 卸下护腿", bg_cmd_unequip_leg);
    bg_groupmsg_router_add("bg 卸下战靴", bg_cmd_unequip_boot);
    bg_groupmsg_router_add("bg 卸下护甲", bg_cmd_unequip_armor);

    bg_groupmsg_router_add("bg 卸下主武器", bg_cmd_unequip_primary);
    bg_groupmsg_router_add("bg 卸下副武器", bg_cmd_unequip_secondary);
    bg_groupmsg_router_add("bg 卸下武器", bg_cmd_unequip_weapon);

    bg_groupmsg_router_add("bg 卸下耳环", bg_cmd_unequip_earrings);
    bg_groupmsg_router_add("bg 卸下戒指", bg_cmd_unequip_rings);
    bg_groupmsg_router_add("bg 卸下项链", bg_cmd_unequip_necklace);
    bg_groupmsg_router_add("bg 卸下宝石", bg_cmd_unequip_jewelry);
    bg_groupmsg_router_add("bg 卸下饰品", bg_cmd_unequip_ornament);

    bg_groupmsg_router_add("bg 卸下", bg_cmd_unequip_item);
    bg_groupmsg_router_add("bg 卸下装备", bg_cmd_unequip_item_2);
    bg_groupmsg_router_add("bg 卸下所有", bg_cmd_unequip_all);
    bg_groupmsg_router_add("bg 卸下所有装备", bg_cmd_unequip_all);
    bg_groupmsg_router_add("bg 卸下全部", bg_cmd_unequip_all);
    bg_groupmsg_router_add("bg 卸下全部装备", bg_cmd_unequip_all);

    bg_groupmsg_router_add("bg 强化头盔", bg_cmd_upgrade_helmet);
    bg_groupmsg_router_add("bg 强化战甲", bg_cmd_upgrade_body);
    bg_groupmsg_router_add("bg 强化护腿", bg_cmd_upgrade_leg);
    bg_groupmsg_router_add("bg 强化战靴", bg_cmd_upgrade_boot);
    bg_groupmsg_router_add("bg 升级头盔", bg_cmd_upgrade_helmet);
    bg_groupmsg_router_add("bg 升级战甲", bg_cmd_upgrade_body);
    bg_groupmsg_router_add("bg 升级护腿", bg_cmd_upgrade_leg);
    bg_groupmsg_router_add("bg 升级战靴", bg_cmd_upgrade_boot);

    bg_groupmsg_router_add("bg 强化主武器", bg_cmd_upgrade_primary);
    bg_groupmsg_router_add("bg 强化副武器", bg_cmd_upgrade_secondary);
    bg_groupmsg_router_add("bg 升级主武器", bg_cmd_upgrade_primary);
    bg_groupmsg_router_add("bg 升级副武器", bg_cmd_upgrade_secondary);

    bg_groupmsg_router_add("bg 强化耳环", bg_cmd_upgrade_earrings);
    bg_groupmsg_router_add("bg 强化戒指", bg_cmd_upgrade_rings);
    bg_groupmsg_router_add("bg 强化项链", bg_cmd_upgrade_necklace);
    bg_groupmsg_router_add("bg 强化宝石", bg_cmd_upgrade_jewelry);
    bg_groupmsg_router_add("bg 升级耳环", bg_cmd_upgrade_earrings);
    bg_groupmsg_router_add("bg 升级戒指", bg_cmd_upgrade_rings);
    bg_groupmsg_router_add("bg 升级项链", bg_cmd_upgrade_necklace);
    bg_groupmsg_router_add("bg 升级宝石", bg_cmd_upgrade_jewelry);
    bg_groupmsg_router_add("bg 确认", bg_cmd_confirm_upgrade);

    bg_groupmsg_router_add("bg 出售", bg_cmd_pawn);
    bg_groupmsg_router_add("bg 交易场", nullptr);
    bg_groupmsg_router_add("bg 合成", nullptr);
    bg_groupmsg_router_add("bg 挑战", nullptr);
    bg_groupmsg_router_add("bg 挑战森林", nullptr);
    bg_groupmsg_router_add("bg pvp", nullptr);
    bg_groupmsg_router_add("bg 购买", nullptr);
    bg_groupmsg_router_add("bg vip", nullptr);

    // 注册私聊相关命令

}

// 读取游戏配置
inline bool bg_load_config() {
    configParser parser(CONFIG_FILE_PATH);
    signInEvent *signInEv = nullptr;        // 签到活动临时变量
    synthesisInfo *synInfo = nullptr;       // 合成信息临时变量

    return parser.load(
        // 切换 state 回调函数
        [](const std::string &line, char &state) -> bool {
            // state: 0: 一般配置; 1: 签到活动; 2: 装备合成
            if (line == "[签到活动]")
                state = 1;
            else if (line == "[装备合成]")
                state = 2;
            else
                state = 0;
            return true;
        },

        // 获取属性值回调函数
        [&](const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo) -> bool {
            // 处理一般配置
            if (state == 0) {
                if (propName == "dburi")                              // 数据库 URI
                    dbUri = propValue + std::string("?authSource=admin");
                else if (propName == "dbname")                        // 数据库名
                    dbName = propValue;
                else if (propName == "monsters")                      // 怪物配置路径
                    monsterConfigPath = propValue;
                else if (propName == "equipments")                    // 装备配置路径
                    eqiConfigPath = propValue;
                else if (propName == "admin") {                       // 管理员
                    try {
                        LL qq = std::stoll(propValue);
                        if (allAdmins.insert(qq).second)
                            console_log("成功添加管理员: " + propValue);
                        else
                            console_log("把管理员" + propValue + "添加到管理员列表时发生错误, 可能是因为重复了? 于行" + std::to_string(lineNo), LogType::warning);
                    }
                    catch (...) {
                        console_log("无法把\"" + propValue + "\"添加到管理员列表, 勤检查是否为有效数值! 于行" + std::to_string(lineNo), LogType::warning);
                    }
                }
                else
                    console_log(std::string("未知的配置名: \"") + propName + std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
            }

            // 处理签到活动
            else if (state == 1) {                                          // 配置
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
                        signInEv->hour = (char)std::stoi(propValue);
                    else if (propName == "minute")
                        signInEv->minute = (char)std::stoi(propValue);
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
                        console_log(std::string("未知的配置名: \"") + propName + std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
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
                                synInfo->requirements.push_back(itemId);
                        }
                    }
                    else if (propName == "coins")
                        synInfo->coins = std::stoll(propValue);
                    else if (propName == "target")
                        synInfo->targetId = std::stoll(propValue);
                }
                catch (...) {
                    console_log("添加合成信息失败, 处理配置时发生错误: 于行" + std::to_string(lineNo), LogType::warning);
                }
            }

            return true;
        },

        // 开始标记回调函数
        [&](const char &state) -> bool {
            if (state == 1)
                signInEv = new signInEvent();
            else if (state == 2)
                synInfo = new synthesisInfo();

            return true;
        },

        // 结束标记回调函数
        [&](const char &state) -> bool {
            if (state == 1) {
                allSignInEvents.push_back(*signInEv);
                delete signInEv;
            }
            else if (state == 2) {
                allSynthesises.push_back(*synInfo);
                delete synInfo;
            }

            return true;
        }
    );
}
