
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
        printf("ANSI_COLOR_RED[##############]ANSI_COLOR_RESET");
        break;

        case 'g':
        printf("ANSI_COLOR_GREEN[##############]ANSI_COLOR_RESET");
        break;

        case 'y':
        printf("ANSI_COLOR_YELLOW[##############]ANSI_COLOR_RESET");
        break;

        case 'b':
        printf("ANSI_COLOR_BLUE[##############]ANSI_COLOR_RESET");
        break;

        case 'm':
        printf("ANSI_COLOR_MAGENTA[##############]ANSI_COLOR_RESET");
        break;

        case 'c':
        printf("ANSI_COLOR_CYAN[##############]ANSI_COLOR_RESET");
        break;

        default:
        printf("ANSI_COLOR_RED[##############]ANSI_COLOR_RESET");
        break;
    }
}

void so_log_i(char c, int i){
    switch (c)
    {
        case 'r':
        printf("ANSI_COLOR_RED[####### %d #######]ANSI_COLOR_RESET", i);
        break;

        case 'g':
        printf("ANSI_COLOR_GREEN[####### %d #######]ANSI_COLOR_RESET", i);
        break;

        case 'y':
        printf("ANSI_COLOR_YELLOW[####### %d #######]ANSI_COLOR_RESET", i);
        break;

        case 'b':
        printf("ANSI_COLOR_BLUE[####### %d #######]ANSI_COLOR_RESET", i);
        break;

        case 'm':
        printf("ANSI_COLOR_MAGENTA[####### %d #######]ANSI_COLOR_RESET", i);
        break;

        case 'c':
        printf("ANSI_COLOR_CYAN[####### %d #######]ANSI_COLOR_RESET", i);
        break;

        default:
        printf("ANSI_COLOR_RED[####### %d #######]ANSI_COLOR_RESET", i);
        break;
    }
}

void so_log_s(char c, char* s){
    switch (c)
    {
        case 'r':
        printf("ANSI_COLOR_RED[####### %s #######]ANSI_COLOR_RESET", s);
        break;

        case 'g':
        printf("ANSI_COLOR_GREEN[####### %s #######]ANSI_COLOR_RESET", s);
        break;

        case 'y':
        printf("ANSI_COLOR_YELLOW[####### %s #######]ANSI_COLOR_RESET", s);
        break;

        case 'b':
        printf("ANSI_COLOR_BLUE[####### %s #######]ANSI_COLOR_RESET", s);
        break;

        case 'm':
        printf("ANSI_COLOR_MAGENTA[####### %s #######]ANSI_COLOR_RESET", s);
        break;

        case 'c':
        printf("ANSI_COLOR_CYAN[####### %s #######]ANSI_COLOR_RESET", s);
        break;

        default:
        printf("ANSI_COLOR_RED[####### %s #######]ANSI_COLOR_RESET", s);
        break;
    }
}
