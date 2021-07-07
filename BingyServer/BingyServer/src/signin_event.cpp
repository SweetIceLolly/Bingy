/*
����: ǩ����б���صĲ���
����: ����
�ļ�: signin_event.cpp
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

    std::scoped_lock<std::mutex> lock(this->mutexSignInCount);
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
    this->signInCount = tmp;
    this->signInCount_cache = true;
    return tmp;
}

bool signInEvent::inc_signInCount(const LL &val) {
    std::scoped_lock<std::mutex> lock(this->mutexSignInCount);
    if (dbUpdateOne(DB_COLL_SIGNIN, "id", this->id, "$inc",
        bsoncxx::builder::stream::document{} << "signInCount" << val
        << bsoncxx::builder::stream::finalize)) {

        this->signInCount += val;
        return true;
    }
    return false;
}

bool signInEvent::set_signInCount(const LL &val) {
    std::scoped_lock<std::mutex> lock(this->mutexSignInCount);
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

    std::scoped_lock<std::mutex> lock(this->mutexSignInCount);
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
    this->prevActiveTime = tmp;
    this->prevActiveTime_cache = true;
    return tmp;
}

bool signInEvent::set_prevActiveTime(const LL &val) {
    std::scoped_lock<std::mutex> lock(this->mutexSignInCount);
    if (dbUpdateOne(DB_COLL_SIGNIN, "id", this->id, "$set",
        bsoncxx::builder::stream::document{} << "prevActiveTime" << val
        << bsoncxx::builder::stream::finalize)) {

        this->prevActiveTime = val;
        this->prevActiveTime_cache = true;
        return true;
    }
    return false;
}

// ����ָ��ʱ���Զ�ƥ���Ӧ�Ļ, Ȼ���޸� coin �� energy ��Ӧ����ֵ
void bg_match_sign_in_event(const dateTime &time, LL &coin, LL &energy, std::vector<LL> &items, std::string &msg) {
    for (auto &item : allSignInEvents) {
        // ������: �����ݲ�Ϊ -1, �����ƥ�����
        if (item.year != -1 && item.year != time.get_year())
            continue;
        // ����·�: ����·ݲ�Ϊ -1, �����ƥ���·�
        if (item.month != -1 && item.month != time.get_month())
            continue;
        // �������: ������Ӳ�Ϊ -1, �����ƥ������
        if (item.day != -1 && item.day != time.get_day())
            continue;
        // ���Сʱ
        if (item.hour != -1 && item.hour != time.get_hour())
            continue;
        // ������
        if (item.minute != -1 && item.minute != time.get_minute())
            continue;
        // ���ǩ������
        if (item.firstN != -1) {
            auto prevActiveTime = dateTime(static_cast<time_t>(item.get_prevActiveTime()));
            bool resetCounter = false;
            bool year_changed = prevActiveTime.get_year() != time.get_year();

            // �ж��Ƿ���Ҫ����ǩ��������
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

            // ���ǩ������
            if (item.get_signInCount() < item.firstN) {
                if (!item.inc_signInCount(1))
                    continue;
            }
            else
                continue;
        }

        // �޸�Ӳ��, ����, ��Ʒ, ��Ϣ
        coin = static_cast<LL>(coin * item.coinFactor);
        energy = static_cast<LL>(energy * item.coinFactor);
        for (const auto &eqi : item.items)
            items.push_back(eqi);

        msg += "\n" + item.message + (item.firstN == -1 ? "" :
            "(ֻ��ǰ" + std::to_string(item.firstN) + "��ǩ�����������ȡ, ����ȡ" + std::to_string(item.get_signInCount()) + "��)");
    }
}
