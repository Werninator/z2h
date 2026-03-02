#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {

}

int main(int argc, char *argv[]) {
    char *filepath = NULL;
    bool newfile = false;
    int c;

    while ((c = getopt(argc, argv, "nf:")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case '?':
                printf("Unknown option -%c\n", c);
                break;
            default:
                printf("Something weird happened..\n");
                return -1;
        }
    }

    printf("newfile: %d\n", newfile);
    printf("filepath: %s\n", filepath);

    return 0;
}
