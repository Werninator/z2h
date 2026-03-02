#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t-n\tcreate new database file\n");
    printf("\t-f\t(required) path to the database file\n");
    return;
}

int main(int argc, char *argv[]) {
    char *filepath = NULL;
    bool newfile = false;
    int opt;

    while ((opt = getopt(argc, argv, "nf:")) != -1) {
        switch (opt) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            default:
                print_usage(argv);
                return -1;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a required argument!\n");
        print_usage(argv);
        return 0;
    }

    // database file descriptor
    int dbfd = -1;
    struct dbheader_t *dbhdr = NULL;

    if (newfile) {
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return -1;
        }

        if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
            printf("Unable to create database header\n");
            return -1;
        }
    } else {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return -1;
        }

        if (validate_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
            printf("Unable to validate database header\n");
            return -1;
        }
    }

    if (output_file(dbfd, dbhdr) == STATUS_ERROR) {
        printf("Unable to write database\n");
        return -1;
    }

    free(dbhdr);
    close_db_file(dbfd);
    return 0;
}
