#ifndef disk_operation_H
#define disk_operation_H

static long u_fs_find_directory(char *directoryname) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    struct Sb sb;

    int read_disk = (int) fread((void *) &sb, sizeof(struct Sb), 1, disk);
    if (read_disk <= 0) {
        printf("fail to read the sb.\n");
        return -1;
    }
    fseek(disk, sb.first_blk * BLOCK_SIZE, SEEK_SET);
    struct Root_directory root_directory;
    fread((void *) &root_directory, sizeof(struct Root_directory), 1, disk);
    for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
        if (strcmp(directoryname, root_directory.directories[i].directory_name) == 0) {
            fclose(disk);
            return root_directory.directories[i].nStartBlock;
        }
    }
    while (root_directory.nNextBlock != 0) {
        fseek(disk, root_directory.nNextBlock * BLOCK_SIZE, SEEK_SET);
        fread((void *) &root_directory, sizeof(struct Root_directory), 1, disk);
        for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
            if (strcmp(directoryname, root_directory.directories[i].directory_name) == 0) {
                fclose(disk);
                return root_directory.directories[i].nStartBlock;
            }
        }
    }
    fclose(disk);
    return 0;
}

static long u_fs_find_file(long directory_pos, char *filename, char *extension) {
    FILE *disk = fopen("/data/.disk", "rb");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, directory_pos * BLOCK_SIZE, SEEK_SET);
    struct u_fs_File_directory u_fs_file_directory;
    fread((void *) &u_fs_file_directory, sizeof(struct u_fs_File_directory), 1, disk);
    fseek(disk, u_fs_file_directory.nStartBlock * BLOCK_SIZE, SEEK_SET);
    struct Directory_entry directory_entry;
    fread((void *) &directory_entry, sizeof(struct Directory_entry), 1, disk);

    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(filename, directory_entry.u_fs_file_directory_list[i].fname) == 0 &&
            strcmp(extension, directory_entry.u_fs_file_directory_list[i].fext) == 0) {
            fclose(disk);
            return directory_entry.u_fs_file_directory_list[i].nStartBlock;
        }
    }

    while (directory_entry.nNextBlock != 0) {
        fseek(disk, directory_entry.nNextBlock * BLOCK_SIZE, SEEK_SET);
        fread((void *) &directory_entry, sizeof(struct Directory_entry), 1, disk);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(filename, directory_entry.u_fs_file_directory_list[i].fname) == 0 &&
                strcmp(extension, directory_entry.u_fs_file_directory_list[i].fext) == 0) {
                fclose(disk);
                return directory_entry.u_fs_file_directory_list[i].nStartBlock;
            }
        }
    }

    fclose(disk);
    return 0;
}

int get_sb(long location_sb, struct Sb *sb_receiver) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_sb * BLOCK_SIZE, SEEK_SET);
    fread((void *) sb_receiver, sizeof(struct Sb), 1, disk);
    fclose(disk);
    return 1;
}

int get_bitmap(long location_bitmap, char bitmap_receiver[BLOCK_SIZE]) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_bitmap * BLOCK_SIZE, SEEK_SET);
    fread(bitmap_receiver, BLOCK_SIZE, 1, disk);

    fclose(disk);
    return 1;
}

int get_root_directory(long location_root_directory, struct Root_directory *root_directory_receiver) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_root_directory * BLOCK_SIZE, SEEK_SET);
    fread((void *) root_directory_receiver, sizeof(struct Root_directory), 1, disk);

    fclose(disk);
    return 1;
}

int
get_u_fs_file_directory(long location_u_fs_file_directory,
                        struct u_fs_File_directory *u_fs_file_directory_receiver) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_u_fs_file_directory * BLOCK_SIZE, SEEK_SET);
    fread((void *) u_fs_file_directory_receiver, sizeof(struct u_fs_File_directory), 1, disk);

    fclose(disk);
    return 1;
}

int get_directory_entry(long location_directory_entry, struct Directory_entry *directory_entry_receiver) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_directory_entry * BLOCK_SIZE, SEEK_SET);
    fread((void *) directory_entry_receiver, sizeof(struct Directory_entry), 1, disk);

    fclose(disk);
    return 1;
}

int get_u_fs_disk_block(long location_u_fs_disk_block, struct u_fs_Disk_block *u_fs_disk_block_receiver) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_u_fs_disk_block * BLOCK_SIZE, SEEK_SET);
    fread((void *) u_fs_disk_block_receiver, sizeof(struct u_fs_Disk_block), 1, disk);

    fclose(disk);
    return 1;
}

int write_sb(long location_sb, struct Sb *sb) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_sb * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) sb, sizeof(struct Sb), 1, disk);
    fclose(disk);
    return 1;
}

int write_bitmap(long location_bitmap, char bitmap[BLOCK_SIZE]) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_bitmap * BLOCK_SIZE, SEEK_SET);
    fwrite(bitmap, BLOCK_SIZE, 1, disk);

    fclose(disk);
    return 1;
}

int write_root_directory(long location_root_directory, struct Root_directory *root_directory) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_root_directory * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) root_directory, sizeof(struct Root_directory), 1, disk);

    fclose(disk);
    return 1;
}

int write_u_fs_file_directory(long location_u_fs_file_directory, struct u_fs_File_directory *u_fs_file_directory) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_u_fs_file_directory * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) u_fs_file_directory, sizeof(struct u_fs_File_directory), 1, disk);

    fclose(disk);
    return 1;
}

int write_directory_entry(long location_directory_entry, struct Directory_entry *directory_entry) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_directory_entry * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) directory_entry, sizeof(struct Directory_entry), 1, disk);

    fclose(disk);
    return 1;
}

int write_u_fs_disk_block(long location_u_fs_disk_block, struct u_fs_Disk_block *u_fs_disk_block) {
    FILE *disk = fopen("/data/.disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_u_fs_disk_block * BLOCK_SIZE, SEEK_SET);
    fwrite((void *) u_fs_disk_block, sizeof(struct u_fs_Disk_block), 1, disk);

    fclose(disk);
    return 1;
}

long find_free_block() {
    FILE *file = fopen("/data/diskimg", "rb+");
    if (file == NULL) {
        printf("fail to read the file.\n");
        return -1;
    }

    int bitmap_length;
    fread(&bitmap_length, sizeof(int), 1, file);
    fseek(file, sizeof(int), SEEK_SET);
    char bitmap[bitmap_length];
    fread(bitmap, (size_t) bitmap_length, 1, file);
    for (int i = 0; i < bitmap_length; ++i) {
        if (bitmap[i] == '0') {
            bitmap[i] = '1';
            fseek(file, sizeof(int), SEEK_SET);
            fwrite(bitmap, (size_t) bitmap_length, 1, file);
            fclose(file);
            return i;
        }
    }
    printf("There is no free block.\n");
    fclose(file);
    return -1;
}

void mark_block_free(long block_location) {
    FILE *file = fopen("/data/diskimg", "rb+");
    if (file == NULL) {
        printf("fail to read the file.\n");
        return;
    }
    int bitmap_length;
    fread(&bitmap_length, sizeof(int), 1, file);

    char bitmap[bitmap_length];
    fseek(file, sizeof(int), SEEK_SET);
    fread(bitmap, (size_t) bitmap_length, 1, file);
    bitmap[block_location] = '0';
    fseek(file, sizeof(int), SEEK_SET);
    fwrite(bitmap, (size_t) bitmap_length, 1, file);
    fclose(file);
}

#endif