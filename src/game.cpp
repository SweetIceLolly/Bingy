/*
描述: Bingy 游戏相关的函数. 整个游戏的主要逻辑
作者: 冰棍
文件: game.cpp
*/

#include <sstream>
#include <condition_variable>
#include "game.hpp"
#include "utils.hpp"
#include "HTTPRequest.hpp"
#include "json.hpp"
#include "error_codes.hpp"

// 默认请求超时
#define DEFAULT_TIMEOUT -1

using namespace nlohmann;

std::string serverUri = DEFAULT_SERVER_URI;
std::string appId;
std::string appSecret;

// 懒人宏
// 每个 Bingy 的 HTTP 请求都有如下内容, 使用这个宏来替代重复部分
#define MAKE_BG_JSON { "appid", appId }, { "secret", appSecret }, { "groupId", GROUP_ID }, { "qq", USER_ID }
#define MAKE_BG_QUERY { "appid", appId }, { "secret", appSecret }, { "groupId", std::to_string(GROUP_ID) }, { "qq", std::to_string(USER_ID) }

// Bingy 的 HTTP 请求回应
typedef struct _bg_http_response {
    int                     code;                       // 状态码
    json                    content;                    // 回应内容
} bg_http_response;

// 玩家强化状态
class player_confirm {
private:
    std::mutex              mutexStatus;                // 线程状态锁
    bool                    upgrading;                  // 玩家是否确认要强化
    std::condition_variable cvStatusChange;             // 状态发生变化
    std::condition_variable cvPrevConfirmCompleted;     // 上一次操作是否完成

public:
    player_confirm() {}
    player_confirm(const player_confirm &pc) {}

    // 完成多次强化 (确认或者取消)
    void completeUpgrade(const bool &confirm) {
        upgrading = confirm;
        cvStatusChange.notify_one();
    }

    // 等待强化确认. 如果玩家确认了强化, 就返回 true; 否则返回 false
    bool waitUpgradeConfirm() {
        upgrading = false;
        std::unique_lock lock(this->mutexStatus);
        cvStatusChange.wait_for(lock, std::chrono::seconds(20));    // 20 秒确认超时
        cvPrevConfirmCompleted.notify_one();
        return this->upgrading;
    }

    // 等待确认完成
    void waitConfirmComplete() {
        std::unique_lock lock(this->mutexStatus);
        cvPrevConfirmCompleted.wait(lock);

        // 不知为什么编译器优化会导致这里出 bug. 于是后面加一些垃圾防止编译器优化
        console_log("");
    }
};

// 玩家强化状态表. 如果玩家在这个表里, 那么玩家有正在进行的多次强化操作
std::unordered_map<LL, player_confirm> upgradeConfirmList;
std::mutex lockConfirmList;

// 强化状态表的相关操作
void confirmAdd(const LL &id) {
    std::scoped_lock _lock(lockConfirmList);
    upgradeConfirmList.insert({ id, player_confirm() });
}

void confirmRemove(const LL &id) {
    std::scoped_lock _lock(lockConfirmList);
    upgradeConfirmList.erase(id);
}

player_confirm& confirmGet(const LL &id) {
    std::scoped_lock _lock(lockConfirmList);
    return upgradeConfirmList[id];
}

bool confirmExists(const LL &id) {
    std::scoped_lock _lock(lockConfirmList);
    return upgradeConfirmList.find(id) != upgradeConfirmList.end();
}

// 进行 POST 请求. 注意: 需要调用方处理异常
bg_http_response bg_http_post(const std::string &path, const json &body, const int &timeout = DEFAULT_TIMEOUT) {
    http::Request request(serverUri + path);
    const auto response = request.send("POST", body.dump(),
        { "Content-Type: application/json" }, std::chrono::milliseconds(timeout));
    return bg_http_response{ response.status, json::parse(response.body) };
}

// 进行 GET 请求. 注意: 需要调用方处理异常
bg_http_response bg_http_get(const std::string &path, const std::vector<std::pair<std::string, std::string>> params, const int &timeout = DEFAULT_TIMEOUT) {
    std::string uri = serverUri + path;
    if (params.size() > 0) {
        uri += "?";
        for (const auto &[key, val] : params) {
            uri += key + "=" + val + "&";
        }
        uri.pop_back();
    }
    
    http::Request request(uri);
    const auto response = request.send("GET", "", {}, std::chrono::milliseconds(timeout));
    return bg_http_response{ response.status, json::parse(response.body) };
}

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

// 根据错误内容描述生成对应的回应
std::string bg_get_err_msg(const bg_http_response &res, const std::string strPrefix = "") {
    int errid = res.content["errid"].get<int>();
    auto desc_entry = error_desc.find(errid);

    if (desc_entry != error_desc.end()) {
        if (desc_entry->second == "")                       // 错误原文返回
            return res.content["msg"].get<std::string>();
        else                                                // 返回人话
            return desc_entry->second;
    }
    else                                                    // 返回带前缀的原文
        return strPrefix + res.content["msg"].get<std::string>();
}

