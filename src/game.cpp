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
    void completeUpgrade(bool confirm) {
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
bg_http_response bg_http_get(const std::string &path, const std::vector<std::pair<std::string, std::string>> &params, const int &timeout = DEFAULT_TIMEOUT) {
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
std::string bg_get_err_msg(const bg_http_response &res, const std::string &strPrefix = "") {
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
            auto items = res.content["items"].get<std::vector<std::pair<std::string, unsigned char>>>();
            std::string msg;

            if (!items.empty()) {
                LL index = 1;

                msg = "背包 (" + std::to_string(items.size()) + "/" + std::to_string(res.content["capacity"].get<LL>()) + ")\n";
                for (const auto &item : items) {
                    msg += std::to_string(index) + ".";

                    switch (static_cast<EqiType>(item.second)) {
                    case EqiType::armor_helmet:
                        msg += "(盔)"; break;
                    case EqiType::armor_body:
                        msg += "(甲)"; break;
                    case EqiType::armor_leg:
                        msg += "(腿)"; break;
                    case EqiType::armor_boot:
                        msg += "(靴)"; break;
                    case EqiType::weapon_primary:
                        msg += "(主)"; break;
                    case EqiType::weapon_secondary:
                        msg += "(副)"; break;
                    case EqiType::ornament_earrings:
                        msg += "(环)"; break;
                    case EqiType::ornament_rings:
                        msg += "(戒)"; break;
                    case EqiType::ornament_necklace:
                        msg += "(链)"; break;
                    case EqiType::ornament_jewelry:
                        msg += "(宝)"; break;
                    }

                    msg += item.first + " ";
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
            if (!item.empty()) {
                if (item.find('-') != item.npos) {
                    // 处理范围
                    auto range = str_split(item, '-');
                    if (range.size() != 2) {
                        cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 出售指令格式为: \"bg 出售 背包序号1 背包序号2 ...\"");
                        return;
                    }
                    auto start = str_to_ll(range[0]), end = str_to_ll(range[1]);
                    if (end < start || start <= 0 || end < 0 || end - start > 100) {
                        cq::send_group_message(GROUP_ID, bg_at(ev) + "出售范围有误!");
                        return;
                    }
                    for (LL i = start; i <= end; ++i)
                        invList.push_back(i - 1);
                }
                else {
                    // 处理单个数值
                    invList.push_back(str_to_ll(item) - 1);
                }
            }
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
    res.content[ #prop ]["weapons"].get<double>() << ", " <<                    \
    res.content[ #prop ]["armors"].get<double>() << ", " <<                     \
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
            if (res.content.find("wear") != res.content.end()) {
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
            if (res.content.find("coinsLeft") != res.content.end()) {           // 单次强化
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功强化" + eqiType_to_str(eqiType) + std::to_string(res.content["times"].get<LL>()) +
                    "次: " + res.content["name"].get<std::string>() + ", 花费" + std::to_string(res.content["coins"].get<LL>()) + "硬币, 还剩" +
                    std::to_string(res.content["coinsLeft"].get<LL>()) + "硬币");
            }
            else {                                                              // 多次强化
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
    try {
        auto res = bg_http_get("/viewtrade", { MAKE_BG_QUERY });
        if (res.code == 200) {
            if (res.content.size() > 0) {
                std::string msg = "---交易物品 (共" + std::to_string(res.content.size()) + "个)---\n";
                for (const auto &item : res.content) {
                    msg += "ID" + std::to_string(item["id"].get<LL>()) + ": " + item["name"].get<std::string>();
                    if (item.find("wear") != item.end())                // 是普通装备则加上磨损
                        msg += ", " + std::to_string(item["wear"].get<LL>()) + "/" + std::to_string(item["originalWear"].get<LL>());
                    msg += " $" + std::to_string(item["price"].get<LL>());
                    if (item["private"].get<bool>())                    // 是私密交易则加上标记
                        msg += " (私)";
                    msg += "\n";
                }
                msg.pop_back();                                         // 去掉多余的换行符
                cq::send_group_message(GROUP_ID, bg_at(ev) + "\n" + msg);
            }
            else {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "目前交易场中没有东西哦!");
            }
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "查看交易场发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "查看交易场发生错误: " + e.what());
    }
}

// 购买交易场商品
void buyTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args) {
    try {
        auto res = bg_http_post("/buytrade", {
            MAKE_BG_JSON,
            { "tradeId", str_to_ll(args[0]) },
            { "password", args.size() == 2 ? args[1] : "" }
        });
        if (res.code == 200) {
            if (res.content.find("wear") != res.content.end()) {        // 为普通装备
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功购买装备: " + res.content["name"].get<std::string>() +
                    ", 磨损度" + std::to_string(res.content["wear"].get<LL>()) + "/" + std::to_string(res.content["originalWear"].get<LL>()) +
                    ", 花费" + std::to_string(res.content["coins"].get<LL>()) + "硬币");
            }
            else {                                                      // 为一次性装备
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功购买一次性物品: " + res.content["name"].get<std::string>() +
                    ", 花费" + std::to_string(res.content["coins"].get<LL>()) + "硬币");
            }
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "购买发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "购买发生错误: " + e.what());
    }
}

// 上架交易场商品
void sellTradeCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args) {
    try {
        bool hasPassword = false;
        if (args.size() == 3) {
            if (args[2] == "私")
                hasPassword = true;
            else {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式不对哦! 上架指令格式为: \"bg 上架 背包序号 价格\"。"
                    "若要指定为有密码的交易, 则在命令最后加个空格和\"私\"字: \"bg 上架 背包序号 私\"");
                return;
            }
        }
        auto res = bg_http_post("/selltrade", {
            MAKE_BG_JSON,
            { "invId", str_to_ll(args[0]) - 1 },
            { "price", str_to_ll(args[1]) },
            { "hasPassword", hasPassword }
        });
        if (res.code == 200) {
            if (hasPassword) {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功上架, 交易ID为" + std::to_string(res.content["tradeId"].get<LL>()) +
                    ", 收取税款" + std::to_string(res.content["tax"].get<LL>()) + "硬币。交易密码已经通过私聊发给你啦!");
                cq::send_private_message(USER_ID, "您的ID为" + std::to_string(res.content["tradeId"].get<LL>()) +
                    "的交易的购买密码为: " + res.content["password"].get<std::string>());
            }
            else
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功上架, 交易ID为" + std::to_string(res.content["tradeId"].get<LL>()) +
                    ", 收取税款" + std::to_string(res.content["tax"].get<LL>()) + "硬币");
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "上架发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "上架发生错误: " + e.what());
    }
}

// 下架交易场商品
void recallTradeCallback(const cq::MessageEvent &ev, const std::string &arg) {
    try {
        auto res = bg_http_post("/recalltrade", { MAKE_BG_JSON, { "tradeId", str_to_ll(arg) } });
        if (res.code == 200) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "成功下架交易" + std::to_string(res.content["tradeId"].get<LL>()) +
                ": 已把\"" + res.content["name"].get<std::string>() + "\"放回背包, 但税款不予退还哦!");
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "下架发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "下架发生错误: " + e.what());
    }
}

