#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file> -a <employee data>\n", argv[0]);
    printf("\t-n\tcreate new database file\n");
    printf("\t-f\t(required) path to the database file\n");
    printf("\t-l\tlist the employees\n");
    printf("\t-a\tadd via CSV list of (name,address,hours)\n");
    return;
}

int main(int argc, char *argv[]) {
    char *filepath = NULL;
    char *addstring = NULL;
    bool newfile = false;
    bool list = false;
    int opt;

    while ((opt = getopt(argc, argv, "nf:a:l")) != -1) {
        switch (opt) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'a':
                addstring = optarg;
                break;
            case 'l':
                list = true;
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

        if (create_db_header(&dbhdr) == STATUS_ERROR) {
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
            close_db_file(dbfd);
            return -1;
        }
    }

    struct employee_t *employees = NULL;

    if (read_employees(dbfd, dbhdr, &employees) == STATUS_ERROR) {
        printf("Unable to read employees\n");
        free(dbhdr);
        close_db_file(dbfd);
        return -1;
    }

    if (addstring) {
        if (add_employee(dbhdr, &employees, addstring) == STATUS_ERROR) {
            printf("Unable to add employee\n");
            free(dbhdr);
            close_db_file(dbfd);
            return -1;
        }
    }

    if (list) {
        list_employees(dbhdr, employees);
    }

    if (output_file(dbfd, dbhdr, employees) == STATUS_ERROR) {
        printf("Unable to write database\n");
        free(dbhdr);
        close_db_file(dbfd);
        return -1;
    }

    free(dbhdr);
    close_db_file(dbfd);
    return 0;
}
