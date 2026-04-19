#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#include <Winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char** argv)
{
    char server_addr[200] = { 0 };
    unsigned short port = 0;// порт сервера
    if (argc > 2) {
        for (int j = 0; j < strlen(argv[1]) && j < 200; j++) {
            server_addr[j] = argv[1][j];
        }
        port = (unsigned short)strtol(argv[2], NULL, 0); // Конвертация str to long
        if (!server_addr[0] || port <= 0 || port > 65535) {
            printf("Error occured while reading cfg file\n");
            return 1;
        }
    }
    else {
        printf("Error arguments was recieved with file\n");
        return 1;
    }

    struct sockaddr_in addr;          // тут будет хранится адресная информация cервера
    int s_id = -1;                    // идентификатор сокета
    char buf[65536] = { 0 };                  // зададим буфер на максимально возможное сообщение в tcp + 1 байт
    int byte_count = 0;               // кол-во полученных байтов
    int err_no = 0;                   //

#ifdef _WIN32
    // Инициализация библиотеки windows
    WSADATA wsa_data;
    unsigned int w_wer_req = MAKEWORD(1, 0);

    if (err_no = WSAStartup(w_wer_req, &wsa_data))
    {
        printf("WSAStartup returned an error: %d", err_no);
        return 1;
    }
#endif

    // задаём протокол сетевого уровня (IP),
    // порт, на котором "сидит" сервер (3030)
    //  и адрес, на котором сидит "сервер"
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server_addr, &(addr.sin_addr.s_addr));

    // запрашиваем у системы ресурсы под сокет
    s_id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (s_id < 0)
    {
        printf("Socket was not created\n");
#ifdef _WIN32
        if (err_no = WSACleanup())
        {
            printf("WSACleanup returned an error: %d", err_no);
        }
#endif
        return 1;
    }

    err_no = connect(s_id, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (err_no < -1)
    {
        printf("Cannot connect to the server\n");
#ifdef _WIN32
        closesocket(s_id);
        if (err_no = WSACleanup())
        {
            printf("WSACleanup returned an error: %d", err_no);
        }
#else
        close(s_id);
#endif
        printf("Socket closed\n");
        return 1;
    }

    buf[0] = 0;
    strcat(buf, "!");
    bool flag_server = true;

    while (flag_server)
    {

        byte_count = send(s_id, buf, strlen(buf), 0);
        if (byte_count < 0)
        {
            printf("Error send\n");
            flag_server = false;
            continue;
        }
        printf("Send message: %s\n", buf);

        byte_count = recv(s_id, buf, sizeof(buf) - 1, 0);
        if (byte_count >= 0)
        {
            buf[byte_count] = 0;
            printf("Recieved message: %s\n\n", buf);
        }

        else
        {
            printf("Error recv\n");
            flag_server = false;
        }
        /*                                      */
        /* ************************************ */
    }

    shutdown(s_id, 2);
#ifdef _WIN32
    closesocket(s_id);
    if (err_no = WSACleanup())
    {
        printf("WSACleanup returned an error: %d", err_no);
    }
#else
    close(s_id);
#endif

    printf("Socket closed\n");
    return 0;
}