// 合成装备
void synthesisCallback(const cq::MessageEvent &ev, const std::vector<std::string> &args) {
    try {
        std::set<LL, std::greater<LL>> invList;
        for (auto it = args.begin() + 1; it != args.end(); ++it) {
            LL index = str_to_ll(*it) - 1;
            if (invList.find(index) == invList.end())
                invList.insert(index);
            else {
                cq::send_group_message(GROUP_ID, bg_at(ev) + "指定的背包序号重复了: " + std::to_string(index + 1));
                return;
            }
        }

        auto res = bg_http_post("/synthesis", { MAKE_BG_JSON, { "invList", invList }, { "target", args[0] } });
        if (res.code == 200) {
            if (res.content.find("methods") != res.content.end()) {
                // 列出可用的合成
                auto methods = res.content["methods"].get<std::vector<std::pair<std::vector<std::string>, LL>>>();
                std::string msg = "装备\"" + res.content["name"].get<std::string>() + "\"的合成方式:\n";
                for (const auto &method : methods) {
                    for (const auto &material : method.first) {
                        msg += material + "+";
                    }
                    msg += "$" + std::to_string(method.second);
                }
                cq::send_group_message(GROUP_ID, msg);
            }
            else {
                // 合成成功
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功合成" +
                    res.content["name"].get<std::string>() + ", 花费" + std::to_string(res.content["coins"].get<LL>()) + "硬币");
            }
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "合成装备发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "合成装备发生错误: " + e.what());
    }
}

