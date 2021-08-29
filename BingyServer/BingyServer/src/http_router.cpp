/*
描述: 初始化 Bingy 服务器的路由
作者: 冰棍
文件: http_router.cpp
*/

#include "http_router.hpp"
#include "http_handlers.hpp"

#define POST(url, func) server.addHandler("POST", url, func);
#define GET(url, func) server.addHandler("GET", url, func);

// 初始化 HTTP 路由
bool init_server_router(rest_server &server) {
    GET("/viewcoins", bg_cmd_view_coins);
    GET("/viewinv", bg_cmd_view_inventory);
    GET("/viewprop", bg_cmd_view_properties);
    GET("/vip", bg_cmd_view_vip);

    POST("/register", bg_cmd_register);
    POST("/signin", bg_cmd_sign_in);
    POST("/pawn", bg_cmd_pawn);
    POST("/fight", bg_cmd_fight);
    POST("/pvp", bg_cmd_pvp);

    GET("/vieweqi", bg_cmd_view_equipments);
    GET("/searcheqi", bg_cmd_search_equipments);
    POST("/equip", bg_cmd_equip);
    POST("/unequip", bg_cmd_unequip);
    POST("/unequipweapon", bg_cmd_unequip_weapon);
    POST("/unequiparmor", bg_cmd_unequip_armor);
    POST("/unequipornament", bg_cmd_unequip_ornament);
    POST("/unequipall", bg_cmd_unequip_all);

    POST("/upgrade", bg_cmd_upgrade);
    POST("/confirm", bg_cmd_confirm_upgrade);

    POST("/upgradeblessing", bg_cmd_upgrade_blessing);

    GET("/viewtrade", bg_cmd_view_trade);
    POST("/buytrade", bg_cmd_buy_trade);
    POST("/selltrade", bg_cmd_sell_trade);
    POST("/recalltrade", bg_cmd_recall_trade);

    POST("/synthesis", bg_cmd_synthesis);

    POST("/adminmodifyfield", bg_cmd_admin_modify_field);

    POST("/chat", bg_cmd_chat);

    return true;
}
