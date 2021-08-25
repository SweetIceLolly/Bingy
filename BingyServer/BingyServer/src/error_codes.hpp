/*
描述: Bingy HTTP 请求返回值错误码列表
作者: 冰棍
文件: error_codes.hpp
*/

#pragma once

// A. 请求层面错误
#define BG_ERR_STR_INVALID_REQUEST                  "无效的请求内容"
#define BG_ERR_INVALID_REQUEST                      1000

#define BG_ERR_STR_AUTH_FAILED                      "无法验证appid"
#define BG_ERR_AUTH_FAILED                          1001

#define BG_ERR_STR_MALLOC                           "申请内存失败, 无法处理请求"
#define BG_ERR_MALLOC                               1002
#define BG_ERR_STR_MALLOC_VAL                       "1002"

// B. 游戏层面错误

// B-0. 未知错误
// B-0.a. 操作前检查发生错误 (400 错误)
#define BG_ERR_STR_PRE_OP_FAILED                    "操作前检查发生错误"
#define BG_ERR_PRE_OP_FAILED                        0

#define BG_ERR_STR_GAME_PAUSED                      "游戏被暂停"
#define BG_ERR_GAME_PAUSED                          1

// B-0.b. 操作时发生错误 (500 错误)
#define BG_ERR_STR_POST_OP_FAILED                   "执行指定操作时发生错误"
#define BG_ERR_POST_OP_FAILED                       50

// B-1. 通用账户检查错误
// B-1.a. 400 错误
#define BG_ERR_STR_UNREGISTERED                     "玩家未注册"
#define BG_ERR_UNREGISTERED                         2000

#define BG_ERR_STR_BLACKLISTED                      "玩家在黑名单中"
#define BG_ERR_BLACKLISTED                          2001

#define BG_ERR_STR_NOT_ADMIN                        "非管理员不得使用管理指令"
#define BG_ERR_NOT_ADMIN                            2002

// B-2. 新玩家注册错误
// B-2-a. 400 错误
#define BG_ERR_STR_ALREADY_REGISTERED               "玩家重复注册"
#define BG_ERR_ALREADY_REGISTERED                   2100

// B-3. 查看硬币
// (无)

// B-4. 签到错误
// B-4-a. 400 错误
#define BG_ERR_STR_ALREADY_SIGNED_IN                "玩家重复签到"
#define BG_ERR_ALREADY_SIGNED_IN                    2300

// B-4-b. 500 错误
#define BG_ERR_STR_SIGN_IN_SET_CONT_FAILED          "设置连续签到天数失败"
#define BG_ERR_SIGN_IN_SET_CONT_FAILED              2350

#define BG_ERR_STR_SIGN_IN_SET_TIME_FAILED          "设置签到时间失败"
#define BG_ERR_SIGN_IN_SET_TIME_FAILED              2351

#define BG_ERR_STR_SIGN_IN_INC_COUNT_FAILED         "设置签到次数失败"
#define BG_ERR_SIGN_IN_INC_COUNT_FAILED             2352

#define BG_ERR_STR_ADD_ITEM_FAILED                  "为玩家添加物品失败"
#define BG_ERR_ADD_ITEM_FAILED                      2353

#define BG_ERR_STR_INC_COINS_FAILED                 "为玩家添加硬币失败"
#define BG_ERR_INC_COINS_FAILED                     2354

#define BG_ERR_STR_INC_ENERGY_FAILED                "为玩家添加体力失败"
#define BG_ERR_INC_ENERGY_FAILED                    2355

#define BG_ERR_STR_INC_EXP_FAILED                   "为玩家添加经验失败"
#define BG_ERR_INC_EXP_FAILED                       2356

// B-5. 查看背包错误
// (无)

// B-6. 出售错误
// B-6.a. 400 错误
#define BG_ERR_STR_ID_OUT_OF_RANGE                  "序号超出了背包范围"
#define BG_ERR_ID_OUT_OF_RANGE                      2500

#define BG_ERR_STR_ID_REPEATED                      "序号重复了"
#define BG_ERR_ID_REPEATED                          2501

// B-6.b. 500 错误
#define BG_ERR_STR_REMOVE_ITEM_FAILED               "从玩家背包移除物品失败"
#define BG_ERR_REMOVE_ITEM_FAILED                   2550

// B.7. 查看属性错误
// (无)

// B.8. 查看装备错误
// (无)

// B.9. 装备错误
// B.9.a 400 错误
#define BG_ERR_STR_MAX_ITEMS                        "最多只能装备5个一次性物品"
#define BG_ERR_MAX_ITEMS                            2800

#define BG_ERR_STR_ITEM_REPEATED                    "装备的一次性物品不得重复"
#define BG_ERR_ITEM_REPEATED                        2801

// B.9.b. 500 错误
#define BG_ERR_STR_EQUIP_REMOVE_FAILED              "把将要装备的装备从背包中移除时发生错误"
#define BG_ERR_EQUIP_REMOVE_FAILED                  2850

#define BG_ERR_STR_EQUIP_UPDATE_FAILED              "修改玩家装备时发生错误"
#define BG_ERR_EQUIP_UPDATE_FAILED                  2851

