#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 128
#define NAME_SIZE 20
#define NOTSET 0
#define EXIST 1
#define NOTEXIST 2

void *send_msg(void *arg);
void *recv_msg(void *arg);
void error_handling(char *msg);

char name[NAME_SIZE];
char msg[BUF_SIZE];
int cli_exist = NOTSET;
int setFName = 0;
int wOk = 1;

void file_send(int sock);
void file_send_all(int sock);
void menuOptions(int sock);
// pthread_mutex_t mutx;

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread;
    void *thread_return;
    if (argc != 4) {
        printf("Usage : %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    // pthread_mutex_init(&mutx, NULL);
    // pthread_mutex_lock(&mutx);
    // pthread_mutex_unlock(&mutx);
    // \B9\C2\C5ؽ\BA \C0\CC\C1\A6 \C0̿밡\B4\C9

    sprintf(name, "%s", argv[3]);
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    write(sock, name, NAME_SIZE);
    // \A4\A4 \C0̸\A7\C0\BB \BC\AD\B9\F6\C2\CA\C0\B8\B7\CE \BA\B8\B3\BF.

    printf("\n\n");
    printf("CONNECTING..... \n [TIP] If you want \"MENU\" -> /menu \n\n");

    pthread_create(&snd_thread, NULL, send_msg, (void *)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void *)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);
    return 0;
}

void *send_msg(void *arg) // send thread main
{
    int sock = *((int *)arg);
    int Flength = 0;
    int i = 0;
    int fSize = 0;
    int fEnd = 0;
    char name_msg[NAME_SIZE + BUF_SIZE];
    memset(name_msg, '\0', sizeof(name_msg));
    char t_msg[BUF_SIZE];
    memset(t_msg, '\0', sizeof(t_msg));
    char last_msg[BUF_SIZE];
    memset(last_msg, '\0', sizeof(last_msg));
    char t_name_msg[BUF_SIZE];
    memset(t_name_msg, '\0', sizeof(t_name_msg));
    char noUse[BUF_SIZE];
    memset(noUse, '\0', sizeof(noUse));
    const char enter[BUF_SIZE] = {"\n"};
    const char whisper[BUF_SIZE] = {"/whisper\n"};

    // \A4\A4 (\C1\F8\C7\E0\C1\DF)

    while (1) {
        if (wOk == 0) {
            sleep(1);
        }

        fgets(msg, BUF_SIZE, stdin);

        if (!strcmp(msg, "/exit\n")) {
            close(sock);
            exit(0);
        } else if (!strcmp(msg, "/sendfile\n")) {
            file_send(sock);
        } else if (!strcmp(msg, "/sendfile all\n")) {
            file_send_all(sock);
        } // \A4\A4 \C0\FCü\BF\A1\B0\D4 \C6\C4\C0\CF \BA\B8\B3\BE \B6\A7
        else if (!strcmp(msg, "/menu\n")) {
            menuOptions(sock);
            printf("\n");
            printf("[MENU]\n\n");
            printf("1. /menu -> some orders \n");
            printf("2. /whisper -> whispering to someone\n");
            printf("3. /sendfile -> 1:1 file transfer \n");
            printf("4. /sendfile all -> 1:N file transfer \n");
            printf("5. /exit -> chatting program exit \n");
            printf("\n[END MENU] \n\n");

        } // \A4\A4 \B8޴\BA\B8\A6 \BA\B8\BF\A9\C1ִ\C2 \B0\E6\BF\EC
        else if (setFName == 1) {
            if (strcmp(msg, enter)) {
                setFName = 0;
            }
        } // \A4\A4 \C6\C4\C0\CF \BC\F6\BDŽ\C3 \C6\C4\C0\CF \C0̸\A7\C0\BB
          // \BC\B3\C1\A4\C7ϴ\C2 \B0\E6\BF\EC
        else if (!strcmp(msg, whisper)) {
            char who[NAME_SIZE];
            char wmsg[BUF_SIZE];

            memset(t_msg, '\0', sizeof(t_msg));

            printf("(!Record) Who(ID) Message : ");
            scanf("%s %s[^\n]", who, wmsg);

            write(sock, "whisper : cl->sr", BUF_SIZE);
            // \A4\A4 \BA\B8\B3\BD\B4ٴ\C2 \BD\C5ȣ\B8\A6 \B8\D5\C0\FA
            // \BA\B8\B3\BF.

            write(sock, who, NAME_SIZE);
            // \A4\A4 \BA\B8\B3\BB\B0\ED\BD\CD\C0\BA \BB\E7\BF\EB\C0\DA
            // \BE\C6\C0̵\F0\B8\A6 \BA\B8\B3\BF.

            strcpy(t_msg, "\n");
            sprintf(t_name_msg, "[(whispering)%s] %s", name, t_msg);
            sprintf(name_msg, "[(whispering)%s] %s", name, wmsg);

            name_msg[strlen(name_msg)] = '\n';

            if (strcmp(name_msg, t_name_msg) != 0)
                write(sock, name_msg, BUF_SIZE);
            // \A4\A4 \BEƹ\AB\B0͵\B5 \C0Է¹\DE\C1\F6 \BEʾ\D2\C0\BB\B6\A7\B4\C2
            // \C3\E2\B7\C2\C7\CF\C1\F6 \BE\CA\C0\BD \A4\A4 \B8޽\C3\C1\F6
            // \BA\B8\B3\BB\B1\E2

        } else {
            strcpy(t_msg, "\n");
            sprintf(t_name_msg, "[%s] %s", name, t_msg);
            sprintf(name_msg, "[%s] %s", name, msg);

            if (strcmp(name_msg, t_name_msg) != 0)
                write(sock, name_msg, BUF_SIZE);
            // \A4\A4 \BEƹ\AB\B0͵\B5 \C0Է¹\DE\C1\F6 \BEʾ\D2\C0\BB\B6\A7\B4\C2
            // \C3\E2\B7\C2\C7\CF\C1\F6 \BE\CA\C0\BD \A4\A4 \B8޽\C3\C1\F6
            // \BA\B8\B3\BB\B1\E2
        }
    }
    return NULL;
}

