#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
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
int rcnt = 0;
int usernum = 0;
int rusernum = 0;
int SERV_PORT;
pthread_mutex_t usermutex, roommutex;

void sigExit(int signum);
int serverSetting(char *ip, int port);
void *joinChat(void *user);
void *deliveryMessage(void *user);
void *notice(void *user);
void whisper(User user, char *rbuf);
void clientExit(User user);
void *mainThread(void *room);
void fileIO(User user, char *rbuf);

int main(int argc, char *argv[]) {
    int room, i;

    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
        exit(0);
    }

    SERV_PORT = atoi(argv[1]);

    signal(SIGINT, sigExit);

    pthread_mutex_init(&usermutex, NULL);
    pthread_mutex_init(&roommutex, NULL);

    printf("Input Room Number: ");
    scanf("%d", &room);
    printf("error before!\n");
    getchar();

    pthread_t thread[256];

    for (i = 0; i < room; i++) {
        pthread_create(&thread[i], NULL, mainThread, &room);
        pthread_detach(thread[i]);
    }
    printf("after\n");
    while (1)
        pause();

    return 0;
}

void *mainThread(void *room) {
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
    servaddr.sin_addr.s_addr = inet_addr("10.10.21.233");

    if (bind(ssd, (struct sockaddr *)&servaddr, sizeof(servaddr))) {
        perror("bind error!");
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

    while ((user.usd =
                accept(ssd, (struct sockaddr *)&cliaddr, (socklen_t *)&clen))) {
        if (user.usd == -1) {
            perror("accept error");
            return 0;
        }

        pthread_mutex_lock(&usermutex);

        user.unum = usernum;

        user.rsd = ssd;
        rm.rsd = ssd;
        rm.usd[rm.ucnt] == user.usd;

        rlist[rm.ucnt] = rm;
        pthread_create(&ptr, NULL, joinChat, &user);
        pthread_detach(ptr);

        usernum++;
        rm.ucnt++;

        pthread_mutex_unlock(&usermutex);
    }

    return 0;
}

void *joinChat(void *user) {
    User us = *(User *)user;
    char nick[MAX_NICK_LEN] = "";
    pthread_t ptr[2];

    send(us.usd, "Welcome Catting Program.\nInput your nickname.\n", 100, 0);
    recv(us.usd, nick, sizeof(nick), 0);
    nick[strlen(nick) - 1] = '\0';
    printf("%s Connect\n", nick);

    strcpy(us.unick, nick);

    list[us.unum] = us;

    pthread_create(&ptr[0], NULL, deliveryMessage, &us);
    pthread_create(&ptr[1], NULL, notice, &us);
    pthread_join(ptr[0], NULL);
    pthread_join(ptr[1], NULL);
}

void *deliveryMessage(void *user) {
    User us = *(User *)user;
    int i;
    char *wmsg;
    char rbuf[MAX_MESSAGE_LEN];
    char sbuf[MAX_MESSAGE_LEN];

    while (recv(us.usd, rbuf, sizeof(rbuf), 0) > 0) {
        if (!strncmp(rbuf, "/w", 2)) {
            whisper(us, rbuf);
            continue;
        } else if (!strncmp(rbuf, "/f", 2)) {
            fileIO(us, rbuf);
            continue;
        } else {
            sprintf(sbuf, "%s : %s", us.unick, rbuf);
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
}

void fileIO(User user, char *rbuf) {
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

void clientExit(User user) {
    int i, j;
    char sbuf[MAX_MESSAGE_LEN];

    sprintf(sbuf, "%s exit.\n", user.unick);

    for (i = 0; i < usernum; i++) {
        if (list[i].usd == user.usd) {
            if (list[j].usd == user.usd)
                continue;
            if (list[j].rsd == user.rsd)
                send(list[j].usd, sbuf, sizeof(sbuf), 0);
        }
        break;
    }

    pthread_mutex_lock(&usermutex);
    for (j = i; j < usernum - 1; j++) {
        list[j] = list[j + 1];
    }
    usernum--;
    pthread_mutex_unlock(&usermutex);
}

void whisper(User user, char *rbuf) {
    int i;
    char *whisp;
    char sbuf[MAX_MESSAGE_LEN];

    strtok(rbuf, " ");
    whisp = strtok(NULL, " ");

    for (i = 0; i < usernum; i++) {
        if (!strcmp(whisp, list[i].unick)) {
            whisp = strtok(NULL, " ");
            sprintf(sbuf, "%s whisper: %s", user.unick, whisp);
            if (list[i].rsd == user.rsd)
                send(list[i].usd, sbuf, strlen(sbuf), 0);
            break;
        }
    }

    if (i == usernum)
        send(user.usd, "no user.\n", 30, 0);
}

void *notice(void *user) {
    int i;
    User us = *(User *)user;
    char sbuf[MAX_MESSAGE_LEN];
    char inbuf[MAX_MESSAGE_LEN];

    sprintf(sbuf, "%s come.\n", us.unick);
    for (i = 0; i < usernum; i++) {
        if (list[i].rsd == us.rsd)
            send(list[i].usd, sbuf, strlen(sbuf), 0);
    }

    while (1) {
        memset(sbuf, 0, sizeof(sbuf));
        memset(inbuf, 0, sizeof(inbuf));
        fgets(inbuf, sizeof(inbuf), stdin);
        sprintf(sbuf, "[Notice]: %s\n", inbuf);
        for (i = 0; i < usernum; i++)
            send(list[i].usd, sbuf, strlen(sbuf), 0);
    }
}

int serverSetting(char *ip, int port) {
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

void sigExit(int signum) {
    printf("close server\n");
    close(rsd);
    close(sd);
    exit(0);
}