#define BG_ERR_STR_EQUIP_ADD_FAILED                 "把之前的装备放回背包时发生错误"
#define BG_ERR_EQUIP_ADD_FAILED                     2852

// B.10. 卸下装备错误
// B.10.a. 400 错误
#define BG_ERR_STR_NOT_EQUIPPED                     "当前未装备对应类型的装备"
#define BG_ERR_NOT_EQUIPPED                         2900

#define BG_ERR_STR_SINGLE_OUT_OF_RANGE              "指定了无效的一次性物品序号"
#define BG_ERR_SINGLE_OUT_OF_RANGE                  2901

// B.10.b. 500 错误
#define BG_ERR_STR_CLEAR_SINGLE_FAILED              "清空一次性装备时发生错误"
#define BG_ERR_CLEAR_SINGLE_FAILED                  2950

#define BG_ERR_STR_SINGLE_ADD_FAILED                "把一次性装备添加到背包时发生错误"
#define BG_ERR_SINGLE_ADD_FAILED                    2951

#define BG_ERR_STR_REMOVE_SINGLE_FAILED             "移除一次性装备时发生错误"
#define BG_ERR_REMOVE_SINGLE_FAILED                 2952

// B.11. 装备强化错误
// B.11.a. 400 错误
#define BG_ERR_STR_INVALID_UPGRADE_TIMES            "无效的升级次数"
#define BG_ERR_INVALID_UPGRADE_TIMES                3000

#define BG_ERR_STR_MAX_UPGRADE_TIMES                "超出了升级次数限制"
#define BG_ERR_MAX_UPGRADE_TIMES                    3001

#define BG_ERR_STR_INSUFFICIENT_COINS               "硬币不足"
#define BG_ERR_INSUFFICIENT_COINS                   3002

#define BG_ERR_STR_UPGRADE_CANCELED                 "连续升级被取消"
#define BG_ERR_UPGRADE_CANCELED                     3003

#define BG_ERR_STR_NO_PENDING_UPGRADE               "目前没有需要确认的连续强化操作"
#define BG_ERR_NO_PENDING_UPGRADE                   3004

// B.11.b. 500 错误
#define BG_ERR_STR_DEC_COINS_FAILED                 "扣除玩家硬币时发生错误"
#define BG_ERR_DEC_COINS_FAILED                     3050

#define BG_ERR_STR_SET_EQI_FAILED                   "设置玩家装备时发生错误"
#define BG_ERR_SET_EQI_FAILED                       3051

// B.12. 查看交易场错误
// (无)

// B.13. 购买交易场商品错误
// B.13.a. 400 错误
#define BG_ERR_STR_TRADEID_INVALID                  "指定的交易ID无效"
#define BG_ERR_TRADEID_INVALID                      3200

#define BG_ERR_STR_PASSWORD_REQUIRED                "指定交易需要购买密码"
#define BG_ERR_PASSWORD_REQUIRED                    3201

#define BG_ERR_STR_PASSWORD_INCORRECT               "购买密码错误"
#define BG_ERR_PASSWORD_INCORRECT                   3202

#define BG_ERR_STR_PASSWORD_NOT_REQUIRED            "指定交易不需要密码, 但是提供了密码"
#define BG_ERR_PASSWORD_NOT_REQUIRED                3203

// B.13.b. 500 错误
#define BG_ERR_STR_REMOVE_TRADE_FAILED              "从交易场移除商品失败"
#define BG_ERR_REMOVE_TRADE_FAILED                  3250

// B.14. 上架交易场商品错误
// B.14.a. 400 错误
#define BG_ERR_STR_INAPPROPRIATE_PRICE              "不合理的价格"
#define BG_ERR_INAPPROPRIATE_PRICE                  3300

#define BG_ERR_STR_CANT_AFFORD_TAX                  "不够钱交税"
#define BG_ERR_CANT_AFFORD_TAX                      3301

// B.14.b. 500 错误
#define BG_ERR_STR_TRADEID_UPDATE_FAILED            "更新 TradeId 失败"
#define BG_ERR_TRADEID_UPDATE_FAILED                3350

#define BG_ERR_STR_ADD_TRADE_FAILED                 "把物品添加到交易场失败"
#define BG_ERR_ADD_TRADE_FAILED                     3351

// B.15. 下架交易场商品错误
// B.15.a. 400 错误
#define BG_ERR_STR_PLAYER_MISMATCH                  "上架者并非玩家"
#define BG_ERR_PLAYER_MISMATCH                      3400

// B.16. 合成装备错误
// B.16.a. 400 错误
#define BG_ERR_STR_INVALID_EQI_ID                   "指定装备ID无效"
#define BG_ERR_INVALID_EQI_ID                       3500

#define BG_ERR_STR_CANT_SYNTHESIS                   "指定装备不能合成"
#define BG_ERR_CANT_SYNTHESIS                       3501

#define BG_ERR_STR_SYNTHESIS_NOT_EXIST              "没有指定的合成"
#define BG_ERR_SYNTHESIS_NOT_EXIST                  3502