// 新玩家注册
void registerCallback(const cq::MessageEvent& ev) {
    try {
        auto res = bg_http_post("/register", { MAKE_BG_JSON });
        if (res.code == 200) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "注册成功! 发送\"bg 签到\"获取体力, 然后发送\"bg 挑战1\"开始冒险之旅吧! 如果需要帮助, 可以发送\"bg 帮助\"哦!");
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "注册期间发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "注册期间发生错误: " + e.what());
    }
}

// 查看硬币
void viewCoinsCallback(const cq::MessageEvent& ev) {
    try {
        auto res = bg_http_get("/viewcoins", { MAKE_BG_QUERY });
        if (res.code == 200) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "硬币数: " + std::to_string(res.content["coins"].get<LL>()));
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "查看硬币发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看硬币发生错误: " + e.what());
    }
}

// 签到
void signInCallback(const cq::MessageEvent &ev) {
    try {
        auto res = bg_http_post("/signin", { MAKE_BG_JSON });
        if (res.code == 200) {
            std::string msg = "签到成功! 你连续签到了" + std::to_string(res.content["signInCountCont"].get<LL>()) + "天, 总共签到" +
                std::to_string(res.content["signInCount"].get<LL>()) + "天。你获得了: " +
                std::to_string(res.content["deltaCoins"].get<LL>()) + "硬币, " +
                std::to_string(res.content["deltaEnergy"].get<LL>()) + "体力, 和" +
                std::to_string(res.content["deltaExp"].get<LL>()) + "经验。目前共拥有" +
                std::to_string(res.content["coins"].get<LL>()) + "硬币和" +
                std::to_string(res.content["energy"].get<LL>()) + "体力。" +
                res.content["eventMsg"].get<std::string>();

            auto errors = res.content["errors"].get<std::vector<std::pair<std::string, int>>>();
            if (errors.size() > 0) {
                msg += "\n签到期间发生了以下错误:\n";
                for (const auto &err : errors) {
                    msg += err.first + " (" + std::to_string(err.second) + ")\n";
                }
            }
            cq::send_group_message(GROUP_ID, bg_at(ev) + msg);
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "签到期间发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "签到期间发生错误: " + e.what());
    }
}

// 查看背包
void viewInventoryCallback(const cq::MessageEvent &ev) {
    try {
        auto res = bg_http_get("/viewinv", { MAKE_BG_QUERY });
        if (res.code == 200) {
            auto items = res.content["items"].get<std::vector<std::string>>();
            std::string msg;

            if (!items.empty()) {
                LL index = 1;

                msg = "背包 (" + std::to_string(items.size()) + "/" + std::to_string(res.content["capacity"].get<LL>()) + ")\n";
                for (const auto &item : items) {
                    msg += std::to_string(index) + "." + item + " ";
                    ++index;
                }
                msg.pop_back();
            }
            else {
                msg = "背包空空如也, 快去获取装备吧!";
            }
            cq::send_group_message(GROUP_ID, bg_at(ev) + msg);
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "查看背包发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看背包发生错误: " + e.what());
    }
}

// 出售装备
void pawnCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args) {
    try {
        std::vector<LL> invList;
        for (const auto &item : args) {
            if (!item.empty())
                invList.push_back(str_to_ll(item) - 1);
        }
        auto res = bg_http_post("/pawn", { MAKE_BG_JSON, { "items", invList } });
        if (res.code == 200) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "成功出售" + std::to_string(res.content["count"].get<LL>()) +
                "个物品, 获得" + std::to_string(res.content["coins"].get<LL>()) + "硬币");
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "出售装备发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "出售装备发生错误: " + e.what());
    }
}

