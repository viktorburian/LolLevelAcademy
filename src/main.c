#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n  - create new database file\n");
    printf("\t -f  - (required) path to database file\n");
    printf("\t -a  - add employee via CSV (name,address,hours)\n");
    printf("\t -l  - list employees\n");
}

int main(int argc, char *argv[]) {
    char *filename = NULL;
    char *addargs = NULL;
    bool newfile = false;
    char c;

    int dbfd = -1;
    struct dbheader_t *dbheader = NULL;
    struct employee_t *employees = NULL;

    while ((c = getopt(argc, argv, "nf:a:")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filename = optarg;
                break;
            case 'a':
                addargs = optarg;
                break;
            case '?':
                printf("Unknown option -%c\n", c);
                break;
            default:
                return -1;
        }
    }

    if (filename == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);

        return -1;
    }

    if (newfile) {
        dbfd = create_db_file(filename);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");
            return -1;
        }
        if (create_db_header(&dbheader) == STATUS_ERROR) {
            printf("Failed to create database header\n");
            return -1;
        }
    } else {
        dbfd = open_db_file(filename);
        if (dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return -1;
        }
        if (validate_db_header(dbfd, &dbheader) == STATUS_ERROR) {
            printf("Database header validation failed\n");
            return -1;
        }
    }

    if (read_employees(dbfd, dbheader, &employees) == STATUS_ERROR) {
        printf("Failed to read employees\n");
        return -1;
    }

    if (addargs) {
        if (add_employee(dbheader, &employees, addargs) == STATUS_ERROR) {
            printf("Failed to add employee\n");
            return -1;
        }
    }

    if (output_file(dbfd, dbheader, employees) == STATUS_ERROR) {
        printf("Failed to output file\n");
        return -1;
    }

    return 0;
}