void *recv_msg(void *arg) // read thread main
{
    int sock = *((int *)arg);
    char name_msg[BUF_SIZE];
    char file_msg[BUF_SIZE];
    const char signal[BUF_SIZE] = {"file : sr->cl"};
    const char end_msg[BUF_SIZE] = {"FileEnd_sr->cl"};
    const char nocl_msg[BUF_SIZE] = {"[NoClient_sorry]"};
    const char yescl_msg[BUF_SIZE] = {"[continue_ok_nowgo]"};
    const char noConnect[BUF_SIZE] = {"too many users. sorry"};
    int str_len = 0;
    int fSize = 0;

    while (1) {
        str_len = read(sock, name_msg, BUF_SIZE);

        if (!strcmp(name_msg, signal)) {

            setFName = 1;
            wOk = 0;

            printf("(!Notice)receive request. ");

            read(sock, &fSize, sizeof(int));
            printf("(File size : %d Byte)\n [press Enter key to continue]",
                   fSize);
            //\C6\C4\C0\CF \BB\E7\C0\CC\C1\EE \B9޾\C6 \C3\E2\B7\C2\C7ϱ\E2.

            printf("(!Notice)set file name : ");

            wOk = 1;
            while (setFName == 1) {
                sleep(1);
            }

            msg[strlen(msg) - 1] = '\0';

            FILE *fp;
            fp = fopen(msg, "wb");

            while (1) {
                //
                read(sock, file_msg, BUF_SIZE);

                if (!strcmp(file_msg, end_msg))
                    break;
                fwrite(file_msg, 1, BUF_SIZE, fp);
            }

            fclose(fp);

            printf("(!Notice)File receive finished \n");
            // \A4\A4 send_msg \BE\B2\B7\B9\B5\E5\C0\C7 Ȱ\B5\BF \C0簳

        } else if (strcmp(name_msg, yescl_msg) == 0) {

            cli_exist = EXIST;

        } else if (strcmp(name_msg, nocl_msg) == 0) {

            cli_exist = NOTEXIST;
        } else if (!strcmp(name_msg, noConnect)) {
            printf("too many users. sorry \n");
            exit(0);
        } else {
            fputs(name_msg, stdout);
        }
    }
    return NULL;
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

void file_send(int sock) {
    // for - index
    int i;
    // send
    char who[NAME_SIZE];
    // File info
    char location[BUF_SIZE];
    FILE *fp;
    FILE *size;

    int fEnd = 0;
    char noUse[BUF_SIZE];
    memset(noUse, '\0', sizeof(noUse));
    int Flength = 0;
    int fSize = 0;
    char t_msg[BUF_SIZE];
    memset(t_msg, '\0', sizeof(t_msg));
    char last_msg[BUF_SIZE];
    memset(last_msg, '\0', sizeof(last_msg));

    printf("(!Record)File location : ");
    scanf("%s", location);

    size = fopen(location, "rb");
    if (size == NULL) {
        printf("(!Notice)No file like that \n");
        return;
    }
    // \A4\A4 \BA\B8\B3\BE \C6\C4\C0\CF\C0\CC \C0\AFȿ\C7Ѱ\A1 Ȯ\C0\CE

    printf("(!Record)To who(ID)? : ");
    scanf("%s", who);

    // send signal client->server
    write(sock, "file : cl->sr", BUF_SIZE);

    // send name
    write(sock, who, NAME_SIZE);

    while (cli_exist == NOTSET) {
        sleep(1);
    }

    if (cli_exist == NOTEXIST) {
        printf("(!Notice)No user like that \n");
        cli_exist = NOTSET;
        return;
    }

    while (1) {
        fEnd = fread(noUse, 1, BUF_SIZE, size);
        fSize += fEnd;

        if (fEnd != BUF_SIZE)
            break;
    }
    fclose(size);

    printf("(!Notice)File transfer start \n(File Size : %d Byte)\n", fSize);
    // send fils size
    write(sock, &fSize,
          sizeof(int)); // \C6\C4\C0\CF ũ\B1\E2\C1\A4\BA\B8 \B8\D5\C0\FA
                        // \BA\B8\B3\BF.

    fSize = 0;

    fp = fopen(location, "rb");

    while (1) {
        // read file data
        Flength = fread(t_msg, 1, BUF_SIZE, fp);
        // copy data
        if (Flength != BUF_SIZE) {
            for (i = 0; i < Flength; i++) {
                last_msg[i] = t_msg[i];
            }
            //\A4\A4 fread \B4\C2 \C6\C4\C0ϳ\A1\BF\A1 \B4\EA\BE\D2\C0\BB
            //\B6\A7 \C0\CC\C0\FC\C0\C7 \B5\A5\C0\CC\C5Ϳ\CD
            //\C7\D4\C3\C4\C1\FC\C0\CC \C0\D6\C0\BB \BC\F6
            //\C0\D6\C0\B8\B9Ƿ\CE \B9\E6\C1\F6\C7Ͽ\B4\C0\BD.

            write(sock, last_msg, BUF_SIZE);

            write(sock, "FileEnd_cl->sr", BUF_SIZE);
            break;
        }
        // send file data
        write(sock, t_msg, BUF_SIZE);
    }
    // \A4\A4 \BC\AD\B9\F6\BF\A1 \C6\C4\C0\CF\C0\C7 \B3\BB\BF\EB\C0\BB
    // \BA\B8\B3\C0\B4ϴ\D9.

    fclose(fp);
    printf("(!Notice)File transfer finish \n");
    cli_exist = NOTSET;
}
void file_send_all(int sock) {
    int i;
    char location[BUF_SIZE];
    char who[NAME_SIZE];
    FILE *fp;
    FILE *size;
    int fEnd = 0;
    char noUse[BUF_SIZE];
    memset(noUse, '\0', sizeof(noUse));
    int Flength = 0;
    int fSize = 0;
    char t_msg[BUF_SIZE];
    memset(t_msg, '\0', sizeof(t_msg));
    char last_msg[BUF_SIZE];
    memset(last_msg, '\0', sizeof(last_msg));

    printf("(!Record)File location : ");
    scanf("%s", location);

    size = fopen(location, "rb");
    if (size == NULL) {
        printf("(!Notice)No file like that \n");
        return;
    }
    // \A4\A4 \BA\B8\B3\BE \C6\C4\C0\CF\C0\CC \C0\AFȿ\C7Ѱ\A1 Ȯ\C0\CE

    write(sock, "file : cl->sr_all", BUF_SIZE);
    // \A4\A4 \B8\D5\C0\FA \C6\C4\C0\CF\C0\BB \BA\B8\B3\BD\B4ٴ\C2
    // \BD\C5ȣ\B8\A6 \BC\AD\B9\F6\C2ʿ\A1 \BA\B8\B3\BF.

    while (1) {
        fEnd = fread(noUse, 1, BUF_SIZE, size);
        fSize += fEnd;

        if (fEnd != BUF_SIZE)
            break;
    }
    fclose(size);

    printf("(!Notice)File transfer start \n(File Size : %d Byte)\n", fSize);
    write(sock, &fSize,
          sizeof(int)); // \C6\C4\C0\CF ũ\B1\E2\C1\A4\BA\B8 \B8\D5\C0\FA
                        // \BA\B8\B3\BF.
    fSize = 0;

    fp = fopen(location, "rb");

    while (1) {
        // copy fread
        Flength = fread(t_msg, 1, BUF_SIZE, fp);

        if (Flength != BUF_SIZE) {
            for (i = 0; i < Flength; i++) {
                last_msg[i] = t_msg[i];
            }
            //\A4\A4 fread \B4\C2 \C6\C4\C0ϳ\A1\BF\A1 \B4\EA\BE\D2\C0\BB
            //\B6\A7 \C0\CC\C0\FC\C0\C7 \B5\A5\C0\CC\C5Ϳ\CD
            //\C7\D4\C3\C4\C1\FC\C0\CC \C0\D6\C0\BB \BC\F6
            //\C0\D6\C0\B8\B9Ƿ\CE \B9\E6\C1\F6\C7Ͽ\B4\C0\BD.

            write(sock, last_msg, BUF_SIZE);

            write(sock, "FileEnd_cl->sr", BUF_SIZE);
            break;
        }
        write(sock, t_msg, BUF_SIZE);
    }
    // \A4\A4 \BC\AD\B9\F6\BF\A1 \C6\C4\C0\CF\C0\C7 \B3\BB\BF\EB\C0\BB
    // \BA\B8\B3\C0\B4ϴ\D9.

    fclose(fp);
    printf("(!Notice)File transfer finish \n");
} // \A4\A4 \C0\FCü\BF\A1\B0\D4 \C6\C4\C0\CF \BA\B8\B3\BE \B6\A7
void menuOptions(int sock) {
    printf("\n");
    printf("[MENU]\n\n");
    printf("1. /menu -> some orders \n");
    printf("2. /whisper -> whispering to someone\n");
    printf("3. /sendfile -> 1:1 file transfer \n");
    printf("4. /sendfile all -> 1:N file transfer \n");
    printf("5. /exit -> chatting program exit \n");
    printf("\n[END MENU] \n\n");
}
