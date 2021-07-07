/*
����: ���ݿ���ز����Ľӿ�
����: ����
�ļ�: database.hpp
*/

#pragma once

#include <string>

#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/exception/exception.hpp>

#include <bsoncxx/builder/stream/document.hpp>

#include "utils.hpp"

// ����һЩ����
#define DEFAULT_DB_URI      "mongodb://127.0.0.1:27017/dreamy"
#define DB_NAME             "bingy"
#define DB_COLL_USERDATA    "userdata"
#define DB_COLL_SIGNIN      "signin"
#define DB_COLL_TRADE       "trade"

// ָ�����ݿ� URI �Ϳ���, �����ڵ��� dbInit ǰ�޸�
extern std::string dbUri;
extern std::string dbName;

// ��ʼ�����ݿ�����. �����ʼ��ʱ�������� (������������ URI ��Ч, ���ӳ�ʱ��), �򷵻�false
bool dbInit();

// �������. ���������Ч, �򷵻� true
bool dbPing();

// ��ȡһ�����ݿ����. �ú������̰߳�ȫ��, ���ԴӲ�ͬ�߳�ͬʱ����
mongocxx::database dbGetDatabase(const char *name);

// ��ȡһ�� Collection ����. �ú������̰߳�ȫ��, ���ԴӲ�ͬ�߳�ͬʱ����
mongocxx::collection dbGetCollection(const char *dbName, const char *collName);

// ��ָ�� collection �¼�һ���ĵ�
bool dbInsertDocument(const char *collName, const bsoncxx::document::value &value);

// ��ָ�� collection ����һ���ĵ�
template <typename T1, typename T2>
bool dbUpdateOne(const char *collName, const std::string &filterKey, const T1 &filterVal,
    const std::string &updateKey, const T2 &updateVal);

// ����ָ�� collection �����з����������ĵ�
template <typename T1, typename T2>
bool dbUpdateAll(const char *collName, const std::string &filterKey, const T1 &filterVal,
    const std::string &updateKey, const T2 &updateVal);

// ����ָ�� collection �������ĵ�
template <typename T>
bool dbUpdateAll(const char *collName, const std::string &updateKey, const T &updateVal);

// ��ȡָ�� collection ��һ�������������ĵ�
template <typename T, typename ... Args>
std::optional<bsoncxx::document::value> dbFindOne(const char *collName, const std::string &key, const T &value, Args&&... rtnFields);

// ��ȡָ�� collection �����з����������ĵ�
template <typename T, typename ... Args>
mongocxx::cursor dbFindAll(const char *collName, const std::string &filterKey, const T &filterValue, Args&&... rtnFields);

// ��ȡָ�� collection �������ĵ�
template <typename ... Args>
mongocxx::cursor dbFindAll(const char *collName, Args&&... rtnFields);

// �Ƴ�ָ�� collection ��һ�������������ĵ�
template <typename T>
bool dbDeleteOne(const char *collName, const std::string &filterKey, const T &filterVal);

// �Ƴ�ָ�� collection �����з����������ĵ�
template <typename T>
bool dbDeleteAll(const char *collName, const std::string &filterKey, const T &filterVal);

// ===========================================================================

