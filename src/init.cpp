/*
描述: 初始化相关代码
作者: 冰棍
文件: init.cpp
*/

#include "init.hpp"
#include "msg_router.hpp"
#include "msg_handlers.hpp"
#include "utils.hpp"
#include "config_parser.hpp"
#include "game.hpp"

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
    
    // 注册消息路由
    bg_msgrouter_init();

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
    bg_groupmsg_router_add("bg 硬币", bg_cmd_view_coins);
    bg_groupmsg_router_add("bg 查看背包", bg_cmd_view_inventory);
    bg_groupmsg_router_add("bg 背包", bg_cmd_view_inventory);
    bg_groupmsg_router_add("bg 查看属性", bg_cmd_view_properties);
    bg_groupmsg_router_add("bg 属性", bg_cmd_view_properties);

    bg_groupmsg_router_add("bg 查看装备", bg_cmd_view_equipments);
    bg_groupmsg_router_add("bg 我的装备", bg_cmd_view_equipments);
    bg_groupmsg_router_add("bg 查找装备", bg_cmd_search_equipments);
    bg_groupmsg_router_add("bg 装备", bg_cmd_equip);
    bg_groupmsg_router_add("bg 出售", bg_cmd_pawn);

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

    bg_groupmsg_router_add("bg 强化护甲", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 强化武器", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 强化饰品", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 强化装备", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 强化", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 升级护甲", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 升级武器", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 升级饰品", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 升级装备", bg_cmd_upgrade_help);
    bg_groupmsg_router_add("bg 升级", bg_cmd_upgrade_help);

    bg_groupmsg_router_add("bg 强化祝福", bg_cmd_upgrade_blessing);
    bg_groupmsg_router_add("bg 升级祝福", bg_cmd_upgrade_blessing);
    bg_groupmsg_router_add("bg 祝福", bg_cmd_blessing_help);

    bg_groupmsg_router_add("bg 交易场", bg_cmd_view_trade);
    bg_groupmsg_router_add("bg 查看交易场", bg_cmd_view_trade);
    bg_groupmsg_router_add("bg 交易", bg_cmd_view_trade);
    bg_groupmsg_router_add("bg 购买", bg_cmd_buy_trade);
    bg_groupmsg_router_add("bg 上架", bg_cmd_sell_trade);
    bg_groupmsg_router_add("bg 下架", bg_cmd_recall_trade);
    
    bg_groupmsg_router_add("bg 合成", bg_cmd_synthesis);
    bg_groupmsg_router_add("bg 挑战", bg_cmd_fight);
    bg_groupmsg_router_add("bg 挑战森林", nullptr);
    bg_groupmsg_router_add("bg pvp", bg_cmd_pvp);
    bg_groupmsg_router_add("bg vip", nullptr);

    bg_groupmsg_router_add("bg /addcoins", bg_cmd_admin_add_coins);
    bg_groupmsg_router_add("bg /addherocoin", bg_cmd_admin_add_heroCoin);
    bg_groupmsg_router_add("bg /addlevel", bg_cmd_admin_add_level);
    bg_groupmsg_router_add("bg /addblessing", bg_cmd_admin_add_blessing);
    bg_groupmsg_router_add("bg /addenergy", bg_cmd_admin_add_energy);
    bg_groupmsg_router_add("bg /addexp", bg_cmd_admin_add_exp);
    bg_groupmsg_router_add("bg /addinvcapacity", bg_cmd_admin_add_invCapacity);
    bg_groupmsg_router_add("bg /addvip", bg_cmd_admin_add_vip);
    bg_groupmsg_router_add("bg /setcoins", bg_cmd_admin_set_coins);
    bg_groupmsg_router_add("bg /setherocoin", bg_cmd_admin_set_heroCoin);
    bg_groupmsg_router_add("bg /setlevel", bg_cmd_admin_set_level);
    bg_groupmsg_router_add("bg /setblessing", bg_cmd_admin_set_blessing);
    bg_groupmsg_router_add("bg /setenergy", bg_cmd_admin_set_energy);
    bg_groupmsg_router_add("bg /setexp", bg_cmd_admin_set_exp);
    bg_groupmsg_router_add("bg /setinvcapacity", bg_cmd_admin_set_invCapacity);
    bg_groupmsg_router_add("bg /setvip", bg_cmd_admin_set_vip);

    // 注册私聊相关命令

}

// 读取游戏配置
inline bool bg_load_config() {
    configParser parser(CONFIG_FILE_PATH);

    return parser.load(
        // 切换 state 回调函数
        [](const std::string &line, char &state) -> bool {
            state = 0;
            return true;
        },

        // 获取属性值回调函数
        [&](const std::string &propName, const std::string &propValue, char state, unsigned int lineNo) -> bool {
            // 处理一般配置
            if (state == 0) {
                if (propName == "server") {                                 // 服务器地址
                    serverUri = propValue;
                    console_log("HTTP 服务器地址为 " + propValue);
                }
                else if (propName == "appid") {                             // 应用 ID
                    appId = propValue;
                    console_log("AppId 为 " + propValue);
                }
                else if (propName == "secret")                              // 应用密匙
                    appSecret = propValue;
                else
                    console_log(std::string("未知的配置名: \"") + propName + std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
            }
            return true;
        },

        // 开始标记回调函数
        [&](char state) -> bool {
            return true;
        },

        // 结束标记回调函数
        [&](char state) -> bool {
            return true;
        }
    );
}
