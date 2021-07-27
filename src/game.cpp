/*
描述: Bingy 游戏相关的函数. 整个游戏的主要逻辑
作者: 冰棍
文件: game.cpp
*/

#include "game.hpp"
#include "utils.hpp"
#include "HTTPRequest.hpp"
#include "json.hpp"
#include "error_codes.hpp"

std::string serverUri = DEFAULT_SERVER_URI;
std::string appId;
std::string appSecret;

// 懒人宏
// 每个 Bingy 的 HTTP 请求都有如下内容, 使用这个宏来替代重复部分
#define MAKE_BG_JSON { "appid", appId }, { "secret", appSecret }, { "groupId", GROUP_ID }, { "qq", USER_ID }

// Bingy 的 HTTP 请求回应
typedef struct _bg_http_response {
    int             code;                               // 状态码
    nlohmann::json  content;                            // 回应内容
} bg_http_response;

// 进行 POST 请求. 注意: 需要调用方处理异常
bg_http_response bg_http_post(const std::string &path, const nlohmann::json &body, const int &timeout = 50000) {
    http::Request request(serverUri + path);
    const auto response = request.send("POST", body.dump(),
        { "Content-Type: application/json" }, std::chrono::milliseconds(timeout));
    return bg_http_response{ response.status, nlohmann::json::parse(response.body.begin(), response.body.end()) };
}

// 进行 GET 请求. 注意: 需要调用方处理异常
//template <typename ... Args>
//bg_http_response bg_http_get(const std::string &path, const int &timeout = 50000, const Args&&... params) {
//    
//}

// 取得艾特玩家字符串
std::string bg_at(const cq::MessageEvent &ev) {
    try {
        auto gmi = cq::get_group_member_info(GROUP_ID, USER_ID);
        return "[CQ:at,qq=" + std::to_string(gmi.user_id) + "] ";
    }
    catch (...) {
        return "@" + std::to_string(USER_ID) + " ";
    }
}

// 新玩家注册
void registerCallback(const cq::MessageEvent& ev) {
    try {
        auto res = bg_http_post("/register", { MAKE_BG_JSON });
        if (res.code == 200) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "注册成功! 发送\"bg 签到\"获取体力, 然后发送\"bg 挑战1\"开始副本之旅吧! 如果需要帮助, 可以发送\"bg 帮助\"哦!");
        }
        else {
            if (res.content["errid"].get<int>() == BG_ERR_ALREADY_REGISTERED)
                cq::send_group_message(GROUP_ID, bg_at(ev) + "你已经注册过啦!");
            else
                cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("注册期间发生错误: ") + res.content["msg"].get<std::string>());
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + std::string("注册期间发生错误: ") + e.what());
    }
}

// 查看硬币
void viewCoinsCallback(const cq::MessageEvent& ev) {
    
}

// 签到
void signInCallback(const cq::MessageEvent &ev) {

}

// 查看背包
void viewInventoryCallback(const cq::MessageEvent &ev) {

}

// 出售装备
void pawnCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args) {

}

// 查看属性
void viewPropertiesCallback(const cq::MessageEvent &ev) {

}

// 查看装备
void viewEquipmentsCallback(const cq::MessageEvent &ev) {

}

// 装备
void equipCallback(const cq::MessageEvent &ev, const std::string &arg) {

}

// 卸下头盔
void unequipHelmetCallback(const cq::MessageEvent &ev) {

}

// 卸下护甲
void unequipBodyCallback(const cq::MessageEvent &ev) {

}

// 卸下护腿
void unequipLegCallback(const cq::MessageEvent &ev) {

}

// 卸下靴子
void unequipBootCallback(const cq::MessageEvent &ev) {

}

// 卸下所有护甲
void unequipArmorCallback(const cq::MessageEvent &ev) {

}

// 卸下主武器
void unequipPrimaryCallback(const cq::MessageEvent &ev) {

}

// 卸下副武器
void unequipSecondaryCallback(const cq::MessageEvent &ev) {

}

// 卸下所有武器
void unequipWeaponCallback(const cq::MessageEvent &ev) {

}

// 卸下耳环
void unequipEarringsCallback(const cq::MessageEvent &ev) {

}

// 卸下戒指
void unequipRingsCallback(const cq::MessageEvent &ev) {

}

// 卸下项链
void unequipNecklaceCallback(const cq::MessageEvent &ev) {

}

// 卸下珠宝
void unequipJewelryCallback(const cq::MessageEvent &ev) {

}

// 卸下所有饰品
void unequipOrnamentCallback(const cq::MessageEvent &ev) {

}

// 卸下一次性装备
void unequipSingleCallback(const cq::MessageEvent &ev, const std::string &arg) {

}

// 卸下所有装备
void unequipAllCallback(const cq::MessageEvent &ev) {

}

// 强化装备
void upgradeCallback(const cq::MessageEvent &ev, const EqiType &eqiType, const std::string &arg) {

}

// 确认强化
void confirmUpgradeCallback(const cq::MessageEvent &ev) {

}

// 查看交易场
void viewTradeCallback(const cq::MessageEvent &ev) {

}

// 购买交易场商品
void buyTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args) {

}

// 上架交易场商品
void sellTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args) {

}

// 下架交易场商品
void recallTradeCallback(const cq::MessageEvent &ev, const std::string &arg) {

}

// 合成装备
void synthesisCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args) {

}

// 挑战副本
void fightCallback(const cq::MessageEvent &ev, const std::string &arg) {

}

// 管理命令: 添加硬币
ADMIN(AddCoins) {

}

// 管理命令: 添加英雄币
ADMIN(AddHeroCoin) {

}

// 管理命令: 添加玩家等级
ADMIN(AddLevel) {

}

// 管理命令: 添加玩家祝福
ADMIN(AddBlessing) {

}

// 管理命令: 添加体力
ADMIN(AddEnergy) {

}

// 管理命令: 添加经验
ADMIN(AddExp) {

}

// 管理命令: 添加硬币
ADMIN(AddInvCapacity) {

}

// 管理命令: 添加 VIP 等级
ADMIN(AddVip) {

}