#define BG_ERR_STR_EQI_NOT_FOUND                    "没有找到指定的装备"
#define BG_ERR_EQI_NOT_FOUND                        3503

// B.17. 挑战副本错误
// B.17.a. 400 错误
#define BG_ERR_STR_INVALID_DUNGEON                  "无效的副本号"
#define BG_ERR_INVALID_DUNGEON                      3600

#define BG_ERR_STR_NO_ENERGY                        "玩家体力已耗尽"
#define BG_ERR_NO_ENERGY                            3601

#define BG_ERR_STR_IN_CD                            "挑战冷却还没结束"
#define BG_ERR_IN_CD                                3602

// B.17.b. 500 错误
#define BG_ERR_STR_DEC_ENERGY_FAILED                "为玩家扣除体力失败"
#define BG_ERR_DEC_ENERGY_FAILED                    3650

#define BG_ERR_STR_SET_FIGHTTIME_FAILED             "设置玩家最后挑战时间失败"
#define BG_ERR_SET_FIGHTTIME_FAILED                 3651

// B.18. 管理员修改玩家属性值错误
// B.18.a. 400 错误
#define BG_ERR_STR_INVALID_ADMIN_REQUEST            "无效的管理指令调用"
#define BG_ERR_INVALID_ADMIN_REQUEST                3700

#define BG_ERR_STR_INVALID_TARGET                   "指定的目标玩家不存在"
#define BG_ERR_INVALID_TARGET                       3701

// B.19. 升级祝福错误
// B.19.a. 500 错误
#define BG_ERR_STR_INC_BLESSING_FAILED              "为玩家添加祝福等级失败"
#define BG_ERR_INC_BLESSING_FAILED                  3850

// B.20. 查找装备错误
// B.20.a. 400 错误
#define BG_ERR_STR_KEYWORD_TOO_LONG                 "关键字过长"
#define BG_ERR_KEYWORD_TOO_LONG                     3900

// 错误号对应到回应字符串 (适用于 Bingy 客户端代码)
// 以下宏用来检查是否为 Bingy 客户端
#ifdef DEFAULT_SERVER_URI
#include <unordered_map>
extern const std::unordered_map<int, const char *> error_desc;
const std::unordered_map<int, const char *> error_desc = {
    { BG_ERR_ALREADY_REGISTERED, "你已经注册过啦!" },
    { BG_ERR_UNREGISTERED, "要先注册哦! 快发送\"bg 注册\"加入游戏吧!" },
    { BG_ERR_BLACKLISTED, "你被拉黑了!" },
    { BG_ERR_ALREADY_SIGNED_IN, "你今天已经签到过了哦! 明天再来签到吧!" },
    { BG_ERR_ID_OUT_OF_RANGE, "" },
    { BG_ERR_ID_REPEATED, "" },
    { BG_ERR_NOT_EQUIPPED, "没有装备这个类型的装备哦!" },
    { BG_ERR_SINGLE_OUT_OF_RANGE, "指定的一次性物品序号超出了范围!" },
    { BG_ERR_INSUFFICIENT_COINS, "" },
    { BG_ERR_INVALID_UPGRADE_TIMES, "指定的强化次数无效!" },
    { BG_ERR_MAX_UPGRADE_TIMES, "不好意思, 一次最多连续强化20次哦!" },
    { BG_ERR_NO_PENDING_UPGRADE, "" },
    { BG_ERR_TRADEID_INVALID, "" },
    { BG_ERR_PASSWORD_REQUIRED, "该交易需要提供密码哦!" },
    { BG_ERR_PASSWORD_INCORRECT, "交易密码不正确!" },
    { BG_ERR_PASSWORD_NOT_REQUIRED, "该交易不需要你提供密码哦! 直接购买即可。" },
    { BG_ERR_INAPPROPRIATE_PRICE, "" },
    { BG_ERR_CANT_AFFORD_TAX, "" },
    { BG_ERR_PLAYER_MISMATCH, "看清楚哦! 这个是你上架的装备吗?" },
    { BG_ERR_INVALID_EQI_ID, "没有找到你要合成的这个 ID 哦! 你可以试下使用装备名称: \"bg 合成 装备名称 背包序号1 背包序号2 ...\"" },
    { BG_ERR_CANT_SYNTHESIS, "" },
    { BG_ERR_SYNTHESIS_NOT_EXIST, "没有这个合成哦! 你可以发送\"bg 合成 装备ID(或名称)\"来查看可用的合成方式。" },
    { BG_ERR_INVALID_DUNGEON, "这个副本不存在哦!" },
    { BG_ERR_NO_ENERGY, "你的体力已经耗尽啦!" },
    { BG_ERR_IN_CD, "" },
    { BG_ERR_MAX_ITEMS, "最多只能装备5个一次性物品哦!" },
    { BG_ERR_ITEM_REPEATED, "同样的一次性物品只能装备一个哦!" },
    { BG_ERR_KEYWORD_TOO_LONG, "你用膝盖想想啊! 怎么会有名字这么长的装备呢?" },
    { BG_ERR_EQI_NOT_FOUND, "没有找到这个装备哦! 换个名称试试?" }
};
#endif