// 懒人宏
// 获取某个属性的字符串. 比如获取攻击 (atk), 则字符串为 攻击值 (武器攻击 + 护甲攻击 + 饰品攻击)
#define GET_EQI_PROP_STR(prop)                                                  \
    std::setprecision(1) <<                                                     \
    res.content[ #prop ]["total"].get<double>() <<                              \
    std::setprecision(0) << " (" <<                                             \
    res.content[ #prop ]["weapons"].get<double>() << "+" <<                     \
    res.content[ #prop ]["armors"].get<double>() << "+" <<                      \
    res.content[ #prop ]["ornaments"].get<double>() << ")"                   

// 查看属性
void viewPropertiesCallback(const cq::MessageEvent &ev) {
    try {
        auto res = bg_http_get("/viewprop", { MAKE_BG_QUERY });
        if (res.code == 200) {
            std::stringstream msg;
            msg << std::fixed << std::setprecision(1)
                << "账号: " << res.content["qq"].get<LL>() << "\n"
                << "等级: " << res.content["level"].get<LL>() << ", 祝福: " << res.content["blessing"].get<LL>() << "\n"
                << "经验: " << res.content["exp"].get<LL>() << "/" << res.content["expNeeded"].get<LL>()
                << " (" << static_cast<double>(res.content["exp"].get<LL>()) / static_cast<double>(res.content["expNeeded"].get<LL>()) * 100.0 << "%)" << "\n"
                << "生命: " << GET_EQI_PROP_STR(hp) << "\n"
                << "攻击: " << GET_EQI_PROP_STR(atk) << "\n"
                << "防护: " << GET_EQI_PROP_STR(def) << "\n"
                << "魔力: " << GET_EQI_PROP_STR(mp) << "\n"
                << "暴击: " << GET_EQI_PROP_STR(crt) << "\n"
                << "破甲: " << GET_EQI_PROP_STR(brk) << "\n"
                << "敏捷: " << GET_EQI_PROP_STR(agi) << "\n"
                << "体力: " << res.content["energy"].get<LL>() << " 硬币: " << res.content["coins"].get<LL>() << "\n"
                << "英雄币: " << res.content["heroCoin"].get<LL>();
            cq::send_group_message(GROUP_ID, bg_at(ev) + "\n" + msg.str());
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "查看背包发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看背包发生错误: " + e.what());
    }
}

// 查看装备
void viewEquipmentsCallback(const cq::MessageEvent &ev) {
    try {
        auto res = bg_http_get("/vieweqi", { MAKE_BG_QUERY });
        if (res.code == 200) {
            std::stringstream msg;
            msg << "---护甲---\n"
                "头盔: " << res.content["armor_helmet"].get<std::string>() << ", " <<
                "战甲: " << res.content["armor_body"].get<std::string>() << "\n" <<
                "护腿: " << res.content["armor_leg"].get<std::string>() << ", " <<
                "战靴: " << res.content["armor_boot"].get<std::string>() << "\n" <<
                "---武器---\n"
                "主武器: " << res.content["weapon_primary"].get<std::string>() << ", " <<
                "副武器: " << res.content["weapon_secondary"].get<std::string>() << "\n" <<
                "---饰品---\n"
                "耳环: " << res.content["ornament_earrings"].get<std::string>() << ", " <<
                "戒指: " << res.content["ornament_rings"].get<std::string>() << "\n" <<
                "项链: " << res.content["ornament_necklace"].get<std::string>() << ", " <<
                "宝石: " << res.content["ornament_jewelry"].get<std::string>();

            auto singleItems = res.content["single_use"].get<std::vector<std::string>>();
            if (singleItems.size() > 0) {
                msg << "\n---一次性---\n";
                LL index = 1;
                for (const auto &item : singleItems) {
                    if (index < singleItems.size())
                        msg << index << "." << item << " ";
                    else
                        msg << index << "." << item;
                    ++index;
                }
            }
            cq::send_group_message(GROUP_ID, bg_at(ev) + "\n" + msg.str());
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "查看装备发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看装备发生错误: " + e.what());
    }
}

// 装备
void equipCallback(const cq::MessageEvent &ev, const std::string &arg) {
    try {
        auto res = bg_http_post("/equip", { MAKE_BG_JSON, {"item", str_to_ll(arg) - 1 } });
        if (res.code == 200) {
            if (res.content.contains("wear")) {
                // 是普通装备
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功装备" + res.content["type"].get<std::string>() + ": " +
                    res.content["name"].get<std::string>() + "+" + std::to_string(res.content["level"].get<LL>()) + ", 磨损" +
                    std::to_string(res.content["wear"].get<LL>()) + "/" + std::to_string(res.content["defWear"].get<LL>()));
            }
            else {
                // 是一次性装备
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功装备一次性物品: " + res.content["name"].get<std::string>());
            }
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "装备发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "装备发生错误: " + e.what());
    }
}

// 卸下指定类型的普通装备
void unequipPlayer(const cq::MessageEvent &ev, const EqiType &type, const LL &index = -1) {
    try {
        bg_http_response res;
        if (type == EqiType::single_use)
            res = bg_http_post("/unequip", { MAKE_BG_JSON, { "type", type }, { "index", index - 1 } });
        else
            res = bg_http_post("/unequip", { MAKE_BG_JSON, { "type", type } });
        if (res.code == 200) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "已卸下" + res.content["item"].get<std::string>());
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "卸下装备发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下装备发生错误: " + e.what());
    }
}

// 卸下所有指定种类的装备
void unequipPlayer(const cq::MessageEvent &ev, const std::string &uriName, const std::string &typeName) {
    try {
        auto res = bg_http_post("/unequip" + uriName, { MAKE_BG_JSON });
        if (res.code == 200) {
            auto items = res.content["items"].get<std::vector<std::string>>();
            if (items.size() > 0) {
                std::string msg;
                for (const auto &item : items) {
                    if (!item.empty())
                        msg += item + ", ";
                }
                msg.pop_back();                                 // 去掉结尾的 ", "
                msg.pop_back();
                cq::send_group_message(GROUP_ID, bg_at(ev) + "已卸下" + msg);
            }
            else {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "目前没有装备" + typeName + "哦!");
            }
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "卸下所有" + typeName + "发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "卸下所有" + typeName + "发生错误: " + e.what());
    }
}

// 卸下头盔
void unequipHelmetCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::armor_helmet);
}

