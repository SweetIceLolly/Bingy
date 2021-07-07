/*
����: Bingy ����������ڵ�, ������ó�ʼ�������ͷַ��¼�
����: ����
�ļ�: bingy.cpp
*/

#include "bingy.hpp"
#include "utils.hpp"
#include "init.hpp"
#include "http_router.hpp"

int main() {
    rest_server server;

    // ��ʼ�� HTTP ������·��
    init_server_router(server);

    // ��ʼ�� Bingy
    auto startTime = std::chrono::high_resolution_clock::now();
    if (bg_init()) {
        std::chrono::duration<double> timeDiff = std::chrono::high_resolution_clock::now() - startTime;
        console_log("Bingy ���سɹ�, ��ʱ" + std::to_string(timeDiff.count() * 1000) + "ms");
    }
    else {
        console_log("Bingy ��ʼ��ʧ��!", LogType::error);
        return 1;
    }
    
    // ���� HTTP ������
    console_log("�������� HTTP ������...");
    server.startServer("127.0.0.1:8000", nullptr);

    return 0;
}