// 挑战副本
void fightCallback(const cq::MessageEvent &ev, const std::string &arg) {
    try {
        auto res = bg_http_post("/fight", { MAKE_BG_JSON, { "level", arg } });
        if (res.code == 200) {
            bool playerFirst = res.content["fighterFirst"].get<bool>();                                 // 是否玩家先手
            bool playerRound = playerFirst;
            auto rounds = res.content["rounds"].get<std::vector<std::tuple<LL, LL, std::string>>>();    // 回合详情
            auto enterMsg = res.content["msg"].get<std::string>();                                      // 怪物出场消息
            std::string fightText;
            LL round = 0;

            // 怪物出场消息
            if (enterMsg != "")
                fightText = enterMsg + "\n";

            for (const auto &[dmg, hp, msg] : rounds) {
                // 在先手的回合显示回合数
                if (playerFirst == playerRound) {
                    ++round;
                    fightText += "R" + std::to_string(round) + "　";
                }

                // 显示回合详情
                if (playerRound)
                    fightText += "玩家伤害：";
                else
                    fightText += "敌方伤害：";
                fightText += std::to_string(dmg) + "，余" + std::to_string(hp);
                if (!msg.empty())
                    fightText += "（" + msg + "）";

                // 在后手一方攻击完后添加换行符
                // 玩家先手 => 怪物回合后添加换行符; 怪物先手 => 玩家回合后添加换行符; 其它情况: 添加空格
                if (playerFirst != playerRound)
                    fightText += "\n";
                else
                    fightText += "　";

                // 切换回合
                playerRound = !playerRound;
            }

            // 对战结束后的附加消息
            if (fightText.back() != '\n')
                fightText += "\n";
            auto postMsg = res.content["postMsg"].get<std::string>();
            if (!postMsg.empty())
                fightText += postMsg + "\n";

            // 战斗结果
            fightText += bg_at(ev);
            if (res.content["win"].get<bool>()) {
                // 玩家获胜, 显示奖励信息
                fightText += "你击败了" + res.content["targetName"].get<std::string>() + "并获得了" +
                    std::to_string(res.content["coins"].get<LL>()) + "硬币和" + std::to_string(res.content["exp"].get<LL>()) + "经验, 体力剩余" +
                    std::to_string(res.content["energy"].get<LL>()) + "。";
                
                auto drops = res.content["drops"].get<std::vector<std::pair<std::string, unsigned char>>>();
                if (!drops.empty()) {
                    fightText += "\n你获得了: ";
                    for (const auto &item : drops) {
                        fightText += "(" + eqiType_to_str(static_cast<EqiType>(item.second)) + ") " + item.first + ", ";
                    }
                    fightText.pop_back();
                    fightText.pop_back();
                }
            }
            else {
                // 玩家战败, 显示战败信息
                fightText += "你被" + res.content["targetName"].get<std::string>() + "击败了! 你失去了" +
                    std::to_string(res.content["coins"].get<LL>()) + "硬币。体力剩余" +
                    std::to_string(res.content["energy"].get<LL>()) + "。";
            }

            // 内部错误
            auto errors = res.content["errors"].get<std::vector<std::pair<std::string, int>>>();
            if (!errors.empty()) {
                fightText += "\n\n发生了以下错误:\n";
                for (const auto &[msg, id] : errors) {
                    fightText += msg + " (" + std::to_string(id) + ")\n";
                }
            }

            cq::send_group_message(GROUP_ID, fightText);
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "挑战副本发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "挑战副本发生错误: " + e.what());
    }
}

// =====================================================================================================
// 管理指令
// =====================================================================================================

// type: 属性类型. 0: coins; 1: heroCoin; 2: level; 3: blessing; 4: energy; 5: exp; 6: invCapacity; 7: vip
// mode: 修改模式. 0 : inc; 1: set
void adminModifyFieldCallback(const cq::MessageEvent &ev, unsigned char type, unsigned char mode, const std::string &arg) {
    static const std::string typeName[] = { "coins", "herocoin", "level", "blessing", "energy", "exp", "invcapacity", "vip" };
    static const std::string typeStr[] = { "硬币", "英雄币", "等级", "祝福", "体力", "经验", "背包容量", "VIP等级" };
    static const std::string modeStr[] = { "添加", "设置为" };

    try {
        auto params = str_split(str_trim(arg), ' ');
        if (params.size() != 2) {
            cq::send_group_message(GROUP_ID, bg_at(ev) + "命令格式错误: bg /add(或set)" + typeName[type] + " qq/all 数值");
            return;
        }

        LL targetId;
        try {
            targetId = qq_parse(params[0]);
        }
        catch (...) {
            if (params[0] == "all")
                targetId = -1;
            else {
                throw std::runtime_error("指定的 QQ 无效");
            }
        }

        auto res = bg_http_post("/adminmodifyfield", {
            MAKE_BG_JSON,
            { "targetId", targetId },
            { "type", type },
            { "mode", mode },
            { "val", str_to_ll(params[1]) }
        });
        if (res.code == 200) {
            if (res.content.find("count") != res.content.end())
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功为" + std::to_string(res.content["count"].get<LL>()) + "个玩家的" +
                    typeStr[type] + modeStr[mode] + std::to_string(res.content["val"].get<LL>()));
            else
                cq::send_group_message(GROUP_ID, bg_at(ev) + "成功把玩家" + std::to_string(res.content["player"].get<LL>()) + "的" +
                    typeStr[type] + modeStr[mode] + std::to_string(res.content["val"].get<LL>()));
        }
        else {
            cq::send_group_message(GROUP_ID, bg_at(ev) + bg_get_err_msg(res, "修改玩家" + typeName[type] + "属性时发生错误: "));
        }
    }
    catch (const std::exception &e) {
        cq::send_group_message(GROUP_ID, bg_at(ev) + "修改玩家" + typeName[type] + "属性时发生错误: " + e.what());
    }
}

