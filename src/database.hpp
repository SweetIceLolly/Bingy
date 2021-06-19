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

// 指定数据库 URI 和库名, 可以在调用 dbInit 前修改
extern std::string dbUri;
extern std::string dbName;

bool dbInit();
bool dbPing();
mongocxx::database dbGetDatabase(const char *name);
mongocxx::collection dbGetCollection(const char *dbName, const char *collName);
bool dbInsertDocument(const char *collName, const bsoncxx::document::value &value);

template <typename T, typename ... Args>
std::optional<bsoncxx::document::value> dbFindOne(const char *collName, const std::string &key, const T &value, Args&&... rtnFields);

template <typename T1, typename T2>
bool dbUpdateOne(const char *collName, const std::string &filterKey, const T1 &filterVal,
    const std::string &updateKey, const T2 &updateVal);

template <typename T, typename ... Args>
mongocxx::cursor dbFindAll(const char *collName, const std::string &filterKey, const T &filterValue, Args&&... rtnFields);

template <typename ... Args>
mongocxx::cursor dbFindAll(const char *collName, Args&&... rtnFields);

// ===========================================================================

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
    catch (...) {
        return std::nullopt;
    }
}

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
    catch (...) {
        return false;
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
    catch (...) {
        throw std::exception("dbFindAll 发生错误!");
    }
}
