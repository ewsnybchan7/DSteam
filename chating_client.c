#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_USER_NUM 5
#define MAX_NICK_LEN 30
#define MAX_MESSAGE_LEN 1024

#define SERV_IP "127.0.0.1"
#define SERV_PORT 10200

typedef struct _user {
    int usd;
    int unuml;
    int rsd;
    char unick[MAX_NICK_LEN];
} User;

typedef struct _room {
    int rsd;
    int usd[5];
    int ucnt;
} Room;

User list[5];
int sd;
int rsd;
int flag = 0;
int rcnt = 0;
int usernum = 0;
pthread_mutex_t usermutex;

void SigExit(int signo);
int SockSetting(char *ip, int port);
void JoinChat(int ssd);
void *RecvMsg(void *user);
void *SendMsg(void *user);
void *thrdmain(void *us);

int main() {
    int port;
    char pt[MAX_MESSAGE_LEN];

    printf("포트를 입력하세요 :");
    scanf("%d", &port);

    getchar();
    signal(SIGINT, SigExit);
    pthread_mutex_init(&usermutex, NULL);
    if ((sd = SockSetting(SERV_IP, port)) == -1) {
        perror("socker");
        return 0;
    }

    JoinChat(sd);

    return 0;
}

void JoinChat(int ssd) {
    User user;
    pthread_t ptr[2];
    char nick[MAX_NICK_LEN] = "";
    char rbuf[MAX_MESSAGE_LEN] = "";

    recv(ssd, rbuf, 100, 0);
    fputs(rbuf, stdout);

    fgets(nick, sizeof(nick), stdin);
    send(ssd, nick, strlen(nick), 0);

    user.usd = ssd;
    strcmp(user.unick, nick);

    pthread_create(&ptr[0], NULL, RecvMsg, &user);
    pthread_detach(ptr[0]);
    pthread_create(&ptr[1], NULL, RecvMsg, &user);
    pthread_detach(ptr[1]);
    while (1)
        pause();
}

void *RecvMsg(void *user) {
    User us = *(User *)user;
    char rbuf[MAX_MESSAGE_LEN];

    while (1) {
        if (flag == 0) {
            recv(us.usd, rbuf, sizeof(rbuf), 0);
        }

        while (recv(us.usd, rbuf, sizeof(rbuf), 0) > 0) {
            fputs(rbuf, stdout);
            memset(rbuf, 0, sizeof(rbuf));
        }
    }
}

void *SendMsg(void *user) {
    User us = *(User *)user;
    char sbuf[MAX_MESSAGE_LEN];

    while (1) {
        fgets(sbuf, sizeof(sbuf), stdin);
        send(us.usd, sbuf, sizeof(sbuf), 0);
        if (!strncmp(sbuf, "/f", 2))
            flag = 1;

        memset(sbuf, 0, sizeof(sbuf));
    }
}

int SockSetting(char *ip, int port) {
    int ssd;
    if ((ssd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

        perror("sock error");

        return -1;
    }

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    if (connect(ssd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("connet");
        return -1;
    }
    return ssd;
}

void SigExit(int signo) {
    printf("클라이언트를 종료합니다.\n");
    close(sd);
    exit(0);
}
