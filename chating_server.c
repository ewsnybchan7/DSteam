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
    int unum;
    int rsd;
    char unick[MAX_NICK_LEN];
} User;

typedef struct _room {
    int rsd;
    int usd[5];
    int ucnt;
} Room;

User list[5];
Room rlist[5];

int sd;
int rsd;
int flag = 0;
int rcnt = 0;
int usernum = 0;
int rusernum = 0;
pthread_mutex_t usermutex, roommutex;

void SigExit(int signo);
int ServerSetting(char *ip, int prot);
void *JoinChat(void *user);
void *DeliveryMessage(void *user);
void *Notice(void *user);
void Whisp(User user, char *rbuf);
void ClientExit(User user);
void *thrdmain(void *room);
void FileIO(User user, char *rbuf);
int main() {
    int room, i;

    signal(SIGINT, SigExit);
    pthread_mutex_init(&usermutex, NULL);
    pthread_mutex_init(&roommutex, NULL);

    printf("방의 개수를 입력");
    scanf("%d", &room);
    getchar();

    pthread_t ptr[256];
    for (i = 0; i < room; i++) {
        pthread_create(&ptr[i], NULL, thrdmain, &room);
        pthread_detach(ptr[i]);
    }
    while (1)
        pause();
    return 0;
}

void *thrdmain(void *room) {
    pthread_mutex_lock(&roommutex);
    User user;

    Room rm;

    int ssd;
    int port = SERV_PORT + rcnt;
    rm.ucnt = rcnt;

    ssd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(ssd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        perror("bind error");
        close(ssd);
    }
    if (listen(ssd, 5) == -1) {
        perror("listen error");
        close(ssd);
    }

    printf("port %d\n", port);

    rcnt++;

    pthread_mutex_unlock(&roommutex);

    struct sockaddr_in cliaddr = {0};
    int clen = sizeof(cliaddr);
    pthread_t ptr;

    while ((user.usd = accept(ssd, (struct sockaddr *)&cliaddr, &clen))) {
        if (user.usd == -1) {
            perror("accept");
            return 0;
        }

        pthread_mutex_lock(&usermutex);

        user.unum = usernum;

        user.rsd = ssd;
        rm.rsd = ssd;
        rm.usd[rm.ucnt] == user.usd;

        rlist[rm.ucnt] = rm;
        pthread_create(&ptr, NULL, JoinChat, &user);
        pthread_detach(ptr);

        usernum++;
        rm.ucnt++;

        pthread_mutex_unlock(&usermutex);
    }
    return 0;
}

void *JoinChat(void *user) {
    User us = *(User *)user;
    char nick[MAX_NICK_LEN] = "";

    pthread_t ptr[2];

    send(us.usd, "채팅방에 오신 것을 환영합니다.\n닉네임을 입력하세요.\n", 100,
         0);
    recv(us.usd, nick, sizeof(nick), 0);
    nick[strlen(nick) - 1] = '\0';
    printf("%s 접속\n", nick);

    strcpy(us.unick, nick);

    list[us.unum] = us;

    pthread_create(&ptr[0], NULL, DeliveryMessage, &us);
    pthread_create(&ptr[1], NULL, Notice, &us);
    pthread_join(ptr[0], NULL);
    pthread_join(ptr[1], NULL);
}

void *DeliveryMessage(void *user) {
    User us = *(User *)user;
    int i;

    char *whisp;

    char rbuf[MAX_MESSAGE_LEN];
    char sbuf[MAX_MESSAGE_LEN];

    while (recv(us.usd, rbuf, sizeof(rbuf), 0) > 0) {
        if (!strncmp(rbuf, "/w", 2)) {
            Whisp(us, rbuf);
            continue;
        } else if (!strncmp(rbuf, "/f", 2)) {
            FileIO(us, rbuf);
            continue;
        } else {
            sprintf(sbuf, "%s: %s", us.unick, rbuf);
            for (i = 0; i < usernum; i++) {
                if (list[i].usd == us.usd)
                    continue;
                if (list[i].rsd == us.rsd)
                    send(list[i].usd, sbuf, sizeof(sbuf), 0);
            }
        }

        memset(rbuf, 0, sizeof(rbuf));
        memset(sbuf, 0, sizeof(sbuf));
    }
    ClientExit(us);
}

void FileIO(User user, char *rbuf) {
    int fd;
    char *fio;

    char sbuf[MAX_MESSAGE_LEN];

    fio = strtok(rbuf, " ");
    fd = open(fio, O_RDONLY, 0644);
    while (read(fd, rbuf, sizeof(rbuf)) > 0) {
        if (send(user.usd, rbuf, sizeof(rbuf), 0) == -1) {
            perror("send");
            exit(1);
        }
    }
}

void ClientExit(User user) {
    int i, j;
    char sbuf[MAX_MESSAGE_LEN];

    sprintf(sbuf, "%s 님이 퇴장하셨습니다.\n", user.unick);
    for (i = 0; i < usernum; i++) {
        if (list[i].usd == user.usd) {
            for (j = 0; j < usernum; j++) {
                if (list[j].usd == user.usd)
                    continue;
                if (list[j].rsd == user.usd)
                    continue;
                if (list[j].rsd == user.rsd)
                    send(list[j].usd, sbuf, sizeof(sbuf), 0);
            }
            break;
        }
    }
    pthread_mutex_lock(&usermutex);
    for (j = i; j < usernum - 1; j++) {
        list[j] = list[j + 1];
    }
    usernum--;
    pthread_mutex_unlock(&usermutex);
}

void Whisp(User user, char *rbuf) {
    int i;
    char *whisp;
    char sbuf[MAX_MESSAGE_LEN];

    strtok(rbuf, " ");
    whisp = strtok(NULL, " ");

    for (i = 0; i < usernum; i++) {
        if (!strcmp(whisp, list[i].unick)) {
            whisp = strtok(NULL, " ");
            sprintf(sbuf, "%s님의 귓속말 : %s", user.unick, whisp);
            if (list[i].rsd == user.rsd)
                send(list[i].usd, sbuf, strlen(sbuf), 0);
            break;
        }
    }
    if (i == usernum)
        send(user.usd, "사용자가없습니다.\n", 30, 0);
}

void *Notice(void *user) {
    int i;
    User us = *(User *)user;
    char sbuf[MAX_MESSAGE_LEN];
    char inbuf[MAX_MESSAGE_LEN];

    sprintf(sbuf, "%s 님이 접속하였습니다.\n", us.unick);
    for (i = 0; i < usernum; i++) {
        if (list[i].rsd == us.rsd)
            send(list[i].usd, sbuf, strlen(sbuf), 0);
    }
    while (1) {
        memset(sbuf, 0, sizeof(sbuf));
        memset(inbuf, 0, sizeof(inbuf));
        fgets(inbuf, sizeof(inbuf), stdin);
        sprintf(sbuf, "[공지]:%s\n", inbuf);
        for (i = 0; i < usernum; i++)
            send(list[i].usd, sbuf, strlen(sbuf), 0);
    }
}
int ServerSetting(char *ip, int port) {
    int ssd;
    ssd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);

    if (bind(ssd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        perror("bind error");
        close(ssd);
        return -1;
    }
    if (listen(ssd, 5) == -1) {
        perror("listen error");
        close(ssd);
        return -1;
    }
    return ssd;
}

void SigExit(int signo) {
    printf("서버를 종료합니다.\n");
    close(rsd);
    close(sd);
    exit(0);
}