// 卸下战甲
void unequipBodyCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::armor_body);
}

// 卸下护腿
void unequipLegCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::armor_leg);
}

// 卸下战靴
void unequipBootCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::armor_boot);
}

// 卸下所有护甲
void unequipArmorCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, "armor", "护甲");
}

// 卸下主武器
void unequipPrimaryCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::weapon_primary);
}

// 卸下副武器
void unequipSecondaryCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::weapon_secondary);
}

// 卸下所有武器
void unequipWeaponCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, "weapon", "武器");
}

// 卸下耳环
void unequipEarringsCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::ornament_earrings);
}

// 卸下戒指
void unequipRingsCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::ornament_rings);
}

// 卸下项链
void unequipNecklaceCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::ornament_necklace);
}

// 卸下宝石
void unequipJewelryCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, EqiType::ornament_jewelry);
}

// 卸下所有饰品
void unequipOrnamentCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, "ornament", "饰品");
}

// 卸下一次性装备
void unequipSingleCallback(const cq::MessageEvent &ev, const std::string &arg) {
    try {
        unequipPlayer(ev, EqiType::single_use, str_to_ll(arg));
    }
    catch (...) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "无效的字符串!");
    }
}

// 卸下所有装备
void unequipAllCallback(const cq::MessageEvent &ev) {
    unequipPlayer(ev, "all", "装备");
}

// 强化装备
void upgradeCallback(const cq::MessageEvent &ev, const EqiType &eqiType, const std::string &arg) {
    try {
        LL times = str_to_ll(arg);
        auto res = bg_http_post("/upgrade", { MAKE_BG_JSON, { "type", eqiType }, { "times", times } });
        if (res.code == 200) {
            if (res.content.contains("coinsLeft")) {            // 单次强化
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功强化" + eqiType_to_str(eqiType) + std::to_string(res.content["times"].get<LL>()) +
                    "次: " + res.content["name"].get<std::string>() + ", 花费" + std::to_string(res.content["coins"].get<LL>()) + "硬币, 还剩" +
                    std::to_string(res.content["coinsLeft"].get<LL>()) + "硬币");
            }
            else {                                              // 多次强化
                cq::send_group_message(GROUP_ID, bg_at(ev) + "你将要连续强化" + eqiType_to_str(eqiType) + std::to_string(res.content["times"].get<LL>()) +
                    "次, 这会花费" + std::to_string(res.content["coins"].get<LL>()) + "硬币。发送\"bg 确认\"继续, 若20秒后没有确认, 则操作取消。");

                // 等待确认
                if (confirmExists(USER_ID)) {
                    // 如果玩家当前有进行中的确认, 那么取消掉正在进行的确认, 并等待那个确认退出
                    confirmGet(USER_ID).completeUpgrade(false);
                    confirmGet(USER_ID).waitConfirmComplete();
                    confirmRemove(USER_ID);
                }
                confirmAdd(USER_ID);
                if (upgradeConfirmList.at(USER_ID).waitUpgradeConfirm())
                    cq::send_group_message(GROUP_ID, bg_at(ev) + "成功强化" + eqiType_to_str(eqiType) + std::to_string(times) +
                        "次, 共花费" + std::to_string(res.content["coins"].get<LL>()) + "硬币");
                else
                    cq::send_group_message(GROUP_ID, bg_at(ev) + "你取消了连续强化" + eqiType_to_str(eqiType) + std::to_string(times) + "次");
                confirmRemove(USER_ID);
            }
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "强化装备发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "强化装备发生错误: " + e.what());
    }
}

// 确认强化
void confirmUpgradeCallback(const cq::MessageEvent &ev) {
    try {
        if (!confirmExists(USER_ID)) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + BG_ERR_STR_NO_PENDING_UPGRADE);
            return;
        }
        auto res = bg_http_post("/confirm", { MAKE_BG_JSON });
        if (res.code == 200) {
            confirmGet(USER_ID).completeUpgrade(true);
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "确认强化装备发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "确认强化装备发生错误: " + e.what());
    }
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
