/*
����: ���ݿ���ز���
����: ����
�ļ�: database.cpp
*/

#include "database.hpp"
#include <mongocxx/instance.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

// һЩ���ݿ������Ҫ��ȫ�ֱ���
std::string             dbUri = DEFAULT_DB_URI;
std::string             dbName = DB_NAME;

mongocxx::instance      mongoInst{};
mongocxx::pool          *mongoPool;

mongocxx::database dbGetDatabase(const char *name) {
    auto client = mongoPool->acquire();
    return (*client)[name];
}

mongocxx::collection dbGetCollection(const char *dbName, const char *collName) {
    auto client = mongoPool->acquire();
    return (*client)[dbName][collName];
}

bool dbInit() {
    try {
        auto mongoUri = mongocxx::uri(dbUri.c_str());   // ���� URI
        static mongocxx::pool pool{ mongoUri };         // �������ӳ�
        mongoPool = &pool;

        // �г��������ݿ��Լ�����Ӽ�Ȩ��
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
