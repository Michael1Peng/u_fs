#ifndef disk_operation_H
#define disk_operation_H

static int u_fs_find_directory(char *directoryname) {
    FILE *disk = fopen(".disk", "rb");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    struct Sb *sb;

    int read_disk = (int) fread((void *) sb, sizeof(struct Sb), 1, disk);
    if (read_disk <= 0) {
        printf("fail to read the sb.\n");
        return -1;
    }

    char bitmap[BLOCK_SIZE];
    for (int i = 0; i < sb.bitmap; i++) {
        fseek(disk, Bitmap_START + i * BLOCK_SIZE, SEEK_SET);
        fread(bitmap, BLOCK_SIZE, 1, disk);
    }
}

#endif