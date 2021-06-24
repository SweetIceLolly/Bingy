/*
描述: 数据库相关操作的接口
作者: 冰棍
文件: database.hpp
*/

#pragma once

#include <string>

#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/exception/exception.hpp>

#include <bsoncxx/builder/stream/document.hpp>

#include "utils.hpp"

// 定义一些常量
#define DEFAULT_DB_URI      "mongodb://127.0.0.1:27017/dreamy"
#define DB_NAME             "bingy"
#define DB_COLL_USERDATA    "userdata"
#define DB_COLL_SIGNIN      "signin"
#define DB_COLL_TRADE       "trade"

// 指定数据库 URI 和库名, 可以在调用 dbInit 前修改
extern std::string dbUri;
extern std::string dbName;

// 初始化数据库连接. 如果初始化时发生错误 (包括但不限于 URI 无效, 连接超时等), 则返回false
bool dbInit();

// 检查连接. 如果连接有效, 则返回 true
bool dbPing();

// 获取一个数据库对象. 该函数是线程安全的, 可以从不同线程同时调用
mongocxx::database dbGetDatabase(const char *name);

// 获取一个 Collection 对象. 该函数是线程安全的, 可以从不同线程同时调用
mongocxx::collection dbGetCollection(const char *dbName, const char *collName);

// 往指定 collection 新加一个文档
bool dbInsertDocument(const char *collName, const bsoncxx::document::value &value);

// 在指定 collection 更新一个文档
template <typename T1, typename T2>
bool dbUpdateOne(const char *collName, const std::string &filterKey, const T1 &filterVal,
    const std::string &updateKey, const T2 &updateVal);

// 更新指定 collection 中所有符合条件的文档
template <typename T1, typename T2>
bool dbUpdateAll(const char *collName, const std::string &filterKey, const T1 &filterVal,
    const std::string &updateKey, const T2 &updateVal);

// 更新指定 collection 中所有文档
template <typename T>
bool dbUpdateAll(const char *collName, const std::string &updateKey, const T &updateVal);

// 获取指定 collection 中一个符合条件的文档
template <typename T, typename ... Args>
std::optional<bsoncxx::document::value> dbFindOne(const char *collName, const std::string &key, const T &value, Args&&... rtnFields);

// 获取指定 collection 中所有符合条件的文档
template <typename T, typename ... Args>
mongocxx::cursor dbFindAll(const char *collName, const std::string &filterKey, const T &filterValue, Args&&... rtnFields);

// 获取指定 collection 中所有文档
template <typename ... Args>
mongocxx::cursor dbFindAll(const char *collName, Args&&... rtnFields);

// 移除指定 collection 中一个符合条件的文档
template <typename T>
bool dbDeleteOne(const char *collName, const std::string &filterKey, const T &filterVal);

// 移除指定 collection 中所有符合条件的文档
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
        throw std::exception("dbFindAll 发生错误!");
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
