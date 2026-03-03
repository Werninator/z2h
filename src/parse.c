#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

int list_employees(struct dbheader_t *header, struct employee_t *employees) {
    if (NULL == header) return STATUS_ERROR;
    if (NULL == employees) return STATUS_ERROR;

    if (header->count == 0) {
        printf("No Employees in this database yet.\n");
        return STATUS_ERROR;
    }

    for (int i = 0; i < header->count; i++) {
        printf("Employee Nr. \t%d\n", i + 1);
        printf("\tName:\t\t%s\n", employees[i].name);
        printf("\tAddress:\t%s\n", employees[i].address);
        printf("\tHours:\t\t%d\n", employees[i].hours);
    }

    return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *header, struct employee_t **employees, char *addstring) {
    if (NULL == header) return STATUS_ERROR;
    if (NULL == employees) return STATUS_ERROR;
    if (NULL == *employees) return STATUS_ERROR;
    if (NULL == addstring) return STATUS_ERROR;

    char *name = strtok(addstring, ",");
    if (NULL == name) return STATUS_ERROR;

    char *address = strtok(NULL, ",");
    if (NULL == address) return STATUS_ERROR;

    char *hours = strtok(NULL, ",");
    if (NULL == hours) return STATUS_ERROR;

    struct employee_t *e = *employees;
    e = realloc(e, sizeof(struct employee_t) * (header->count + 1));

    if (e == NULL) {
        return STATUS_ERROR;
    }

    header->count++;

    // -1 because of \0
    strncpy(e[header->count-1].name, name, sizeof(e[header->count-1].name)-1);
    strncpy(e[header->count-1].address, address, sizeof(e[header->count-1].address)-1);
    e[header->count-1].hours = atoi(hours);

    *employees = e;

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *header, struct employee_t **employeesOut) {
    if (fd < 0) {
        printf("Got a bad fd from the user\n");
        return STATUS_ERROR;
    }

    int count = header->count;
    struct employee_t *employees = calloc(count, sizeof(struct employee_t));

    if (employees == NULL) {
        printf("Malloc failed to allocate employees\n");
        return STATUS_ERROR;
    }

    read(fd, employees, count * sizeof(struct employee_t));

    // unpack
    for (int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *header, struct employee_t *employees) {
    if (fd < 0) {
        printf("Got a bad fd from the user\n");
        return STATUS_ERROR;
    }

    // before converting for iterating through the employees
    int realcount = header->count;

    // endianness things with
    // - ntohl: host to network long 32bit
    // - ntohs: host to network short 16bit
    header->magic = htonl(header->magic);
    header->version = htons(header->version);
    header->count = htons(header->count);
    header->filesize = htonl(sizeof(struct dbheader_t) + (sizeof(struct employee_t) * realcount));

    // pointer to the beginning of the file
    lseek(fd, 0, SEEK_SET);

    write(fd, header, sizeof(struct dbheader_t));

    for (int i = 0; i < realcount; i++) {
        // pack
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("Got a bad fd from the user\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

    if (header == NULL) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }

    // printf("employees[0].name %s\n", employees[0].name);pack the header from the file
    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    // endianness things with
    // - ntohl: network to host long 32bit
    // - ntohs: network to host short 16bit
    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != HEADER_DEFAULT_VERSION) {
        printf("Improper header version!\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size) {
        printf("Corrputed database!\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
}

int create_db_header(struct dbheader_t **headerOut) {
    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

    if (header == NULL) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }

    header->version = HEADER_DEFAULT_VERSION;
    header->count = 0;
    header->magic = HEADER_MAGIC;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}
