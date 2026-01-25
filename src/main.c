#include <_stdio.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n  - create new database file\n");
    printf("\t -f  - (required) path to database file\n");
}

int main(int argc, char *argv[]) {
    char *filename = NULL;
    bool newfile = false;
    char c;

    int dbfd = -1;
    struct dbheader_t *dbheader = NULL;

    while ((c = getopt(argc, argv, "nf:")) != -1) {
        switch (c) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filename = optarg;
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
        if (create_db_header(dbfd, &dbheader) == STATUS_ERROR) {
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

    output_file(dbfd, dbheader);

    return 0;
}
