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
    POST("/register", bg_cmd_register);
    GET("/viewcoins", bg_cmd_view_coins);
    POST("/signin", bg_cmd_sign_in);
    GET("/viewinv", bg_cmd_view_inventory);
    POST("/pawn", bg_cmd_pawn);
    GET("/viewprop", bg_cmd_view_properties);
    GET("/vieweqi", bg_cmd_view_equipments);
    POST("/equip", bg_cmd_equip);
    POST("/unequip", bg_cmd_unequip);
    POST("/unequipweapon", bg_cmd_unequip_weapon);
    POST("/unequiparmor", bg_cmd_unequip_armor);
    POST("/unequipornament", bg_cmd_unequip_ornament);
    POST("/unequipall", bg_cmd_unequip_all);


    return true;
}
