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

int tcp_listen(int host, int port, int backlog) {
    int sd;
    struct sockaddr_in servaddr;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("socket fail");
        exit(1);
    }

    bzero((char *)&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(host);
    servaddr.sin_port = htons(port);

    if (bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind fail");
        exit(1);
    }
    listen(sd, backlog);
    return sd;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server, client;
    struct stat obj;
    int servsock, clisock;
    char buf[MAX_BUF], command[COMMAND], fileName[NAME_LEN];
    int k, i, size, len, c;
    int filehandle;

    servsock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);

    len = sizeof(client);
    clisock = accept(servsock, (struct sockaddr *)&client, (socklen_t *)&len);
    while (1) {
        recv(clisock, buf, MAX_BUF, 0);
        sscanf(buf, "%s", command);
        if (!strcmp(command, "ls")) {
            system("ls > temps.txt");
            stat("temps.txt", &obj);
            size = obj.st_size;
            send(clisock, &size, sizeof(int), 0);
            filehandle = open("temps.txt", O_RDONLY);
            sendfile(clisock, filehandle, NULL, size);
        } else if (!strcmp(command, "get")) {
            sscanf(buf, "%s%s", fileName, fileName);
            stat(fileName, &obj);
            filehandle = open(fileName, O_RDONLY);
            size = obj.st_size;
            if (filehandle == -1)
                size = 0;
            send(clisock, &size, sizeof(int), 0);
            if (size)
                sendfile(clisock, filehandle, NULL, size);
        } else if (!strcmp(command, "put")) {
            int c = 0, len;
            char *f;
            sscanf(buf + strlen(command), "%s", fileName);
            recv(clisock, &size, sizeof(int), 0);

            while (1) {
                filehandle = open(fileName, O_CREAT | O_EXCL | O_WRONLY, 0666);
                if (filehandle == -1)
                    sprintf(fileName + strlen(fileName), "_1");
                else
                    break;
            }
            f = (char *)malloc(size);
            recv(clisock, f, size, 0);
            c = write(filehandle, f, size);
            close(filehandle);
            send(clisock, &c, sizeof(int), 0);
        } else if (!strcmp(command, "pwd")) {
            system("pwd>temp.xt");
            i = 0;
            FILE *f = fopen("temp.txt", "r");
            while (!feof(f))
                buf[i++] = fgetc(f);
            buf[i - 1] = '\0';
            fclose(f);
            send(clisock, buf, MAX_BUF, 0);
        } else if (!strcmp(command, "cd")) {
            if (chdir(buf + 3) == 0)
                c = 1;
            else
                c = 0;
            send(clisock, &c, sizeof(int), 0);
        } else if (!strcmp(command, "bye") || !strcmp(command, "quit")) {
            printf("FTP server quitting..\n");
            i = 1;
            send(clisock, &i, sizeof(int), 0);
            exit(0);
        }
    }
    return 0;
}
