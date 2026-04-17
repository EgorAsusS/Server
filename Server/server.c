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
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    FILE* fcfg = NULL;
    if (argc > 1) {
        fcfg = fopen(argv[1], "r");
    }
    char IP[16] = { 0 };
    int PORT = 0;
    if (fcfg) {
        int count = fscanf(fcfg, "IP: %15s Port: %d\n", IP, &PORT);
        if (!count || PORT <= 0 || PORT > 65535) {
            printf("Error occured while reading cfg file\n");
            return 1;
        }
    }
    else {
        printf("Error occured while opening cfg file\n");
        return 1;
    }
    
    struct sockaddr_in addr;               // тут будет хранится наш адрес
    int s_id = -1;                         // идентификатор сокета
    struct sockaddr_in client_addr;        // тут будет храниться адрес клиента
    char cl_addr_str[INET_ADDRSTRLEN + 1]; // буфер для хранения адреса клиента в десятично точечной нотации
    int cl_s_id = -1;                      // идентификатор сокета для работы с конкретным клиентом
    // сюда положим длину структуры с клиентским адресом
#ifdef _WIN32
    int cl_addr_len = 0;
#else
    unsigned int cl_addr_len = 0;
#endif
    unsigned short port = PORT; // порт
    char buf[65536];            // зададим буфер на максимально возможное сообщение в tcp + 1 байт
    int byte_count = 0;         // кол-во полученных байтов
    char* tmp;                  // указател на строку, в которой будет храниться адрес клиента
    int err_no = 0;

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
    // порт, на котором будем прослушивать соединения (3030)
    //  и адрес, на котором будем ожидать соединения (INADDR_ANY означает все доступные адреса машины)
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, IP, &(addr.sin_addr.s_addr));

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

    // привяжем гнездо к нашим порту и адресу
    err_no = bind(s_id, (struct sockaddr*)&addr, sizeof(struct sockaddr_in));
    if (err_no < 0)
    {
        printf("Binding error\n");
#ifdef _WIN32
        closesocket(cl_s_id);
        if (err_no = WSACleanup())
        {
            printf("WSACleanup returned an error: %d", err_no);
        }
#else
        close(cl_s_id);
#endif
        return 1;
    }

    // объявим гнездо гнездом для ожидания соединений
    err_no = listen(s_id, 5);
    if (err_no < 0)
    {
        printf("Listen error\n");
#ifdef _WIN32
        closesocket(cl_s_id);
        if (err_no = WSACleanup())
        {
            printf("WSACleanup returned an error: %d", err_no);
        }
#else
        close(cl_s_id);
#endif
        return 1;
    }

    // бесконечный цикл. По-хорошему он должен прерываться сигналом из вне
    while (1)
    {
        printf("Waiting for a new connection!\n");

        // ждём...
        cl_addr_len = sizeof(struct sockaddr_in);
        cl_s_id = accept(s_id, (struct sockaddr*)&client_addr, &cl_addr_len);

        if (cl_s_id < 0)
        {
            // TODO обработать ошибку
        }
        else {
            /* ************************************ */
            //   работаем с клиентом
            // cl_s_id -- идентификатор сокета для работы с конкретным клиентом

            // преобразуем адрес клиента в строковое представление
            cl_addr_str[0] = 0;
            tmp = (char*)inet_ntop(AF_INET, &client_addr.sin_addr, cl_addr_str, INET_ADDRSTRLEN + 1);
            // занулим адресную инфу клиента (она нам больше не нужна)
            memset(&client_addr, 0, sizeof(struct sockaddr_in));

            bool flag_client = true;
            while (flag_client) {
                byte_count = recv(cl_s_id, buf, sizeof(buf) - 1, 0);
                if (byte_count >= 0)
                {
                    //buf[byte_count] = 0;
                    printf("Recieved message: %s\n", buf);
                    strcat(buf, "!");
                    printf("Send message: %s\n\n", buf);
                    send(cl_s_id, buf, byte_count + 1, 0);
                }
                else
                {
                    printf("Error recv\n");
                    flag_client = false;
                }
            }
                shutdown(cl_s_id, 2);
            #ifdef _WIN32
                closesocket(cl_s_id);
            #else
                close(cl_s_id);
            #endif
        }
        /*                                      */
        /* ************************************ */
    }

    // Завершение
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