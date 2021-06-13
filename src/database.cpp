/*
描述: 数据库相关操作
作者: 冰棍
文件: database.cpp
*/

#include "database.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

// 一些数据库操作需要的全局变量
std::string             dbUri = DEFAULT_DB_URI;
std::string             dbName = DB_NAME;

mongocxx::instance      mongoInst{};
mongocxx::pool          *mongoPool;

// 获取一个数据库对象. 该函数是线程安全的, 可以从不同线程同时调用
mongocxx::database dbGetDatabase(const char *name) {
    auto client = mongoPool->acquire();
    return (*client)[name];
}

// 获取一个 Collection 对象. 该函数是线程安全的, 可以从不同线程同时调用
mongocxx::collection dbGetCollection(const char *dbName, const char *collName) {
    auto client = mongoPool->acquire();
    return (*client)[dbName][collName];
}

// 初始化数据库连接. 如果初始化时发生错误 (包括但不限于 URI无效, 连接超时等), 则返回false
bool dbInit() {
    try {
        auto mongoUri = mongocxx::uri(dbUri.c_str());   // 处理 URI
        static mongocxx::pool pool{ mongoUri };         // 创建连接池
        mongoPool = &pool;

        // 列出可用数据库以检察连接及权限
        auto client = pool.acquire();
        (*client).list_database_names();

        return true;
    }
    catch (const mongocxx::exception &e) {
        console_log(e.what(), LogType::error);
        return false;
    }
    catch (...) {
        return false;
    }
}

// 检查连接. 如果连接有效, 则返回 true
bool dbPing() {
    try {
        dbGetDatabase(dbName.c_str()).run_command(make_document(kvp("ping", 1)));
        return true;
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

// 往指定 collection 新加一个文档
bool dbInsertDocument(const char *collName, const bsoncxx::document::value &value) {
    try {
        auto result = dbGetCollection(dbName.c_str(), collName).insert_one(value.view());
        if (result)
            return result->result().inserted_count() == 1;
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
