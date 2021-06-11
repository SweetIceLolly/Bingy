/*
描述: 签到活动列表相关的操作
作者: 冰棍
文件: signin_event.cpp
*/

#include "signin_event.hpp"
#include "database.hpp"

std::vector<signInEvent>    allSignInEvents;

signInEvent::signInEvent() {
    signInCount = 0;
    signInCount_cache = false;
    prevActiveTime = 0;
    prevActiveTime_cache = false;
    id = 0;
    year = 0;
    month = 0;
    day = 0;
    hour = 0;
    minute = 0;
    coinFactor = 0;
    energyFactor = 0;
    firstN = 0;
    message = "";
}

signInEvent::signInEvent(const signInEvent &ev) {
    signInCount = ev.signInCount;
    signInCount_cache = ev.signInCount_cache;
    prevActiveTime = ev.prevActiveTime;
    prevActiveTime_cache = ev.prevActiveTime_cache;
    id = ev.id;
    year = ev.year;
    month = ev.month;
    day = ev.day;
    hour = ev.hour;
    minute = ev.minute;
    coinFactor = ev.coinFactor;
    energyFactor = ev.energyFactor;
    firstN = ev.firstN;
    items = ev.items;
    message = ev.message;
}

LL signInEvent::get_signInCount(bool use_cache) {
    if (signInCount_cache && use_cache)
        return this->signInCount;

    auto result = dbFindOne(DB_COLL_SIGNIN, "id", this->id, "signInCount");
    if (!result) {
        bsoncxx::document::value doc = bsoncxx::builder::stream::document{}
            << "id" << id
            << "prevActiveTime" << (LL)0
            << "signInCount" << (LL)0
            << bsoncxx::builder::stream::finalize;
        dbInsertDocument(DB_COLL_SIGNIN, doc);
        return 0;
    }
    auto field = result->view()["signInCount"];
    if (!field.raw()) {
        set_signInCount(0);
        return 0;
    }
    auto tmp = field.get_int64().value;

    std::lock_guard<std::mutex> lock(this->mutexSignInCount);
    this->signInCount = tmp;
    this->signInCount_cache = true;
    return tmp;
}

bool signInEvent::inc_signInCount(const LL &val) {
    std::lock_guard<std::mutex> lock(this->mutexSignInCount);
    if (dbUpdateOne(DB_COLL_SIGNIN, "id", this->id, "$inc",
        bsoncxx::builder::stream::document{} << "signInCount" << val
        << bsoncxx::builder::stream::finalize)) {

        this->signInCount += val;
        return true;
    }
    return false;
}

bool signInEvent::set_signInCount(const LL &val) {
    std::lock_guard<std::mutex> lock(this->mutexSignInCount);
    if (dbUpdateOne(DB_COLL_SIGNIN, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "signInCount" << val
        << bsoncxx::builder::stream::finalize)) {

        this->signInCount = val;
        this->signInCount_cache = true;
        return true;
    }
    return false;
}

LL signInEvent::get_prevActiveTime(bool use_cache) {
    if (prevActiveTime_cache && use_cache)
        return this->prevActiveTime;

    auto result = dbFindOne(DB_COLL_SIGNIN, "id", this->id, "prevActiveTime");
    if (!result) {
        bsoncxx::document::value doc = bsoncxx::builder::stream::document{}
            << "id" << id
            << "prevActiveTime" << (LL)0
            << "signInCount" << (LL)0
            << bsoncxx::builder::stream::finalize;
        dbInsertDocument(DB_COLL_SIGNIN, doc);
        return 0;
    }
    auto field = result->view()["prevActiveTime"];
    if (!field.raw()) {
        set_prevActiveTime(0);
        return 0;
    }
    auto tmp = field.get_int64().value;

    std::lock_guard<std::mutex> lock(this->mutexSignInCount);
    this->prevActiveTime = tmp;
    this->prevActiveTime_cache = true;
    return tmp;
}

bool signInEvent::set_prevActiveTime(const LL &val) {
    std::lock_guard<std::mutex> lock(this->mutexSignInCount);
    if (dbUpdateOne(DB_COLL_SIGNIN, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "prevActiveTime" << val
        << bsoncxx::builder::stream::finalize)) {

        this->prevActiveTime = val;
        this->prevActiveTime_cache = true;
        return true;
    }
    return false;
}

// 根据指定时间自动匹配对应的活动, 然后修改 coin 和 energy 对应的数值
void bg_match_sign_in_event(const dateTime &time, LL &coin, LL &energy, std::vector<LL> items, std::string &msg) {
    for (auto &item : allSignInEvents) {
        // 检查年份: 如果年份不为 -1, 则必须匹配年份
        if (item.year != -1 && item.year != time.get_year())
            continue;
        // 检查月份: 如果月份不为 -1, 则必须匹配月份
        if (item.month != -1 && item.month != time.get_month())
            continue;
        // 检查日子: 如果日子不为 -1, 则必须匹配日子
        if (item.day != -1 && item.day != time.get_day())
            continue;
        // 检查小时
        if (item.hour != -1 && item.hour != time.get_hour())
            continue;
        // 检查分钟
        if (item.minute != -1 && item.minute != time.get_minute())
            continue;
        // 检查签到人数
        if (item.firstN != -1) {
            auto prevActiveTime = dateTime(static_cast<time_t>(item.get_prevActiveTime()));
            bool resetCounter = false;
            bool year_changed = prevActiveTime.get_year() != time.get_year();

            // 判断是否需要重置签到计数器
            if (item.month != -1)
                resetCounter = year_changed;
            else if (item.day != -1)
                resetCounter = prevActiveTime.get_month() != time.get_month() || year_changed;

            if (resetCounter) {
                if (!item.set_prevActiveTime(time.get_timestamp()))
                    continue;
                if (!item.set_signInCount(0))
                    continue;
            }

            // 检查签到人数
            if (item.get_signInCount() < item.firstN) {
                if (!item.inc_signInCount(1))
                    continue;
            }
            else
                continue;
        }

        // 修改硬币, 体力, 物品, 消息
        coin = static_cast<LL>(coin * item.coinFactor);
        energy = static_cast<LL>(energy * item.coinFactor);
        for (const auto &eqi : item.items)
            items.push_back(eqi);

        msg += "\n" + item.message + (item.firstN == -1 ? "" :
            "(已领取" + std::to_string(item.get_signInCount()) + ", 共" + std::to_string(item.firstN) + "玩家能领取)");
    }
}
