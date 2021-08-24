/*
描述: Bingy 怪物类相关的操作
作者: 冰棍
文件: monster.cpp
*/

#include "monster.hpp"
#include "utils.hpp"
#include "config_parser.hpp"

#define DEFAULT_MONSTER_PATH    "monsters.txt"

std::string                         monsterConfigPath = DEFAULT_MONSTER_PATH;
std::unordered_map<LL, monsterData> allMonsters;
std::unordered_map<LL, dungeonData> allDungeons;
luckyDraw                           forestDraw;     // 森林抽奖机

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
        [&](const std::string &propName, const std::string &propValue, char state, unsigned int lineNo) -> bool {
            try {
                if (propName == "id")
                    temp->id = std::stoll(propValue);
                else if (propName == "name")
                    temp->name = propValue;
                else if (propName == "atk")
                    temp->atk = std::stod(propValue);
                else if (propName == "def")
                    temp->def = std::stod(propValue);
                else if (propName == "brk")
                    temp->brk = std::stod(propValue);
                else if (propName == "agi")
                    temp->agi = std::stod(propValue);
                else if (propName == "hp")
                    temp->hp = std::stod(propValue);
                else if (propName == "crt")
                    temp->crt = std::stod(propValue);
                else if (propName == "dungeonweight")
                    temp->dungeonWeight = std::stoll(propValue);
                else if (propName == "forestweight")
                    temp->forestWeight = std::stoll(propValue);
                else if (propName == "coin")
                    temp->coin = std::stoll(propValue);
                else if (propName == "exp")
                    temp->exp = std::stoll(propValue);
                else if (propName == "message")
                    temp->message = propValue;
                else if (propName == "drop") {
                    for (const auto &it : str_split(propValue, ',')) {
                        // drop=id1:chance1,id2:chance2,id3:chance3, ...
                        auto dropProp = str_split(it, ':');
                        auto *dropInfo = new monsterDrop();
                        dropInfo->id = std::stoll(dropProp[0]);
                        dropInfo->chance = std::stod(dropProp[1]);
                        temp->drop.push_back(*dropInfo);
                        delete dropInfo;
                    }
                }
                else {
                    console_log(std::string("未知的配置名: \"") + propName +
                        std::string("\", 于行") + std::to_string(lineNo), LogType::warning);
                }
            }
            catch (const std::exception &e) {
                console_log("处理怪物配置时发生错误: 于行" + std::to_string(lineNo) + ", 原因: " + e.what(), LogType::warning);
            }
            catch (...) {
                console_log("处理怪物配置时发生错误: 于行" + std::to_string(lineNo), LogType::warning);
            }
            return true;
        },

        // 开始标记回调函数
        [&](char state) -> bool {
            temp = new monsterData();
            return true;
        },

        // 结束标记回调函数
        [&](char state) -> bool {
            bool rtn = allMonsters.insert({ temp->id, *temp }).second;
            delete temp;
            if (!rtn)
                console_log("无法把怪物 ID = " + std::to_string(temp->id) + " 添加到怪物列表, 可能是因为 ID 重复", LogType::error);
            return rtn;
        }
    );
}

// 初始化怪物出现和掉落概率
void bg_init_monster_chances() {
    // 初始化所有副本中怪物出现的概率
    for (auto &dungeon : allDungeons) {
        for (const auto &monster : dungeon.second.monsters) {
            dungeon.second.monstersDraw.insertItem(monster, allMonsters.at(monster).dungeonWeight);
        }
    }

    // 初始化所有怪物掉落的概率
    for (auto &monster : allMonsters) {
        monster.second.initDropDraw();
    }
}

// 获取浮点数的小数点后的小数点位数
inline int getDecimals(double n) {
    int rtn = 0;
    n -= static_cast<int>(n);
    while (abs(n) > 1e-16 && rtn < 16) {
        n *= 10;
        ++rtn;
        n -= static_cast<int>(n);
    }
    return rtn;
}

// 根据怪物的掉落列表重新生成怪物的掉落抽取器
void monsterData::initDropDraw() {
    int     maxPrecision = 0;               // 掉落列表中概率的最大精度
    double  noDropChance = 1;               // 无掉落的概率 (= 1 - 所有掉落概率总和)

    for (const auto &item : this->drop) {
        int precision = getDecimals(item.chance);
        if (precision > maxPrecision)
            maxPrecision = precision;
        noDropChance -= item.chance;
    }
    for (const auto &item : this->drop) {
        this->dropDraw.insertItem(item.id, static_cast<LL>(item.chance * pow(10.0, maxPrecision)));
    }
    this->dropDraw.insertItem(-1, static_cast<LL>(noDropChance * pow(10.0, maxPrecision)));
    if (noDropChance < 0) {
        console_log("怪物" + this->name + " (id: " + std::to_string(this->id) + ") 的装备掉落概率总和大于1!", LogType::warning);
    }
}
