/*
描述: Bingy 怪物类相关的操作
作者: 冰棍
文件: monster.cpp
*/

#include "monster.hpp"
#include "utils.hpp"
#include "config_parser.hpp"

#define DEFAULT_MONSTER_PATH    "monsters.txt"

std::string monsterConfigPath = DEFAULT_MONSTER_PATH;
std::unordered_map<LL, monsterData> allMonsters;

// 读取所有怪物信息
bool bg_load_monster_config() {
    configParser    parser(monsterConfigPath);
    monsterData     *temp = nullptr;                // 怪物信息临时变量

    return parser.load(
        // 切换 state 回调函数. 这里用不上. state 一直保持为 0
        [](const std::string &line, char &state) -> bool {
            return true;
        },

        // 获取属性值回调函数
        [&](const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo) -> bool {
            try {
                if (propName == "id")
                    temp->id = std::stoll(propValue);
                // todo
            }
            catch (...) {
                console_log("处理怪物配置时发生错误: 于行" + std::to_string(lineNo), LogType::warning);
            }
            return true;
        },

        // 开始标记回调函数
        [&](const char &state) -> bool {
            temp = new monsterData();
            return true;
        },

        // 结束标记回调函数
        [&](const char &state) -> bool {
            bool rtn = allMonsters.insert({ temp->id, *temp }).second;
            delete temp;
            if (!rtn)
                console_log("无法把怪物 ID = " + std::to_string(temp->id) + " 添加到怪物列表, 可能是因为 ID 重复", LogType::error);
            return rtn;
        }
    );
}
