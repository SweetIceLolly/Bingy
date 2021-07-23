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
// B-1. 通用账户检查错误
// B-1.a. 400 错误
#define BG_ERR_STR_UNREGISTERED                     "玩家未注册"
#define BG_ERR_UNREGISTERED                         2000

#define BG_ERR_STR_BLACKLISTED                      "玩家在黑名单中"
#define BG_ERR_BLACKLISTED                          2001

// B-2. 新玩家注册错误
// B-2-a. 400 错误
#define BG_ERR_STR_ALREADY_REGISTERED               "玩家重复注册"
#define BG_ERR_ALREADY_REGISTERED                   2100

// B-2-b. 500 错误
#define BG_ERR_STR_ADDING_NEW_PLAYER_FAILED         "添加新玩家失败"
#define BG_ERR_ADDING_NEW_PLAYER_FAILED             2150

// B-3. 查看硬币
// B-3-a. 500 错误
#define BG_ERR_STR_GET_COINS_FAILED                 "读取玩家硬币数失败"
#define BG_ERR_GET_COINS_FAILED                     2250

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

#define BG_ERR_STR_SIGN_IN_ADD_ITEM_FAILED          "为玩家添加活动物品失败"
#define BG_ERR_SIGN_IN_ADD_ITEM_FAILED              2353

#define BG_ERR_STR_SIGN_IN_INC_COINS_FAILED         "为玩家添加硬币失败"
#define BG_ERR_SIGN_IN_INC_COINS_FAILED             2354

#define BG_ERR_STR_SIGN_IN_INC_ENERGY_FAILED        "为玩家添加体力失败"
#define BG_ERR_SIGN_IN_INC_ENERGY_FAILED            2355

#define BG_ERR_STR_SIGN_IN_INC_EXP_FAILED           "为玩家添加经验失败"
#define BG_ERR_SIGN_IN_INC_EXP_FAILED               2356

#define BG_ERR_STR_SIGN_IN_FAILED                   "签到发生错误"
#define BG_ERR_SIGN_IN_FAILED                       2357

// B-5. 查看背包错误
// B-5-a. 500 错误
#define BG_ERR_STR_VIEW_INV_FAILED                  "查看背包失败"
#define BG_ERR_VIEW_INV_FAILED                      2450

// B-6. 出售错误
// B-6.a. 400 错误
#define BG_ERR_STR_ID_OUT_OF_RANGE                  "序号超出了背包范围"
#define BG_ERR_ID_OUT_OF_RANGE                      2500

#define BG_ERR_STR_ID_REPEATED                      "序号重复了"
#define BG_ERR_ID_REPEATED                          2501

#define BG_ERR_STR_PRE_PAWN_FAILED                  "出售前检查发生错误"
#define BG_ERR_PRE_PAWN_FAILED                      2502

// B-6.b. 500 错误
#define BG_ERR_STR_REMOVE_ITEM_FAILED               "从玩家背包移除物品失败"
#define BG_ERR_REMOVE_ITEM_FAILED                   2550

#define BG_ERR_STR_PAWN_FAILED                      "出售失败, 未知的内部错误"
#define BG_ERR_PAWN_FAILED                          2551

// B.7. 查看属性错误
// B.7.a. 500 错误
#define BG_ERR_STR_VIEW_PROP_FAILED                 "查看属性失败"
#define BG_ERR_VIEW_PROP_FAILED                     2650

// B.8. 查看装备错误
// B.8.a. 500 错误
#define BG_ERR_STR_VIEW_EQI_FAILED                  "查看装备失败"
#define BG_ERR_VIEW_EQI_FAILED                      2750

// B.9. 装备错误
// B.9.a. 400 错误
#define BG_ERR_STR_PRE_EQUIP_FAILED                 "装备前检查发生错误"
#define BG_ERR_PRE_EQUIP_FAILED                     2800

// B.9.b. 500 错误
#define BG_ERR_STR_EQUIP_REMOVE_FAILED              "把将要装备的装备从背包中移除时发生错误"
#define BG_ERR_EQUIP_REMOVE_FAILED                  2850

#define BG_ERR_STR_EQUIP_UPDATE_FAILED              "修改玩家装备时发生错误"
#define BG_ERR_EQUIP_UPDATE_FAILED                  2851

#define BG_ERR_STR_EQUIP_ADD_FAILED                 "把之前的装备放回背包时发生错误"
#define BG_ERR_EQUIP_ADD_FAILED                     2852

#define BG_ERR_STR_EQUIP_FAILED                     "装备时发生错误"
#define BG_ERR_EQUIP_FAILED                         2854

// B.10. 卸下装备错误
// B.10.a. 400 错误
#define BG_ERR_STR_NOT_EQUIPPED                     "当前未装备对应类型的装备"
#define BG_ERR_NOT_EQUIPPED                         2900

#define BG_ERR_STR_SINGLE_OUT_OF_RANGE              "指定了无效的一次性物品序号"
#define BG_ERR_SINGLE_OUT_OF_RANGE                  2901

#define BG_ERR_STR_PRE_UNEQUIP_FAILED               "卸下装备前检查发生错误"
#define BG_ERR_PRE_UNEQUIP_FAILED                   2902

// B.10.b. 500 错误
#define BG_ERR_STR_UNEQUIP_FAILED                   "卸下装备失败"
#define BG_ERR_UNEQUIP_FAILED                       2950

#define BG_ERR_STR_CLEAR_SINGLE_FAILED              "清空一次性装备时发生错误"
#define BG_ERR_CLEAR_SINGLE_FAILED                  2951

#define BG_ERR_STR_SINGLE_ADD_FAILED                "把一次性装备添加到背包时发生错误"
#define BG_ERR_SINGLE_ADD_FAILED                    2952

#define BG_ERR_STR_REMOVE_SINGLE_FAILED             "移除一次性装备时发生错误"
#define BG_ERR_REMOVE_SINGLE_FAILED                 2953

// B.11. 装备强化错误
// B.11.a. 400 错误
#define BG_ERR_STR_INVALID_UPGRADE_TIMES            "无效的升级次数"
#define BG_ERR_INVALID_UPGRADE_TIMES                3000

#define BG_ERR_STR_PRE_UPGRADE_FAILED               "强化前检查发生错误"
#define BG_ERR_PRE_UPGRADE_FAILED                   3001

#define BG_ERR_STR_MAX_UPGRADE_TIMES                "超出了升级次数限制"
#define BG_ERR_MAX_UPGRADE_TIMES                    3002

#define BG_ERR_STR_INSUFFICIENT_COINS               "硬币不足"
#define BG_ERR_INSUFFICIENT_COINS                   3003

#define BG_ERR_STR_UPGRADE_CANCELED                 "连续升级被取消"
#define BG_ERR_UPGRADE_CANCELED                     3004

#define BG_ERR_STR_NO_PENDING_UPGRADE               "目前没有需要确认的连续强化操作"
#define BG_ERR_NO_PENDING_UPGRADE                   3005

// B.11.b. 500 错误
#define BG_ERR_STR_DEC_COINS_FAILED                 "扣除玩家硬币时发生错误"
#define BG_ERR_DEC_COINS_FAILED                     3050

#define BG_ERR_STR_SET_EQI_FAILED                   "设置玩家装备时发生错误"
#define BG_ERR_SET_EQI_FAILED                       3051

#define BG_ERR_STR_UPGRADE_FAILED                   "强化期间发生错误"
#define BG_ERR_UPGRADE_FAILED                       3052
