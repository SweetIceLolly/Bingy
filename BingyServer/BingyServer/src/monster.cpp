/*
����: Bingy ��������صĲ���
����: ����
�ļ�: monster.cpp
*/

#include "monster.hpp"
#include "utils.hpp"
#include "config_parser.hpp"

#define DEFAULT_MONSTER_PATH    "monsters.txt"

std::string                         monsterConfigPath = DEFAULT_MONSTER_PATH;
std::unordered_map<LL, monsterData> allMonsters;    // ע��: ��ȡ��ʱ����Բ��ü���, ���ǲ�Ҫʹ��[], ��Ҫʹ�� at(). ���߳�д���ʱ��������
std::unordered_map<LL, dungeonData> allDungeons;    // ע��: ��ȡ��ʱ����Բ��ü���, ���ǲ�Ҫʹ��[], ��Ҫʹ�� at(). ���߳�д���ʱ��������
luckyDraw                           forestDraw;     // ɭ�ֳ齱��

// ��ȡ���й�����Ϣ
bool bg_load_monster_config() {
    configParser    parser(monsterConfigPath);
    monsterData     *temp = nullptr;                // ������Ϣ��ʱ����

    return parser.load(
        // �л� state �ص�����. �����ò���. state һֱ����Ϊ 0
        [](const std::string &line, char &state) -> bool {
            return true;
        },

        // ��ȡ����ֵ�ص�����
        [&](const std::string &propName, const std::string &propValue, const char &state, const unsigned int &lineNo) -> bool {
            try {
                if (propName == "id")
                    temp->id = std::stoll(propValue);
                else if (propName == "name")
                    temp->name = propValue;
                else if (propName == "atk")
                    temp->atk = std::stoll(propValue);
                else if (propName == "def")
                    temp->def = std::stoll(propValue);
                else if (propName == "brk")
                    temp->brk = std::stoll(propValue);
                else if (propName == "agi")
                    temp->agi = std::stoll(propValue);
                else if (propName == "hp")
                    temp->hp = std::stoll(propValue);
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
            }
            catch (const std::exception &e) {
                console_log("�����������ʱ��������: ����" + std::to_string(lineNo) + ", ԭ��: " + e.what(), LogType::warning);
            }
            catch (...) {
                console_log("�����������ʱ��������: ����" + std::to_string(lineNo), LogType::warning);
            }
            return true;
        },

        // ��ʼ��ǻص�����
        [&](const char &state) -> bool {
            temp = new monsterData();
            return true;
        },

        // ������ǻص�����
        [&](const char &state) -> bool {
            bool rtn = allMonsters.insert({ temp->id, *temp }).second;
            delete temp;
            if (!rtn)
                console_log("�޷��ѹ��� ID = " + std::to_string(temp->id) + " ��ӵ������б�, ��������Ϊ ID �ظ�", LogType::error);
            return rtn;
        }
    );
}

// ��ʼ��������ֺ͵������
void bg_init_monster_chances() {
    // ��ʼ�����и����й�����ֵĸ���
    for (auto &dungeon : allDungeons) {
        for (const auto &monster : dungeon.second.monsters) {
            dungeon.second.monstersDraw.insertItem(monster, allMonsters.at(monster).dungeonWeight);
        }
    }

    // ��ʼ�����й������ĸ���
    for (auto &monster : allMonsters) {
        monster.second.initDropDraw();
    }
}

// ��ȡ��������С������С����λ��
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

// ���ݹ���ĵ����б��������ɹ���ĵ����ȡ��
void monsterData::initDropDraw() {
    int     maxPrecision = 0;               // �����б��и��ʵ���󾫶�
    double  noDropChance = 1;               // �޵���ĸ��� (= 1 - ���е�������ܺ�)

    for (const auto &item : this->drop) {
        int precision = getDecimals(item.chance);
        if (precision > maxPrecision)
            maxPrecision = precision;
        noDropChance -= item.chance;
    }
    for (const auto &item : this->drop) {
        this->dropDraw.insertItem(item.id, item.chance * pow(10.0, maxPrecision));
    }
    this->dropDraw.insertItem(-1, noDropChance * pow(10.0, maxPrecision));
    if (noDropChance < 0) {
        console_log("����" + this->name + " (id: " + std::to_string(this->id) + ") ��װ����������ܺʹ���1!", LogType::warning);
    }
}
