#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LEN 128

int user_y = 0;
int user_num = 0;
char pathname[MAX_LEN] = ".//";
typedef struct _user {
    char name[MAX_LEN];
} User;
User user[10]; // User infomation
// cursor point
void move_cur(int x, int y) {
    printf("\033[%dd\033[%dG", y, x);
    fflush(stdout);
}

// clear line
void erase(int x, int y, int length) {
    int i;
    move_cur(x, y);
    for (i = 0; i < length; i++)
        printf(" ");
    move_cur(x, y);
}
// notice line cursor
void notice_cur() {
    printf("\033[%dd\033[%dG", 2, 18);
    fflush(stdout);
}
// file name line cursor
void filename_cur(User us) {
    int i, j = 0, count = 0;
    for (i = 0; i < MAX_LEN; i++) {
        if (count - 4 == 0) {
            count = 0;
            j += 2;
        }
        printf("\033[%dd\033[%dG", 6 + j, 18 + 12 * count);
        fflush(stdout);
        printf("%c[1;34m", 27);
        count++;
    }
    printf("%c[0m", 27);
}
// name line cursor
void name_cur(int length, char *name) {
    int i = 0;
    for (i = 0; i < length; i++) {
        printf("\033[%dd\033[%dG", 2, 6 + 2 * i);
        fflush(stdout);
        printf("%s", name);
    }
}
// user line cursor
void user_cur(char *nick_name) {
    printf("\033[%dd\033[%dG", 6 + user_y, 2);
    fflush(stdout);
    printf("%c[1;32m", 27);
    printf("%s", nick_name);
    printf("%c[0m", 27);
    user_y = user_y + 2;
}
// using user action
void use_search(User us) {}
// input line point
void input_cur() {
    printf("\033[%dd\033[%dG", 23, 18);
    fflush(stdout);
}

void input_notice(); // input notice
int input_name();    // input name
char *input();       // input don't used
// back ground bord
void bg_bord() {
    system("clear");
    int i, j;
    int m[24][40] = {
        3, 8, 8, 8, 8, 8, 8, 8,  9,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8,
        8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8, 8, 8, 8, 8, 8, 2, 1, 0, 0,  0,  0, 0,
        0, 0, 1, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 1, 4, 8, 8, 8, 8, 8, 8, 8, 12, 8,  8, 8,
        8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8,
        8, 8, 8, 8, 5, 1, 0, 0,  0,  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  1, 4,
        8, 8, 8, 8, 8, 8, 8, 12, 8,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8,
        8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8, 8, 8, 8, 8, 5, 1, 0, 0, 0,  0,  0, 0,
        0, 1, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 1, 1, 0, 0, 0,  0,  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1,  1, 0,
        0, 0, 0, 0, 0, 0, 1, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,  0,  0, 0,
        1, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  1,  1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 1, 1, 0, 0, 0, 0,  0,  0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  1,  0, 0,
        0, 0, 0, 0, 0, 1, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0,  0,  0, 1,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 1,  1,  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 1, 1, 0, 0, 0, 0, 0,  0,  0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1,  0,  0, 0,
        0, 0, 0, 0, 1, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0,  0,  1, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 1, 1,  0,  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        1, 1, 0, 0, 0, 0, 0, 0,  0,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0,  0,  0, 0,
        0, 0, 0, 1, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 1, 4, 8, 8, 8, 8, 8, 8, 8,  12, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8,
        8, 8, 8, 8, 8, 5, 1, 0,  0,  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,  0,  0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0, 1,
        6, 8, 8, 8, 8, 8, 8, 8,  10, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8,
        8, 8, 8, 8, 8, 8, 8, 8,  8,  8, 8, 8, 8, 8, 8, 8, 7};

    for (i = 0; i < 24; i++) {
        for (j = 0; j < 40; j++) {

            switch (m[i][j]) {
            case 0:
                printf("  ");
                break;
            case 1:
                printf("│");
                break;
            case 2:
                printf("┐");
                break;
            case 3:
                printf("┌");
                break;
            case 4:
                printf("├");
                break;
            case 5:
                printf("┤");
                break;
            case 8:
                printf("──");
                break;
            case 6:
                printf("└");
                break;
            case 7:
                printf("┘");
                break;
            case 9:
                printf("┬");
                break;
            case 10:
                printf("┴");
                break;
            case 11:
                printf("─");
                break;
            case 12:
                printf("┼");
                break;
            }
        }
        printf("\n");
    }

    // NOTICE color
    move_cur(2, 2);
    printf("%c[1;33m", 27);
    printf("NOTICE");
    printf("%c[0m", 27);
    move_cur(2, 4);

    // INFO COMMEND
    printf("%c[1;35m", 27);
    printf("COMMEND");
    printf("%c[0m", 27);
    // move_cur(2, 6);
    // printf("NICKNAME");
    // move_cur(2, 8);
    // printf("NICKNAME");
    // move_cur(2, 10);
    // printf("NICKNAME");

    move_cur(18, 4);
    printf("%c[1;35m", 27);
    printf("quit    ls    put    get    cd");
    printf("%c[0m", 27);
    // file name point
    // move_cur(18, 6);
    // printf("file name");
    // move_cur(30, 6);
    // printf("file name");
    // move_cur(42, 6);
    // printf("file name");
    // move_cur(54, 6);
    // printf("file name");
    // move_cur(66, 6);
    // printf("file name");
}

