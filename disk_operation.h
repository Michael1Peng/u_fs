#ifndef disk_operation_H
#define disk_operation_H

#include "fuse.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "u_fs_structs.h"

static int u_fs_find_directory(char *directoryname) {
    FILE *disk = fopen("disk", "rb");
    if (disk==NULL){
        printf("fail to open disk.\n");
        return -1;
    }

    Sb sb = malloc(sizeof(Sb));

    int read_disk=fread((void*)sb, sizeof(Sb), 1, disk);
    if(read_disk<=0){
        printf("fail to read the sb.\n");
        return -1;
    }

    char []
    for (int i = 0; i <sb.bitmap ; i++) {
        fseek(disk, Bitmap_START+i*BLOCK_SIZE, SEEK_SET);

    }
}

#endif