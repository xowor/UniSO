#include <stdio.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

void so_log(char c){
    switch (c)
    {
        case 'r':
        printf("%s[##############]%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
        break;

        case 'g':
        printf("%s[##############]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
        break;

        case 'y':
        printf("%s[##############]%s\n", ANSI_COLOR_YELLOW, ANSI_COLOR_RESET);
        break;

        case 'b':
        printf("%s[##############]%s\n", ANSI_COLOR_BLUE, ANSI_COLOR_RESET);
        break;

        case 'm':
        printf("%s[##############]%s\n", ANSI_COLOR_MAGENTA, ANSI_COLOR_RESET);
        break;

        case 'c':
        printf("%s[##############]%s\n", ANSI_COLOR_CYAN, ANSI_COLOR_RESET);
        break;

        default:
        printf("%s[##############]%s\n", ANSI_COLOR_RED, ANSI_COLOR_RESET);
        break;
    }
}

void so_log_i(char c, int i){
    switch (c)
    {
        case 'r':
        printf("%s[####### %d #######]%s\n", ANSI_COLOR_RED, i, ANSI_COLOR_RESET);
        break;

        case 'g':
        printf("%s[####### %d #######]%s\n", ANSI_COLOR_GREEN, i, ANSI_COLOR_RESET);
        break;

        case 'y':
        printf("%s[####### %d #######]%s\n", ANSI_COLOR_YELLOW, i, ANSI_COLOR_RESET);
        break;

        case 'b':
        printf("%s[####### %d #######]%s\n", ANSI_COLOR_BLUE, i, ANSI_COLOR_RESET);
        break;

        case 'm':
        printf("%s[####### %d #######]%s\n", ANSI_COLOR_MAGENTA, i, ANSI_COLOR_RESET);
        break;

        case 'c':
        printf("%s[####### %d #######]%s\n", ANSI_COLOR_CYAN, i, ANSI_COLOR_RESET);
        break;

        default:
        printf("%s[####### %d #######]%s\n", ANSI_COLOR_RED, i, ANSI_COLOR_RESET);
        break;
    }
}

void so_log_s(char c, char* s){
    switch (c)
    {
        case 'r':
        printf("%s[####### %s #######]%s\n", ANSI_COLOR_RED, s, ANSI_COLOR_RESET);
        break;

        case 'g':
        printf("%s[####### %s #######]%s\n", ANSI_COLOR_GREEN, s, ANSI_COLOR_RESET);
        break;

        case 'y':
        printf("%s[####### %s #######]%s\n", ANSI_COLOR_YELLOW, s, ANSI_COLOR_RESET);
        break;

        case 'b':
        printf("%s[####### %s #######]%s\n", ANSI_COLOR_BLUE, s, ANSI_COLOR_RESET);
        break;

        case 'm':
        printf("%s[####### %s #######]%s\n", ANSI_COLOR_MAGENTA, s, ANSI_COLOR_RESET);
        break;

        case 'c':
        printf("%s[####### %s #######]%s\n", ANSI_COLOR_CYAN, s, ANSI_COLOR_RESET);
        break;

        default:
        printf("%s[####### %s #######]%s\n", ANSI_COLOR_RED, s, ANSI_COLOR_RESET);
        break;
    }
}

void so_log_p(char c, void* p){
    switch (c)
    {
        case 'r':
        printf("%s[####### %p #######]%s\n", ANSI_COLOR_RED, p, ANSI_COLOR_RESET);
        break;

        case 'g':
        printf("%s[####### %p #######]%s\n", ANSI_COLOR_GREEN, p, ANSI_COLOR_RESET);
        break;

        case 'y':
        printf("%s[####### %p #######]%s\n", ANSI_COLOR_YELLOW, p, ANSI_COLOR_RESET);
        break;

        case 'b':
        printf("%s[####### %p #######]%s\n", ANSI_COLOR_BLUE, p, ANSI_COLOR_RESET);
        break;

        case 'm':
        printf("%s[####### %p #######]%s\n", ANSI_COLOR_MAGENTA, p, ANSI_COLOR_RESET);
        break;

        case 'c':
        printf("%s[####### %p #######]%s\n", ANSI_COLOR_CYAN, p, ANSI_COLOR_RESET);
        break;

        default:
        printf("%s[####### %p #######]%s\n", ANSI_COLOR_RED, p, ANSI_COLOR_RESET);
        break;
    }
}
