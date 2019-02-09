//
// Created by Michael on 2019-02-06.
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "u_fs_structs.h"

void Init() {

    FILE *file = fopen("/data/.disk", "w");
    if (file == NULL) {
        printf("fail to read the file.\n");
        return;
    }

    struct Sb *sb = malloc(sizeof(struct Sb));
    sb->first_blk = 1;
    fwrite((void *) sb, sizeof(struct Sb), 1, file);

    struct Root_directory *root_directory = malloc(sizeof(struct Root_directory));
    fseek(file, sb->first_blk * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) root_directory, sizeof(struct Root_directory), 1, file);
    fclose(file);
}

int main(int argc, char *argv[]) {
    Init();
}