#ifndef disk_operation_H
#define disk_operation_H

static long u_fs_find_directory(char *directoryname) {
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
    struct Root_directory *root_directory;
    fread((void *) root_directory, sizeof(struct Root_directory), 1, SEEK_SET);
    for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
        if (strcmp(directoryname, root_directory->directories[i].directory_name) == 0) {
            fclose(disk);
            return root_directory->directories[i].nStartBlock;
        }
    }
    while (root_directory->nNextBlock != 0) {
        fseek(disk, root_directory->nNextBlock * BLOCK_SIZE, SEEK_SET);
        fread((void *) root_directory, sizeof(struct Root_directory), 1, SEEK_SET);
        for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
            if (strcmp(directoryname, root_directory->directories[i].directory_name) == 0) {
                fclose(disk);
                return root_directory->directories[i].nStartBlock;
            }
        }
    }
    fclose(disk);
    return 0;
}

static long u_fs_find_file(long directory_pos, char *filename) {
    FILE *disk = fopen(".disk", "rb");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, directory_pos * BLOCK_SIZE, SEEK_SET);
    struct u_fs_File_directory *u_fs_file_directory;
    fread((void *) u_fs_file_directory, sizeof(struct u_fs_File_directory), 1, disk);
    fseek(disk, u_fs_file_directory->nStartBlock * BLOCK_SIZE, SEEK_SET);
    struct Directory_entry *directory_entry;
    fread((void *) directory_entry, sizeof(struct Directory_entry), 1, SEEK_SET);

    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(filename, directory_entry->u_fs_file_directory_list[i].fname) == 0) {
            fclose(disk);
            return directory_entry->u_fs_file_directory_list[i].nStartBlock;
        }
    }

    while (directory_entry->nNextBlock != 0) {
        fseek(disk, directory_entry->nNextBlock * BLOCK_SIZE, SEEK_SET);
        fread((void *) directory_entry, sizeof(struct Directory_entry), 1, SEEK_SET);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(filename, directory_entry->u_fs_file_directory_list[i].fname) == 0) {
                fclose(disk);
                return directory_entry->u_fs_file_directory_list[i].nStartBlock;
            }
        }
    }

    fclose(disk);
    return 0;
}

#endif