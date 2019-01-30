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

    fseek(disk, sb->first_blk * BLOCK_SIZE, SEEK_SET);
    struct u_fs_File_directory *u_fs_file_directory;
    fread((void *) u_fs_file_directory, sizeof(struct u_fs_File_directory), 1, disk);

    fseek(disk, u_fs_file_directory->nStartBlock * BLOCK_SIZE, SEEK_SET);
    struct u_fs_Disk_block *u_fs_disk_block;
    fread((void *) u_fs_disk_block, sizeof(struct u_fs_Disk_block), 1, disk);
    char string_number[2];
    strncpy(string_number, u_fs_disk_block->data, 2);
    int number = atoi(string_number);
    char data[16];
    for (int i = 0; i < number; i++) {
        strncpy(data, u_fs_disk_block->data + 2 + i * 16, 16);
        struct Root_directory_data *root_directory_data=Root_directory_data_new(data);
    }
}

#endif