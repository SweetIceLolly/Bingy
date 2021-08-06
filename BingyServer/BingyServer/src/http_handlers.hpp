/*
描述: Bingy 服务器的 HTTP 请求处理接口
作者: 冰棍
文件: http_handlers.hpp
*/

#pragma once

#include "rest_server/rest_server.hpp"

// 懒人宏
// 定义形如 bg_cmd_xxx 的 HTTP 请求回调函数
#define CMD(cmd) void bg_cmd_ ##cmd (mg_connection *connection, int ev, mg_http_message *ev_data, void *fn_data)

// HTTP 服务器的 poll 回调函数
void bg_server_poll(mg_connection *connection, int &ev, mg_http_message *ev_data, void *fn_data);

// 用来存储 HTTP 请求内容的结构体
typedef struct _http_req {
    struct mg_str   query;              // 请求的 query. 记得用完之后 free 对应的指针
    char            *body;              // 请求的 body. 记得用完之后 free

    unsigned        *signal;            // 信号. 把它设置为 1 代表响应完毕, 该回应随之会被发送给客户端
    char            **res_body;         // HTTP 返回的文本
    int             *res_http_code;     // HTTP 返回码
} http_req;

// 写入请求回应
inline void bg_http_reply(http_req *req, const int &http_code, const char *body) {
    *req->res_http_code = http_code;
    *(req->res_body) = strdup(body);
    *req->signal = 1;
}

// 发送请求错误回应
inline void bg_http_reply_error(http_req *req, const int &http_code, const std::string &msg, const int &errid) {
    bg_http_reply(req, http_code, ("{ \"msg\": \"" + msg + "\", \"errid\": " + std::to_string(errid) + "}").c_str());
}

/**
 * 新玩家注册
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 无其余内容
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(register);

/**
 * 查看硬币
 * 类型: GET
 * 参数位置: query
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回 coins
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(view_coins);

/**
 * 签到
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回:
 *      errors: 存有所有签到期间发生的错误的数组, 格式为 [[错误描述, 错误号], ...]
 *      signInCountCont: 连续签到天数
 *      signInCount: 签到次数
 *      deltaCoins: 获得硬币
 *      coins: 当前硬币
 *      deltaEnergy: 获得体力
 *      energy: 当前体力
 *      deltaExp: 获得经验
 *      eventMsg: 活动消息. 若没有活动则为空字符串
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(sign_in);

/**
 * 查看背包
 * 类型: GET
 * 参数位置: query
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回:
 *      items: 存有所有装备名称的数组. 若没有装备, 则为空数组
 *      capacity: 背包容量
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(view_inventory);

/**
 * 出售装备
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  items: [背包序号1, 背包序号2, ...] (从 0 开始)
 * 返回值:
 *  200: 成功, 返回:
 *      count: 出售的数量
 *      coins: 获得硬币
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(pawn);

/**
 * 查看属性
 * 类型: GET
 * 参数位置: query
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回包含属性值的 JSON
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(view_properties);

/**
 * 查看装备
 * 类型: GET
 * 参数位置: query
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回包含所有装备序号和等级的 JSON
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(view_equipments);

/**
 * 装备
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  item: 背包序号 (从 0 开始)
 * 返回值:
 *  200: 成功, 返回:
 *      type: 装备类型
 *      name: 装备名字
 *      level: (type != EqiType::single_use 时出现) 装备等级
 *      wear: (type != EqiType::single_use 时出现) 磨损
 *      defWear: (type != EqiType::single_use 时出现) 原始磨损
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(equip);

/**
 * 卸下装备
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  type: 装备类型
 *  index: (type == EqiType::single_use 时出现) 要卸下的一次性装备序号
 * 返回值:
 *  200: 成功, 返回:
 *      item: 卸下的装备名称
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(unequip);

/**
 * 卸下所有指定类型的装备
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回:
 *      items: 卸下的所有装备列表. 如果为空列表, 说明当前没有装备指定类型的装备
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(unequip_weapon);
CMD(unequip_armor);
CMD(unequip_ornament);
CMD(unequip_all);

/**
 * 强化装备
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  type: 装备类型
 *  times: 强化次数
 * 返回值:
 *  200: 成功, 返回:
 *      如果为多次强化: type: 装备类型; times: 强化次数; coins: 将花费硬币
 *      如果为单次强化: name: 装备名称; times: 强化次数; coins: 花费硬币; coinsLeft: 剩余硬币
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(upgrade);

/**
 * 确认强化
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 无其余内容
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(confirm_upgrade);

/**
 * 查看交易场
 * 类型: GET
 * 参数位置: query
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 * 返回值:
 *  200: 成功, 返回 items, 为交易场内容的数组, 其中包括:
 *      id: 交易商品 ID
 *      name: 物品名字
 *      wear, originalWear: (当物品为一次性用品时出现) 磨损, 原始磨损
 *      price: 价格
 *      private: 是否为私密交易
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(view_trade);

/**
 * 购买交易场商品
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  tradeId: 交易 ID
 *  password: 交易密码. 如果没密码则设置为空字符串
 * 返回值:
 *  200: 成功, 返回:
 *      如果为一次性物品: name: 装备名称; coins: 花费硬币
 *      否则: name: 装备名称; coins: 花费硬币; wear: 装备磨损; originalWear: 原始磨损
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(buy_trade);

/**
 * 上架交易场商品
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  invId: 背包序号
 *  price: 价格
 *  hasPassword: 是否为私密交易
 * 返回值:
 *  200: 成功, 返回:
 *      tradeId: 交易 ID
 *      tax: 税收
 *      password: 自动生成的密码
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(sell_trade);

/**
 * 下架交易场商品
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  tradeId: 交易 ID
 * 返回值:
 *  200: 成功, 返回:
 *      tradeId: 交易 ID
 *      name: 装备名称
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(recall_trade);

/**
 * 合成装备
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  invList: 存有背包序号的数组. 不可有重复序号
 *  target: 合成目标装备 ID 或者名称
 * 返回值:
 *  200: 成功, 如果指定了背包序号, 则返回:
 *      name: 装备名称
 *      coins: 花费硬币
 *      如果没有指定背包序号, 则返回:
 *      name: 装备名称
 *      methods: 能够合成的方式列表. 格式为: [{[材料1, 材料2, ...], 需要硬币}, ...]
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(synthesis);

/**
 * 挑战副本
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  id: 副本号
 * 返回值:
 *  200: 成功, 返回:
 *      fight: 回合详情
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
 */
CMD(fight);

/**
 * 管理员修改玩家属性
 * 类型: POST
 * 参数位置: body
 * 参数:
 *  appid: 应用 ID
 *  secret: 密匙
 *  groupId: 群号
 *  qq: QQ 号
 *  targetId: 目标玩家 (-1 (全部玩家) 或者 QQ 号)
 *  type: 属性类型. 0: coins; 1: heroCoin; 2: level; 3: blessing; 4: energy; 5: exp; 6: invCapacity; 7: vip
 *  mode: 修改模式. 0: inc; 1: set
 *  val: 修改的数值
 * 返回值:
 *  200: 成功, 返回:
 *      count: (当目标玩家为全体时出现) 玩家数量
 *      player: (当目标玩家为指定玩家时出现) 目标玩家
 *      val: 修改数值
 *  400: 失败, 详情见返回的 msg 和 errid
 *  500: 内部错误, 详情见返回的 msg 和 errid
*/
CMD(admin_modify_field);
