
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include "types.h"
#include "crypto.h"

extern const char curve_parameter[192];
static const char *opt_string = "glll:s:?:v";
volatile int32_t toStop = 0;

static void display_usage(void)
{
    fprintf(stderr, "This is crypto tool.\n");
    fprintf(stderr, "Usage: crypto option [arguments]\n");
    fprintf(stderr, "  -g\tgenerate key pair\n");
    fprintf(stderr, "  -s\tecc sign\n");
    fprintf(stderr, "  -v\tecc verify\n");
}

int softsim_printf (int leve, int flag, const char *msg, ...)
{
    char content[1024] = {0};
    uint32_t len = 0;
    va_list vl_list;

    va_start(vl_list, msg);
    vsnprintf((char *)&content[len], sizeof(content) - len, (const char *)msg, vl_list);
    va_end(vl_list);

    printf(content, strlen(content));

    return 0;
}


int main(int argc, char * const *argv)
{
    int opt = 0;
    init_curve_parameter(curve_parameter);

    if (argc < 2) {
        display_usage();
        return -1;
    }
    opt = getopt(argc, argv, opt_string);
    while (opt != -1) {
        switch (opt) {
            case 'g':
                printf("general key pair\n");
                break;
            case 'e':
                break;
            case 's':
                break;
            case 'v':
                break;

            case '?':
                display_usage();
                break;

            default:
                break;
        }
        opt = getopt(argc, argv, opt_string);
    }
}
