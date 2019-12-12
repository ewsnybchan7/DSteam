#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
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

int tcp_connect(int af, char *servip, unsigned short port) {
    struct sockaddr_in servaddr;
    int sd;
    // make socket
    if ((sd = socket(af, SOCK_STREAM, 0)) < 0)
        return -1;
    // servaddr init
    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = af;
    inet_pton(AF_INET, servip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);

    // call connect
    if (connect(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        return -1;

    return sd;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server;
    struct stat obj;
    int sock;
    char bufmsg[MAX_LINE];
    char buf[MAX_BUF], command[COMMAND], fileName[MAX_LINE], *f;
    char nick[NAME_LEN];
    char temp[20];
    int k, size, status;
    int filehandle;

    if (argc != 3) {
        printf("usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    sock = tcp_connect(AF_INET, argv[1], atoi(argv[2]));
    if (sock == -1) {
        printf("tcp_connect fail");
        exit(1);
    }

    printf("\033[1;33mInput Name > ");
    scanf("%s", nick);

    while (1) {
        printf("\033[1;33mCommand : get, put pwd, ls, cd, quit\n");
        printf("\033[1;32m%s > ", nick);
        fgets(bufmsg, MAX_LINE, stdin);
        fprintf(stderr, "\033[97m");
        if (!strcmp(bufmsg, "get\n")) {
            printf("Select Download File: ");
            scanf("%s", fileName);
            fgets(temp, MAX_LINE, stdin);
            strcpy(buf, "get ");
            strcat(buf, fileName);
            send(sock, buf, 100, 0);
            recv(sock, &size, sizeof(int), 0);
            if (!size) {
                printf("No file!\n");
                continue;
            }
            f = (char *)malloc(size);
            recv(sock, f, size, 0);
            while (1) {
                filehandle = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0666);
                if (filehandle == -1)
                    sprintf(fileName + strlen(fileName), "_1");
                else
                    break;
            }
            write(filehandle, f, size);
            close(filehandle);
            printf("Download complete\n");
        } else if (!strcmp(bufmsg, "put\n")) {
            printf("Select Upload File: ");
            scanf("%s", fileName);
            fgets(temp, MAX_LINE, stdin);
            filehandle = open(fileName, O_RDONLY);
            if (filehandle == -1) {
                printf("No file!\n");
                continue;
            }
            strcpy(buf, "put ");
            strcat(buf, fileName);
            send(sock, buf, 100, 0);
            stat(fileName, &obj);
            size = obj.st_size;
            send(sock, &size, sizeof(int), 0);
            sendfile(sock, filehandle, NULL, size);
            recv(sock, &status, sizeof(int), 0);
            if (status)
                printf("Upload Complete\n");
            else
                printf("Upload Fail\n");
        } else if (!strcmp(bufmsg, "pwd\n")) {
            strcpy(buf, "pwd");
            send(sock, buf, 100, 0);
            recv(sock, buf, 100, 0);
            printf("---The path of the Remote Directory---\n%s", buf);
        } else if (!strcmp(bufmsg, "ls\n")) {
            strcpy(buf, "ls");
            send(sock, buf, 100, 0);
            recv(sock, &size, sizeof(int), 0);
            f = (char *)malloc(size);
            recv(sock, f, size, 0);
            filehandle = creat("temp.txt", O_WRONLY);
            write(filehandle, f, size);
            close(filehandle);
            printf("---The Remote Directory List---\n");
            system("cat temp.txt");
        } else if (!strcmp(bufmsg, "cd\n")) {
            strcpy(buf, "cd ");
            printf("Input Move Path: ");
            scanf("%s", buf + 3);
            fgets(temp, MAX_LINE, stdin);
            send(sock, buf, 100, 0);
            recv(sock, &status, sizeof(int), 0);
            if (status)
                printf("Complete cd\n");
            else
                printf("Fail cd\n");
        } else if (!strcmp(bufmsg, "quit\n")) {
            strcpy(buf, "quit");
            send(sock, buf, 100, 0);
            recv(sock, &status, 100, 0);
            if (status) {
                printf("Disconnect Client..\n");
                exit(0);
            }
            printf("Fail Disconnect\n");
        }
    }
}
