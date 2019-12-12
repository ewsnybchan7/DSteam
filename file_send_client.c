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
#define BUF_SIZE 128

//#define SERV_PORT 10200

// user 구조체
typedef struct _user {
    int usd;  // socketnum
    int unum; // usernumber
    int rsd;  //
    char unick[MAX_NICK_LEN];
} User;

// room 구조체
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
int port;
pthread_mutex_t usermutex;

void SigExit(int signo);
int SockSetting(char *ip, int port);
void JoinServer(int ssd);
void *RecvFile(void *user);
void *SendFile(void *user);
void *thrdmain(void *us);

int main(int argc, const char *argv[]) {
    char pt[MAX_MESSAGE_LEN];
    char IP[MAX_MESSAGE_LEN];

    if (argc != 2) {
        printf("Usage: %s <ip>\n", argv[0]);
        exit(0);
    }
    printf("포트를 입력하세요 :");
    scanf("%d", &port);
    strcpy(IP, argv[1]);
    getchar();
    signal(SIGINT, SigExit);
    pthread_mutex_init(&usermutex, NULL);
    if ((sd = SockSetting(IP, port)) == -1) {
        perror("socker");
        return 0;
    }

    JoinServer(sd);

    return 0;
}

// ssd = 소켓
void JoinServer(int ssd) {
    User user;
    pthread_t ptr[2];
    char nick[MAX_NICK_LEN] = "";
    char rbuf[MAX_MESSAGE_LEN] = ""; // recive buffer

    recv(ssd, rbuf, 100, 0);
    fputs(rbuf, stdout);

    //닉네임
    fgets(nick, sizeof(nick), stdin);
    send(ssd, nick, strlen(nick), 0);

    user.usd = ssd;
    strcmp(user.unick, nick);

    pthread_create(&ptr[0], NULL, SendFile, &user);
    pthread_detach(ptr[0]);
    pthread_create(&ptr[1], NULL, RecvFile, &user);
    pthread_detach(ptr[1]);
    while (1)
        pause();
}

void *RecvFile(void *user) {
    User us = *(User *)user;
    char rbuf[MAX_MESSAGE_LEN];
    FILE *fp;
    // us.usd ==socket
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

void *SendFile(void *user) {
    User us = *(User *)user;
    char sbuf[MAX_MESSAGE_LEN];
    char file_name[MAX_MESSAGE_LEN];
    char noUse[MAX_MESSAGE_LEN];
    char file_msg[BUF_SIZE] = {
        '\0',
    };
    int file_size;

    int fEnd = 0;
    int fSize = 0;
    int Flength = 0;
    int fd;
    FILE *fp;
    FILE *size;
    FILE *writefp;
    FILE *readfp;
    int i = 0;
    char *buf;
    char last_msg[BUF_SIZE] = {
        '\0',
    };

    while (1) {
        fgets(sbuf, sizeof(sbuf), stdin);
        send(us.usd, sbuf, sizeof(sbuf), 0);

        if (!strcmp(sbuf, "#list")) {
            send(us.usd, "##list##", sizeof("##list##"), 0);
        } else if (!strncmp(sbuf, "/f", 2)) {
            strtok(sbuf, " ");
            buf = strtok(NULL, " ");
            buf = strtok(buf, "\n");

            fd = open(buf, O_RDONLY);
            if (size == NULL) {
                printf("에러 파일 크기가 NULL입니다.\n");
                continue;
            }

            readfp = fdopen(fd, "r");
            writefp = fdopen(sd, "w");

            while (1) {
                i++;
                fread(noUse, 1, MAX_MESSAGE_LEN, readfp);
                fwrite(noUse, 1, MAX_MESSAGE_LEN, writefp);

                if (fEnd != BUF_SIZE)
                    break;
            }
            fclose(size);

            printf("파일전송시작\n파일 크기: %d Byte", fSize);
            send(us.usd, &fSize, sizeof(int), 0);

            fSize = 0;

            fp = fopen(sbuf, "rb");
            while (1) {
                Flength = fread(file_msg, 1, MAX_MESSAGE_LEN, fp);
                printf("!!!\n");
                if (Flength != BUF_SIZE) {
                    for (i = 0; i < Flength; i++) {
                        last_msg[i] = file_msg[i];
                    }
                    send(us.usd, last_msg, BUF_SIZE, 0);
                    send(us.usd, "end", , 0);
                    break;
                }
                send(us.usd, file_msg, BUF_SIZE, 0);
            }
            fclose(fp);
            printf("파일전송 종료\n");
        } else if (!strcmp(sbuf, "#recv")) {
            send(us.usd, "##request_file##", BUF_SIZE, 0);
            printf("파일 번호입력\n");
            fgets(sbuf, sizeof(sbuf), stdin);
            send(us.usd, sbuf, BUF_SIZE, 0);
            memset(sbuf, 0, sizeof(sbuf));
        }

        memset(sbuf, 0, sizeof(sbuf));
    }
}

//소켓 info정리
int SockSetting(char *ip, int port) {
    int ssd;
    if ((ssd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

        perror("소켓 오류");

        return -1;
    }

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    if (connect(ssd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("연결 오류");
        return -1;
    }
    return ssd;
}

void SigExit(int signo) {
    printf("클라이언트를 종료합니다.\n");
    close(sd);
    exit(0);
}