// 管理命令: 添加/设置硬币
ADMIN(AddCoins) {
    adminModifyFieldCallback(ev, 0, 0, arg);
}

ADMIN(SetCoins) {
    adminModifyFieldCallback(ev, 0, 1, arg);
}

// 管理命令: 添加/设置英雄币
ADMIN(AddHeroCoin) {
    adminModifyFieldCallback(ev, 1, 0, arg);
}

ADMIN(SetHeroCoin) {
    adminModifyFieldCallback(ev, 1, 1, arg);
}

// 管理命令: 添加/设置玩家等级
ADMIN(AddLevel) {
    adminModifyFieldCallback(ev, 2, 0, arg);
}

ADMIN(SetLevel) {
    adminModifyFieldCallback(ev, 2, 1, arg);
}

// 管理命令: 添加/设置玩家祝福
ADMIN(AddBlessing) {
    adminModifyFieldCallback(ev, 3, 0, arg);
}

ADMIN(SetBlessing) {
    adminModifyFieldCallback(ev, 3, 1, arg);
}

// 管理命令: 添加/设置体力
ADMIN(AddEnergy) {
    adminModifyFieldCallback(ev, 4, 0, arg);
}

ADMIN(SetEnergy) {
    adminModifyFieldCallback(ev, 4, 1, arg);
}

// 管理命令: 添加/设置经验
ADMIN(AddExp) {
    adminModifyFieldCallback(ev, 5, 0, arg);
}

ADMIN(SetExp) {
    adminModifyFieldCallback(ev, 5, 1, arg);
}

// 管理命令: 添加/设置背包容量
ADMIN(AddInvCapacity) {
    adminModifyFieldCallback(ev, 6, 0, arg);
}

ADMIN(SetInvCapacity) {
    adminModifyFieldCallback(ev, 6, 1, arg);
}

// 管理命令: 添加/设置 VIP 等级
ADMIN(AddVip) {
    adminModifyFieldCallback(ev, 7, 0, arg);
}

ADMIN(SetVip) {
    adminModifyFieldCallback(ev, 7, 1, arg);
}
