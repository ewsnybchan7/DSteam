/**    chat_client **/

#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUF_SIZE 100
#define NORMAL_SIZE 20
#define MAXLINE 127
#define NAME_SIZE 100
void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(const char *msg);

void menu();
void changeName();
void menuOptions(int sock);
void send_file(int sock); // signal_handler function
void recive_file(int sock);

char name[NORMAL_SIZE] = "[DEFALT]"; // name
char msg_form[NORMAL_SIZE];          // msg form
char serv_time[NORMAL_SIZE];         // server time
char msg[BUF_SIZE];                  // msg
char serv_port[NORMAL_SIZE];         // server port number
char clnt_ip[NORMAL_SIZE];           // client ip address
char file_name[100];                 // send of file's name
char user_name[100];                 // send of file recive user
int file_size;                       // files size

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;
    if (argc != 4) {
        printf(" Usage : %s <ip> <port> <name>\n", argv[0]);
        exit(1);
    }

    /** local time **/
    struct tm *t;
    time_t timer = time(NULL);
    t = localtime(&timer);
    sprintf(serv_time, "%d-%d-%d %d:%d", t->tm_year + 1900, t->tm_mon + 1,
            t->tm_mday, t->tm_hour, t->tm_min);

    sprintf(name, "[%s]", argv[3]);
    sprintf(clnt_ip, "%s", argv[1]);
    sprintf(serv_port, "%s", argv[2]);
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("conncet() error");
    }

    /** call menu **/
    menu();

    pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}

void *send_msg(void *arg) {
    int sock = *((int *)arg);
    char name_msg[NORMAL_SIZE + BUF_SIZE];
    char myInfo[BUF_SIZE];
    char *who = NULL;
    char temp[BUF_SIZE];

    /** send join messge **/
    printf(" >> join the chat !! \n");
    sprintf(myInfo, "%s's join. IP_%s\n", name, clnt_ip);
    write(sock, myInfo, strlen(myInfo));

    while (1) {
        printf(">>");
        fgets(msg, BUF_SIZE, stdin);

        // menu_mode command -> !menu
        if (!strcmp(msg, "!menu\n")) {
            menuOptions(sock);
            continue;
        }

        else if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
            close(sock);
            exit(0);
        }

        // send message
        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));
    }
    return NULL;
}

void *recv_msg(void *arg) {
    int sock = *((int *)arg);
    char name_msg[NORMAL_SIZE + BUF_SIZE];
    int str_len;

    while (1) {
        str_len = read(sock, name_msg, NORMAL_SIZE + BUF_SIZE - 1);
        if (str_len == -1)
            return (void *)-1;
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

void menuOptions(int sock) {
    char select;
    // print menu
    printf("\n\t**** menu mode ****\n");
    printf("\t1. change name\n");
    printf("\t2. clear/update\n\n");
    printf("\t3. send file\n");
    printf("\tthe other key is cancel");
    printf("\n\t*******************");
    printf("\n\t>> ");
    scanf("%c", &select);
    getchar();
    switch (select) {
    // change user name
    case '1':
        changeName();
        break;

    // console update(time, clear chatting log)
    case '2':
        menu();
        break;
    // send file
    case '3':
        send_file(sock);
        break;

    // menu error
    default:
        printf("\tcancel.");
        break;
    }
}

/** change user name **/
void changeName() {
    char nameTemp[100];
    printf("\n\tInput new name -> ");
    scanf("%s", nameTemp);
    sprintf(name, "[%s]", nameTemp);
    printf("\n\tComplete.\n\n");
}

void menu() {
    system("clear");
    printf(" **** moon/sum chatting client ****\n");
    printf(" server port : %s \n", serv_port);
    printf(" client IP   : %s \n", clnt_ip);
    printf(" chat name   : %s \n", name);
    printf(" server time : %s \n", serv_time);
    printf(" ************* menu ***************\n");
    printf(" if you want to select menu -> !menu\n");
    printf(" 1. change name\n");
    printf(" 2. clear/update\n");
    printf(" 3. Send File <User name> <File name>\n");
    printf(" **********************************\n");
    printf(" Exit -> q & Q\n\n");
}

void error_handling(const char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
void send_file(int sock) {
    int fp;
    int total = 0;
    int sread;
    char buf[MAXLINE + 1];
    char recv_msg[MAXLINE + 1];
    printf("Enter the File name\n");
    scanf("%s", file_name);
    if ((fp = open(file_name, O_RDONLY)) < 0) {
        printf("open failed\n");
        return;
    }
    int file_name_size = strlen(file_name);
    file_name[file_name_size - 1] = 0;
    // find fail files

    if (fgets(user_name, sizeof(user_name), stdin) == NULL) {
        printf("Input error");
        return;
    }
    write(sock, "file : cl->sr", BUF_SIZE);
    write(sock, user_name, sizeof(user_name));
    // send file name
    write(sock, file_name, sizeof(file_name));
    file_size = lseek(fp, 0, SEEK_END);

    // send file size
    write(sock, &file_size, sizeof(file_size));
    lseek(fp, 0, SEEK_SET);
    // send file
    while (total != file_size) {
        sread = read(fp, buf, 100);
        // printf("\rfile is sending now.. \n");
        total += sread;
        buf[sread] = 0;
        send(sock, buf, sread, 0);
        printf("\rprocessing :%4.2f%% ", total * 100 / (float)file_size);
        usleep(1000);
    }
    printf("\nsending End\n");
}
void revice_flie(int sock) {}
