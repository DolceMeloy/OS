#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define PORT 5500
#define BACKLOG 5
#define SIGCONNUM 3
#define BUFSIZE 2048

volatile sig_atomic_t sighupReceived = 0;

void handler(int sigNum) {
    sighupReceived = 1;  // Устанавливаем флаг, когда сигнал SIGHUP получен
    printf("SIGHUP received\n");
}

int Socket(int domain, int type, int protocol) {
    int res = socket(domain, type, protocol);
    if (res < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = bind(sockfd, addr, addrlen);
    if (res < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog) {
    int res = listen(sockfd, backlog);
    if (res < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int res = accept(sockfd, addr, addrlen);
    if (res < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    return res;
}

int main() {
    int max, bytes;
    int incomingSockFD = 0;
    struct sockaddr_in socketAddress;
    struct sigaction sa;
    fd_set readfds;
    sigset_t maskBlocked, maskOrig;
    char buf[BUFSIZE] = {0};

    // Создание серверного сокета
    int serverFD = Socket(AF_INET, SOCK_STREAM, 0);

    socketAddress.sin_family = AF_INET;
    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(PORT);

    // Привязка сокета к порту
    Bind(serverFD, (struct sockaddr*)&socketAddress, sizeof(socketAddress));
    
    // Ожидание подключений
    Listen(serverFD, BACKLOG);

    // Обработчик сигнала SIGHUP
    sigaction(SIGHUP, NULL, &sa);
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &sa, NULL);

    // Блокировка сигнала SIGHUP
    sigemptyset(&maskBlocked);
    sigemptyset(&maskOrig);
    sigaddset(&maskBlocked, SIGHUP);
    sigprocmask(SIG_BLOCK, &maskBlocked, &maskOrig);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(serverFD, &readfds);

        if (incomingSockFD > 0) {
            FD_SET(incomingSockFD, &readfds);
        }

        // Определяем максимальный дескриптор для pselect
        max = (incomingSockFD > serverFD) ? incomingSockFD : serverFD;

        // Ожидание события на одном из сокетов
        if (pselect(max + 1, &readfds, NULL, NULL, NULL, &maskOrig) < 0 && errno != EINTR) {
            perror("pselect failed");
            exit(EXIT_FAILURE);
        }

        // Обработка сигнала SIGHUP
        if (sighupReceived) {
            printf("SIGHUP received\n");
            sighupReceived = 0;
            continue;
        }

        // Если есть данные на соединении
        if (incomingSockFD > 0 && FD_ISSET(incomingSockFD, &readfds)) {
            bytes = read(incomingSockFD, buf, BUFSIZE);
            if (bytes > 0) {
                printf("Received %d bytes\n", bytes);
            } else {
                if (bytes == 0) {
                    printf("Connection closed.\n");
                    close(incomingSockFD);
                    incomingSockFD = 0;
                } else {
                    perror("read error");
                }
            }
            continue;
        }

        // Если новое соединение на сервере
        if (FD_ISSET(serverFD, &readfds)) {
            struct sockaddr_in clientAddress;
            socklen_t addrlen = sizeof(clientAddress);

            // Принимаем новое соединение
            int newSockFD = Accept(serverFD, (struct sockaddr*)&clientAddress, &addrlen);
            printf("New connection.\n");

            // Если уже есть активное соединение, закрываем новое
            if (incomingSockFD > 0) {
                printf("Closing new connection immediately.\n");
                close(newSockFD);
            } else {
                incomingSockFD = newSockFD;
            }
        }
    }

    close(serverFD);
    return 0;
//sss
}
