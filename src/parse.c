#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_endian.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#include "common.h"
#include "parse.h"

int create_db_header(struct dbheader_t **headerOut) {
    if (headerOut == NULL) {
        printf("Invalid pointer to headerOut\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to create db header\n");
        return STATUS_ERROR;
    }

    header->magic = HEADER_MAGIC;
    header->version = 0x1;
    header->count = 0;
    header->filesize = sizeof(struct dbheader_t);

    *headerOut = header;

    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (headerOut == NULL) {
        printf("Invalid pointer to headerOut\n");
        return STATUS_ERROR;
    }

    if (fd < 0) {
        printf("Got invalid FD\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        printf("Malloc failed to create a db header\n");
        return STATUS_ERROR;
    }

    if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("Improper header magic\n");
        free(header);
        return STATUS_ERROR;
    }
    if (header->version != 1) {
        printf("Improper header version\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat dbstat = {0};
    fstat(fd, &dbstat);
    if (header->filesize != dbstat.st_size) {
        printf("Corrupted database\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;

    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *header, struct employee_t *employees) {
    if (fd < 0) {
        printf("Got invalid FD\n");
        return STATUS_ERROR;
    }

    if (header == NULL) {
        printf("Invalid pointer to header\n");
        return STATUS_ERROR;
    }

    if (employees == NULL) {
        printf("Invalid pointer to employees\n");
        return STATUS_ERROR;
    }

    int count = header->count;

    header->magic = htonl(header->magic);
    header->version = htons(header->version);
    header->count = htons(header->count);
    header->filesize = htonl(sizeof(struct dbheader_t) + count * sizeof(struct employee_t));

    lseek(fd, 0, SEEK_SET);
    
    write(fd, header, sizeof(struct dbheader_t));

    for (int i = 0; i < count; i++) {
        employees[i].hours = htonl(employees[i].hours);
        write(fd, &employees[i], sizeof(struct employee_t));
    }

    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *header, struct employee_t **employeesOut) {
    int ret_sts = STATUS_ERROR;

    if (fd < 0) {
        printf("Got invalid FD\n");
        return ret_sts;
    }

    if (header == NULL) {
        printf("Invalid pointer to `header`\n");
        return ret_sts;
    }

    if (employeesOut == NULL) {
        printf("Invalid pointer to `employeesOut`\n");
        return ret_sts;
    }

    int count = header->count;
    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL) {
        printf("Malloc failed\n");
        return ret_sts;
    }

    size_t bytes_count = count * sizeof(struct employee_t);
    if (read(fd, employees, bytes_count) != bytes_count) {
        perror("read");
        free(employees);
        return ret_sts;
    }

    for (int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;

    ret_sts = STATUS_SUCCESS;

    return ret_sts;
}

int add_employee(struct dbheader_t *header, struct employee_t *employees, char *addstring) {
    if (header == NULL) {
        printf("Invalid pointer to header\n");
        return STATUS_ERROR;
    }

    if (employees == NULL) {
        printf("Invalid pointer to employees\n");
        return STATUS_ERROR;
    }

    int index = header->count - 1;

    char *name = strtok(addstring, ",");
    char *address = strtok(NULL, ",");
    char *hours = strtok(NULL, ",");

    strncpy(employees[index].name, name, sizeof(employees[index].name));
    strncpy(employees[index].address, address, sizeof(employees[index].address));
    employees[index].hours = atoi(hours);

    return STATUS_SUCCESS;
}