template <typename T1, typename T2>
bool dbUpdateOne(const char *collName, const std::string &filterKey, const T1 &filterVal,
    const std::string &updateKey, const T2 &updateVal) {

    try {
        auto result = dbGetCollection(dbName.c_str(), collName).update_one(
            bsoncxx::builder::stream::document{} << filterKey << filterVal << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << updateKey << updateVal << bsoncxx::builder::stream::finalize
        );

        if (result.has_value())
            return result->matched_count() == 1;
        else
            return false;
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (...) {
        return false;
    }
}

template <typename T1, typename T2>
bool dbUpdateAll(const char *collName, const std::string &filterKey, const T1 &filterVal,
    const std::string &updateKey, const T2 &updateVal) {

    try {
        auto result = dbGetCollection(dbName.c_str(), collName).update_many(
            bsoncxx::builder::stream::document{} << filterKey << filterVal << bsoncxx::builder::stream::finalize,
            bsoncxx::builder::stream::document{} << updateKey << updateVal << bsoncxx::builder::stream::finalize
        );

        if (result.has_value())
            return result->matched_count() > 0;
        else
            return false;
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (...) {
        return false;
    }
}

template <typename T>
bool dbUpdateAll(const char *collName, const std::string &updateKey, const T &updateVal) {
    try {
        auto result = dbGetCollection(dbName.c_str(), collName).update_many({},
            bsoncxx::builder::stream::document{} << updateKey << updateVal << bsoncxx::builder::stream::finalize
        );

        if (result.has_value())
            return result->matched_count() > 0;
        else
            return false;
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (...) {
        return false;
    }
}

template <typename T, typename ... Args>
std::optional<bsoncxx::document::value> dbFindOne(const char *collName, const std::string &key, const T &value, Args&&... rtnFields) {
    try {
        mongocxx::options::find opts{};
        bsoncxx::builder::basic::document doc;
        ((doc.append(bsoncxx::builder::basic::kvp(rtnFields, 1))), ...);
        doc.append(bsoncxx::builder::basic::kvp("_id", 0));
        opts.projection(doc.extract());

        bsoncxx::document::value filter = bsoncxx::builder::stream::document{}
            << key << value
            << bsoncxx::builder::stream::finalize;

        return dbGetCollection(dbName.c_str(), collName).find_one(filter.view(), opts);
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        return std::nullopt;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
        return std::nullopt;
    }
    catch (...) {
        return std::nullopt;
    }
}

template <typename T, typename ... Args>
mongocxx::cursor dbFindAll(const char *collName, const std::string &filterKey, const T &filterValue, Args&&... rtnFields) {
    try {
        mongocxx::options::find opts{};
        bsoncxx::builder::basic::document doc;
        ((doc.append(bsoncxx::builder::basic::kvp(rtnFields, 1))), ...);
        doc.append(bsoncxx::builder::basic::kvp("_id", 0));
        opts.projection(doc.extract());

        bsoncxx::document::value filter = bsoncxx::builder::stream::document{}
            << filterKey << filterValue
            << bsoncxx::builder::stream::finalize;

        return dbGetCollection(dbName.c_str(), collName).find(filter.view(), opts);
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        return std::nullopt;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
        return std::nullopt;
    }
    catch (...) {
        return std::nullopt;
    }
}

template <typename ... Args>
mongocxx::cursor dbFindAll(const char *collName, Args&&... rtnFields) {
    try {
        mongocxx::options::find opts{};
        bsoncxx::builder::basic::document doc;
        ((doc.append(bsoncxx::builder::basic::kvp(rtnFields, 1))), ...);
        doc.append(bsoncxx::builder::basic::kvp("_id", 0));
        opts.projection(doc.extract());

        return dbGetCollection(dbName.c_str(), collName).find({}, opts);
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        throw e;
    }
    catch (const std::exception &e) {
        console_log(e.what(), LogType::error);
        throw e;
    }
    catch (...) {
        throw std::exception("dbFindAll ��������!");
    }
}

template <typename T>
bool dbDeleteOne(const char *collName, const std::string &filterKey, const T &filterVal) {
    try {
        auto result = dbGetCollection(dbName.c_str(), collName).delete_one(
            bsoncxx::builder::stream::document{} << filterKey << filterVal << bsoncxx::builder::stream::finalize
        );
        if (result.has_value())
            return result->deleted_count() == 1;
        else
            return false;
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (...) {
        return false;
    }
}

template <typename T>
bool dbDeleteAll(const char *collName, const std::string &filterKey, const T &filterVal) {
    try {
        auto result = dbGetCollection(dbName.c_str(), collName).delete_many(
            bsoncxx::builder::stream::document{} << filterKey << filterVal << bsoncxx::builder::stream::finalize
        );
        if (result.has_value())
            return result->deleted_count() > 0;
        else
            return false;
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (...) {
        return false;
    }
}
