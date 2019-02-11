//
// Created by Michael on 2019-02-10.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include "u_fs_structs.h"

int main(int argc, char *argv[]) {
    char directoryname[18];
    char filename[9];
    char extension[4];
    memset(directoryname, 0, 18);
    memset(filename, 0, 9);
    memset(extension, 0, 3);

    sscanf(argv[1], "/%[^/]/%[^.].%s", directoryname, filename, extension);
    printf("%s %s %s", directoryname, filename, extension);

    if (strcmp(filename, "") != 0 || strcmp(extension, "") != 0) {
        printf("filename or extension is not empty\n");
        return 1;
    }

    if (strcmp(directoryname, "") == 0) {
        printf("The directory name is empty\n");
        return 1;
    }

    if (strlen(directoryname) > 8 || strlen(directoryname) < 0) {
        printf("The length of directory name is incorrect. It should be less than 8 and more than 0.\n");
    }
}