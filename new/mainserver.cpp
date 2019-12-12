#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LINE 511
#define MAX_BUF 100
#define NAME_LEN 20
#define COMMAND 5
#define MAX_USER_NUM 5

typedef struct _user {
    int userSock;
    int userIdx;
    int servSock;
    char nick[NAME_LEN];
} User;

User list[MAX_USER_NUM];

int sd;

int useridx = 0;
int userSocks[MAX_USER_NUM];
pthread_mutex_t usermutex;

void *tcp_listen(void *port);
void *mainThread(void *us);
void sigExit(int signum);

int main(int argc, char *argv[]) {
    pthread_t ptr; //
    int port;

    if (argc != 2) {
        printf("usage: %s <port>\n", argv[0]);
    }
    port = atoi(argv[1]);

    signal(SIGINT, sigExit);

    pthread_mutex_init(&usermutex, NULL);

    pthread_create(&ptr, NULL, tcp_listen, &port);
    pthread_detach(ptr);

    while (1) {
        pause();
    }

    return 0;
}

void *tcp_listen(void *port) {
    User user;
    int servPort = *(int *)port;

    struct sockaddr_in servaddr;
    int sd;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("socket fail");
        exit(1);
    }

    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(servPort);

    if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind fail");
        exit(1);
    }
    if (listen(sd, 5) == -1) {
        perror("listen fail");
        close(sd);
    }

    struct sockaddr_in client = {0};
    int clen = sizeof(client);
    pthread_t ptr;

    printf("server open\n");
    while ((user.userSock =
                accept(sd, (struct sockaddr *)&client, (socklen_t *)&clen))) {
        if (user.userSock == -1) {
            perror("accept fail");
            return 0;
        }

        pthread_mutex_lock(&usermutex);

        user.userIdx = useridx;
        user.servSock = sd;
        userSocks[useridx] = user.userSock;
        list[useridx] = user;
        pthread_create(&ptr, NULL, mainThread, &user);
        pthread_detach(ptr);
        useridx++;
        pthread_mutex_unlock(&usermutex);
    }

    return 0;
}

void *mainThread(void *us) {
    User user = *(User *)us;
    struct stat obj;
    char name[NAME_LEN] = "";
    char *userdir;
    char buf[MAX_BUF], command[COMMAND], fileName[NAME_LEN];
    char lsbuf[MAX_BUF], filelist[MAX_BUF];
    int k, i, size, c, dir;
    int filehandle;

    recv(user.userSock, name, NAME_LEN, 0);
    printf("%s Connect\n", name);
    strcpy(user.nick, name);
    userdir = (char *)malloc(strlen(name) + 2);
    sprintf(userdir, "./%s", name);

    dir = mkdir(userdir, 0777);
    if (dir == -1) {
        printf("%s is original user\n", user.nick);
    }
    dir = mkdir("./res", 0777);

    sprintf(filelist, "./res/filelist_%s.txt", name);
    sprintf(lsbuf, "touch %s ", filelist);
    system(lsbuf);
    chmod(filelist, 0666);
    system("touch ./res/log.txt");
    chmod("./res/log.txt", 0666);
    chdir(userdir);
    sprintf(filelist, "../res/filelist_%s.txt", name);
    sprintf(lsbuf, "ls > %s", filelist);

    while (1) {
        recv(user.userSock, buf, MAX_BUF, 0);
        sscanf(buf, "%s", command);
        if (!strcmp(command, "ls")) {
            system(lsbuf);
            stat(filelist, &obj);
            size = obj.st_size;
            send(user.userSock, &size, sizeof(int), 0);
            filehandle = open(filelist, O_RDONLY);
            sendfile(user.userSock, filehandle, NULL, size);

        } else if (!strcmp(command, "get")) {
            sscanf(buf, "%s%s", fileName, fileName);
            stat(fileName, &obj);
            filehandle = open(fileName, O_RDONLY);
            size = obj.st_size;
            if (filehandle == -1)
                size = 0;
            send(user.userSock, &size, sizeof(int), 0);
            if (size)
                sendfile(user.userSock, filehandle, NULL, size);
        } else if (!strcmp(command, "put")) {
            int c = 0, len;
            char *f;
            sscanf(buf + strlen(command), "%s", fileName);
            recv(user.userSock, &size, sizeof(int), 0);

            while (1) {
                filehandle = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0666);
                if (filehandle == -1)
                    sprintf(fileName + strlen(fileName), "_1");
                else
                    break;
            }
            f = (char *)malloc(size);
            recv(user.userSock, f, size, 0);
            c = write(filehandle, f, size);
            close(filehandle);
            send(user.userSock, &c, sizeof(int), 0);
        } else if (!strcmp(command, "cd")) {
            if (chdir(buf + 3) == 0)
                c = 1;
            else
                c = 0;
            send(user.userSock, &c, sizeof(int), 0);
        } else if (!strcmp(command, "quit")) {
            printf("%s Quit..\n", user.nick);
            i = 1;
            send(user.userSock, &i, sizeof(int), 0);
            break;
        }
    }
    free(userdir);
    return 0;
}

void sigExit(int signum) {
    printf("close Server\n");
    close(sd);
    exit(0);
}