int main() {
    // notic cur (18,2)
    int i = 0, j = 0;
    int count = 0;
    system("clear");
    bg_bord();

    // file 5개씩 넣을수 있음
    // 18,30,42,54,66
    // x는 12씩 증가
    // file이 5개일때마다 y증가
    // y는 2씩 증가

    // example using input name and notice
    //   move_cur(18, 6);
    //    system("ls -1");
    i = input_name();
    i = input_name();
    i = input_name();

    notice_cur();
    input_notice();

    move_cur(2, 23);
    // client commend cur (18,23)
    // end cursor don't break tool
    move_cur(1, 26);
}

void input_notice() {
    // eraseing notice writing
    erase(18, 2, 50);

    printf("%c[1;33m", 27);
    printf("ENTER THE NOTICE");
    printf("%c[0m", 27);

    char msg[MAX_LEN];

    input_cur();
    scanf("%s", msg);
    move_cur(18, 2);
    erase(18, 2, 20);
    printf("%c[1;33m", 27);
    printf("%s", msg);
    printf("%c[0m", 27);

    input_cur();
}
int input_name() {
    // notice line writing
    int i = 0;
    notice_cur();
    printf("%c[1;33m", 27);
    printf("ENTER THE NAME");
    printf("%c[0m", 27);

    char msg[MAX_LEN];
    // input line point
    input_cur();
    scanf("%s", msg);
    for (i = 0; i < 10; i++) {
        if (!strcmp(user[i].name, msg)) {
            erase(18, 23, 5);
            erase(2, 23, 5); // input line clear

            // red color activtion name
            printf("%c[1;31m", 27);
            printf("%s", msg);
            printf("%c[0m", 27);
            return i;
        }
    }
    // strcat(pathname, msg);
    int count = 0, j = 0;
    i = 0;
    DIR *dir;
    struct dirent *ent;

    dir = opendir(pathname);
    if (dir != NULL) {

        /* print all the files and directories within directory */
        while ((ent = readdir(dir)) != NULL) {
            if (count - 3 == 0) {
                i = 0;
                j++;
                count = 0;
            }
            move_cur(18 + 20 * i, 6 + j * 2);
            printf("%s\n", ent->d_name);
            i++;
            count++;
        }
        closedir(dir);
    }

    erase(2, 23, 10); // name line clear

    // red color activtion name
    printf("%c[1;31m", 27);
    printf("%s", msg);
    printf("%c[0m", 27);

    // save user cursor
    user_cur(msg);

    filename_cur(user[user_num]);
    erase(18, 23, 20); // clear input cursor
    return user_num++;
}
