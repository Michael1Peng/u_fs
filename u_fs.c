#define FUSE_USE_VERSION 31

#include "fuse.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include "u_fs_structs.h"
#include "disk_operation.h"


static int u_fs_getattr(const char *path, struct stat *stbuf,
                        struct fuse_file_info *fi) {
    (void) fi;

    memset(stbuf, 0, sizeof(struct stat));
    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    } else {
        sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);
        long location_directory = u_fs_find_directory(directoryname);
        if (location_directory == 0) {
            return -ENOENT;
        }
        if (filename[0] == '\0') { //no file name, and dir exists
            stbuf->st_mode = S_IFDIR | 755;
            stbuf->st_nlink = 2;
            return 0;
        } else {
            size_t fsize = 0;
            long location_file = u_fs_find_file(location_directory, directoryname);
            if (location_file == 0) {
                return -ENOENT;
            }
            stbuf->st_mode = S_IFREG | 0666;
            stbuf->st_nlink = 1;
            stbuf->st_size = fsize;
            return 0;
        }
    }
}

static int u_fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

    if (strcmp(path, "/") == 0) {

        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);

        FILE *disk = fopen(".disk", "rb+");
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
            if (strcmp(root_directory.directories[i].directory_name, "") != 0)
                filler(buf, root_directory.directories[i].directory_name, NULL, 0, 0);
        }
        fclose(disk);
        while (root_directory.nNextBlock != 0) {
            fseek(disk, root_directory.nNextBlock * BLOCK_SIZE, SEEK_SET);
            fread((void *) &root_directory, sizeof(struct Root_directory), 1, disk);
            for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
                if (strcmp(root_directory.directories[i].directory_name, "") != 0)
                    filler(buf, root_directory.directories[i].directory_name, NULL, 0, 0);
            }
        }
        return 0;
    }

    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }

    FILE *disk = fopen(".disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_directory * BLOCK_SIZE, SEEK_SET);
    struct u_fs_File_directory u_fs_file_directory;
    fread((void *) &u_fs_file_directory, sizeof(struct u_fs_File_directory), 1, disk);
    fseek(disk, u_fs_file_directory.nStartBlock * BLOCK_SIZE, SEEK_SET);
    struct Directory_entry directory_entry;
    fread((void *) &directory_entry, sizeof(struct Directory_entry), 1, disk);

    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") == 0) {
            filler(buf, directory_entry.u_fs_file_directory_list[i].fname, NULL, 0, 0);
        }
    }

    while (directory_entry.nNextBlock != 0) {
        fseek(disk, directory_entry.nNextBlock * BLOCK_SIZE, SEEK_SET);
        fread((void *) &directory_entry, sizeof(struct Directory_entry), 1, disk);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") == 0) {
                filler(buf, directory_entry.u_fs_file_directory_list[i].fname, NULL, 0, 0);
            }
        }
    }

    fclose(disk);
    return 0;
}

static int u_fs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    (void) fi;

    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }

    long location_file = u_fs_find_file(location_directory, directoryname);
    if (location_file == 0) {
        return -ENOENT;
    }

    FILE *disk = fopen(".disk", "rb+");
    if (disk == NULL) {
        printf("fail to open disk.\n");
        return -1;
    }

    fseek(disk, location_file * BLOCK_SIZE, SEEK_SET);
    struct u_fs_File_directory u_fs_file_directory;
    fread((void *) &u_fs_file_directory, sizeof(struct u_fs_File_directory), 1, disk);

    if (offset > u_fs_file_directory.fsize) {
        return -EFBIG;
    }

    fseek(disk, u_fs_file_directory.nStartBlock * BLOCK_SIZE, SEEK_SET);
    struct u_fs_Disk_block u_fs_disk_block;
    fread((void *) &u_fs_disk_block, sizeof(struct u_fs_Disk_block), 1, disk);

    while (offset >= MAX_DATA_IN_BLOCK) {
        offset -= MAX_DATA_IN_BLOCK;

    }
}

static struct fuse_operations u_fs_operations = {
        .getattr=u_fs_getattr,
        .readdir=u_fs_readdir,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &u_fs_operations, NULL);
}