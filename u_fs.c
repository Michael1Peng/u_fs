/** @file
 * Compile with:
 *
 *     gcc -Wall u_fs.c `pkg-config fuse3 --cflags --libs` -o u_fs
 */

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

//获取文件或文件夹属性
static int u_fs_getattr(const char *path, struct stat *stbuf,
                        struct fuse_file_info *fi) {
    (void) fi;

//  处理对应路径
    memset(stbuf, 0, sizeof(struct stat));
    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);
    printf("It's stat.\n");

//    搜索根目录的属性
    if (strcmp(path, "/") == 0) {
        printf("We are reading the root directory.\n");
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    } else {
//        搜索文件夹属性
        sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);
//        搜索对应文件夹是否存在
        printf("directoryname:%s filename:%s extension:%s\n", directoryname, filename, extension);
        long location_directory = u_fs_find_directory(directoryname);
        if (location_directory == 0) {
            return -ENOENT;
        }
        if (filename[0] == '\0') { //no file name, and dir exists
            printf("We are reading the directory:%s.\n", directoryname);
            stbuf->st_mode = S_IFDIR | 755;
            stbuf->st_nlink = 2;
            return 0;
        } else {
//            搜索文件属性
            size_t fsize = 0;
//            搜索对应文件是否存在
            long location_file = u_fs_find_file(location_directory, filename, extension);
            printf("We are reading the file:%s in the directory:%s.\n", filename, directoryname);
            struct u_fs_File_directory u_fs_file_directory;
            get_u_fs_file_directory(location_file, &u_fs_file_directory);
            if (location_file == 0) {
                return -ENOENT;
            }
            stbuf->st_mode = S_IFREG | 0666;
            stbuf->st_nlink = 1;
            stbuf->st_size = (off_t) u_fs_file_directory.fsize;
            return 0;
        }
    }
}

//获取文件夹下的文件目录信息
static int u_fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi,
                        enum fuse_readdir_flags flags) {
    (void) offset;
    (void) fi;
    (void) flags;

//    处理对应路径
    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    printf("It's ls.\n");
    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

//    获取根目录下的文件目录信息
    if (strcmp(path, "/") == 0) {
        printf("We are reading the root directory.\n");
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);

//        获得对应的Sb 和Root_directory
        struct Sb sb;
        get_sb(0, &sb);

        struct Root_directory root_directory;
        if (!get_root_directory(sb.first_blk, &root_directory)) {
            return -ENOENT;
        }

//        搜索Root_directory 下面的文件夹目录
        for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
            if (strcmp(root_directory.directories[i].directory_name, "") != 0) {
                filler(buf, root_directory.directories[i].directory_name, NULL, 0, 0);
            }
        }
//        搜索还有是否下个Root_directory 块
        while (root_directory.nNextBlock != 0) {
            get_root_directory(root_directory.nNextBlock, &root_directory);
            for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
                if (strcmp(root_directory.directories[i].directory_name, "") != 0) {
                    filler(buf, root_directory.directories[i].directory_name, NULL, 0, 0);
                }
            }
        }
        return 0;
    }

//    获取对应文件夹下的文件目录信息
//    搜索文件夹是否存在？
    printf("We are reading the directory:%s.\n", directoryname);
    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }
    printf("Found the location of the directory:%s location:%li\n", directoryname, location_directory);

//    获得对应的u_fs_File_directory 和Directory_entry
    struct u_fs_File_directory u_fs_file_directory;
    get_u_fs_file_directory(location_directory, &u_fs_file_directory);
    struct Directory_entry directory_entry;
    get_directory_entry(u_fs_file_directory.nStartBlock, &directory_entry);
    char filename_and_extension[80];

//    搜索Directory_entry 下面的文件夹目录
    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") != 0) {
            strcpy(filename_and_extension, directory_entry.u_fs_file_directory_list[i].fname);
            if (strcmp(directory_entry.u_fs_file_directory_list[i].fext, "") != 0) {
                strcat(filename_and_extension, ".");
                strcat(filename_and_extension, directory_entry.u_fs_file_directory_list[i].fext);
            }
            filler(buf, filename_and_extension, NULL, 0, 0);
        }
    }
//    搜索还有是否下个Directory_entry 块
    while (directory_entry.nNextBlock != 0) {
        get_directory_entry(directory_entry.nNextBlock, &directory_entry);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") != 0) {
                strcpy(filename_and_extension, directory_entry.u_fs_file_directory_list[i].fname);
                if (strcmp(directory_entry.u_fs_file_directory_list[i].fext, "") != 0) {
                    strcat(filename_and_extension, ".");
                    strcat(filename_and_extension, directory_entry.u_fs_file_directory_list[i].fext);
                }
                filler(buf, filename_and_extension, NULL, 0, 0);
            }
        }
    }
    return 0;

}

//创建文件夹
static int u_fs_mkdir(const char *path, mode_t mode) {
    (void) path;
    (void) mode;

    printf("It's mkdir.\n");
//    处理对应路径
    char directoryname[2 * (MAX_FILENAME + 1)];
    char filename[2 * (MAX_FILENAME + 1)];
    char extension[2 * (MAX_EXTENSION + 1)];
    memset(directoryname, 0, 2 * (MAX_FILENAME + 1));
    memset(filename, 0, 2 * (MAX_FILENAME + 1));
    memset(extension, 0, 2 * (MAX_EXTENSION + 1));

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

//    判断对应路径名是否合法
    if (strcmp(filename, "") != 0 || strcmp(extension, "") != 0) {
        printf("filename or extension is not empty\n");
        return -ENAMETOOLONG;
    }

    if (strcmp(directoryname, "") == 0) {
        printf("The directory name is empty\n");
        return -1;
    }

    if (strlen(directoryname) > 8 || strlen(directoryname) < 0) {
        printf("The length of directory name is incorrect. It should be less than 8 and more than 0.\n");
    }

//    获取新的硬盘块
    long file_directory_location = find_free_block();
    long directory_entry_location = find_free_block();

    if (file_directory_location == -1 || directory_entry_location == -1) {
        printf("There is no free blocks.\n");
        return -1;
    }

//    获得对应的Sb 和Root_directory
    struct Sb sb;
    get_sb(0, &sb);

    struct Root_directory root_directory;
    if (!get_root_directory(sb.first_blk, &root_directory)) {
        return -ENOENT;
    }

    int store_flag = 0;

//    搜索Root_directory 下面的文件夹目录，判断是否有空的
    for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
        if (strcmp(root_directory.directories[i].directory_name, "") == 0) {
            root_directory.numbers = root_directory.numbers + 1;
            strncpy(root_directory.directories[i].directory_name, directoryname, 8);
            root_directory.directories[i].nStartBlock = file_directory_location;
            write_root_directory(sb.first_blk, &root_directory);
            store_flag = 1;
            break;
        }
    }

//    如果没有，判断下一个Root_directory 块
    long root_directory_location = sb.first_blk;
    while (root_directory.nNextBlock != 0 && store_flag == 0) {
        root_directory_location = root_directory.nNextBlock;
        get_root_directory(root_directory.nNextBlock, &root_directory);
        for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
            if (strcmp(root_directory.directories[i].directory_name, "") == 0) {
                root_directory.numbers = root_directory.numbers + 1;
                strncpy(root_directory.directories[i].directory_name, directoryname, 8);
                root_directory.directories[i].nStartBlock = file_directory_location;
                write_root_directory(root_directory_location, &root_directory);
                store_flag = 1;
                break;
            }
        }
    }

//    如果前面的Root_directory 块都没有，则创建新的Root_directory 块
    if (store_flag == 0) {
        long root_directory_location_new = find_free_block();
        if (root_directory_location_new == -1) {
            printf("There is no free blocks.\n");
            return -1;
        }

        root_directory.nNextBlock = root_directory_location_new;
        write_root_directory(root_directory_location, &root_directory);

        struct Root_directory root_directory_new;
        root_directory_new.numbers = 1;
        strncpy(root_directory_new.directories[0].directory_name, directoryname, 8);
        root_directory_new.directories[0].nStartBlock = file_directory_location;
        write_root_directory(root_directory_location_new, &root_directory_new);
    }

//    将对应的u_fs_File_directory 和Directory_entry 放到对应的硬盘块
    struct u_fs_File_directory u_fs_file_directory;
    strncpy(u_fs_file_directory.fname, directoryname, 8);
    u_fs_file_directory.nStartBlock = directory_entry_location;
    u_fs_file_directory.flag = 2;
    write_u_fs_file_directory(file_directory_location, &u_fs_file_directory);

    struct Directory_entry directory_entry;
    directory_entry.numbers = 0;
    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; i++) {
        strcpy(directory_entry.u_fs_file_directory_list[i].fname, "");
    }
    directory_entry.nNextBlock = 0;
    write_directory_entry(directory_entry_location, &directory_entry);

    return 0;
}

//删除文件夹
static int u_fs_rmdir(const char *path) {
    (void) path;

    printf("It's rmdir.\n");
//    处理对应路径
    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

//    判断对应路径名是否合法
    if (strcmp(filename, "") != 0 || strcmp(extension, "") != 0) {
        printf("filename or extension is not empty\n");
        return -ENOTDIR;
    }

//    搜索文件夹是否存在？
    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }

//    获得对应的u_fs_File_directory 和Directory_entry
    struct u_fs_File_directory u_fs_file_directory;
    get_u_fs_file_directory(location_directory, &u_fs_file_directory);
    struct Directory_entry directory_entry;
    get_directory_entry(u_fs_file_directory.nStartBlock, &directory_entry);

//    检查文件夹是否为空
    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") != 0) {
            return -ENOTEMPTY;
        }
    }

    while (directory_entry.nNextBlock != 0) {
        get_directory_entry(directory_entry.nNextBlock, &directory_entry);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "\n") != 0) {
                return -ENOTEMPTY;
            }
        }
    }

//    检查完，可以删除
    struct Sb sb;
    get_sb(0, &sb);

    struct Root_directory root_directory;
    if (!get_root_directory(sb.first_blk, &root_directory)) {
        return -ENOENT;
    }

//    找到对应的文件夹
    int dir_in_root_directory_flag = 0;
    for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
        if (strcmp(root_directory.directories[i].directory_name, directoryname) == 0) {
            strcpy(root_directory.directories[i].directory_name, "");
            mark_block_free(root_directory.directories[i].nStartBlock);
            root_directory.directories[i].nStartBlock = 0;
            dir_in_root_directory_flag = 1;
            write_root_directory(sb.first_blk, &root_directory);
            break;
        }
    }
    while (root_directory.nNextBlock != 0 || dir_in_root_directory_flag == 0) {
        get_root_directory(root_directory.nNextBlock, &root_directory);
        for (int i = 0; i < MAX_DIRS_IN_ROOT; i++) {
            if (strcmp(root_directory.directories[i].directory_name, directoryname) == 0) {
                strcpy(root_directory.directories[i].directory_name, "");
                mark_block_free(root_directory.directories[i].nStartBlock);
                root_directory.directories[i].nStartBlock = 0;
                dir_in_root_directory_flag = 1;
                write_root_directory(sb.first_blk, &root_directory);
                break;
            }
        }
    }

//    删除对应的块
    mark_block_free(u_fs_file_directory.nStartBlock);
    get_directory_entry(u_fs_file_directory.nStartBlock, &directory_entry);
    while (directory_entry.nNextBlock != 0) {
        mark_block_free(directory_entry.nNextBlock);
        get_directory_entry(directory_entry.nNextBlock, &directory_entry);
    }
    return 0;
}

//创建文件
static int u_fs_mknod(const char *path, mode_t mode, dev_t dev) {
    (void) mode;
    (void) dev;

//    处理对应路径
    char directoryname[2 * (MAX_FILENAME + 1)];
    char filename[2 * (MAX_FILENAME + 1)];
    char extension[2 * (MAX_EXTENSION + 1)];
    memset(directoryname, 0, 2 * (MAX_FILENAME + 1));
    memset(filename, 0, 2 * (MAX_FILENAME + 1));
    memset(extension, 0, 2 * (MAX_EXTENSION + 1));

    printf("It's mknod.\n");
    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

    printf("directoryname:%s filename:%s extension:%s\n", directoryname, filename, extension);

//    检查文件名和路径名是否合格？
    if (strlen(filename) > 8 || strlen(filename) < 0 || strlen(extension) > 3 || strlen(extension) < 0) {
        printf("The length of file name or extension is incorrect. \n");
        printf("File name should be less than 8 and more than 0.\n");
        printf("Extension should be less than 3 and more than 0.\n");
        return -ENAMETOOLONG;
    }

    if (strcmp(filename, "") == 0) {
        printf("filename or extension is not empty\n");
        return -1;
    }

    if (strcmp(directoryname, "") == 0) {
        return -EPERM;
    }

//    寻找文件名是否存在
    long location_directory = u_fs_find_directory(directoryname);
    struct u_fs_File_directory u_fs_file_directory;
    get_u_fs_file_directory(location_directory, &u_fs_file_directory);
    long location_directory_entry = u_fs_file_directory.nStartBlock;
    struct Directory_entry directory_entry;
    get_directory_entry(location_directory_entry, &directory_entry);

    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, filename) == 0 &&
            strcmp(directory_entry.u_fs_file_directory_list[i].fext, extension) == 0) {
            return -EEXIST;
        }
    }

    while (directory_entry.nNextBlock != 0) {
        get_directory_entry(directory_entry.nNextBlock, &directory_entry);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, filename) == 0 &&
                strcmp(directory_entry.u_fs_file_directory_list[i].fext, extension) == 0) {
                return -EEXIST;
            }
        }
    }

//    获取新的硬盘块
    long file_directory_location = find_free_block();
    long disk_block_location = find_free_block();

    if (file_directory_location == -1 || disk_block_location == -1) {
        printf("There is no free blocks.\n");
        return -1;
    }

//    搜索Directory_entry 下面的文件夹目录，判断是否有空的
    get_directory_entry(location_directory_entry, &directory_entry);
    int store_flag = 0;
    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") == 0) {
            directory_entry.numbers = directory_entry.numbers + 1;
            strcpy(directory_entry.u_fs_file_directory_list[i].fname, filename);
            strcpy(directory_entry.u_fs_file_directory_list[i].fext, extension);
            directory_entry.u_fs_file_directory_list[i].nStartBlock = file_directory_location;
            directory_entry.u_fs_file_directory_list[i].flag = 1;
            write_directory_entry(location_directory_entry, &directory_entry);
            store_flag = 1;
            break;
        }
    }

    while (directory_entry.nNextBlock != 0 && store_flag == 0) {
        location_directory_entry = directory_entry.nNextBlock;
        get_directory_entry(location_directory_entry, &directory_entry);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, "") == 0) {
                directory_entry.numbers = directory_entry.numbers + 1;
                strcpy(directory_entry.u_fs_file_directory_list[i].fname, filename);
                strcpy(directory_entry.u_fs_file_directory_list[i].fext, extension);
                directory_entry.u_fs_file_directory_list[i].nStartBlock = file_directory_location;
                directory_entry.u_fs_file_directory_list[i].flag = 1;
                write_directory_entry(location_directory_entry, &directory_entry);
                store_flag = 1;
                break;
            }
        }
    }

//    如果前面的Directory_entry 块都没有，则创建新的Directory_entry 块
    if (store_flag == 0) {
        long location_directory_entry_new = find_free_block();
        if (location_directory_entry_new == -1) {
            printf("There is no free blocks.\n");
            return -1;
        }

        directory_entry.nNextBlock = location_directory_entry_new;
        write_directory_entry(location_directory_entry, &directory_entry);

        struct Directory_entry directory_entry_new;
        directory_entry_new.numbers = 1;
        strcpy(directory_entry_new.u_fs_file_directory_list[0].fname, filename);
        strcpy(directory_entry_new.u_fs_file_directory_list[0].fext, extension);
        directory_entry_new.u_fs_file_directory_list[0].nStartBlock = file_directory_location;
        directory_entry_new.u_fs_file_directory_list[0].flag = 1;
        write_directory_entry(location_directory_entry_new, &directory_entry_new);
    }

//    将对应的u_fs_File_directory 和u_fs_Disk_block 放到对应的硬盘块
    struct u_fs_File_directory u_fs_file;
    strcpy(u_fs_file.fname, filename);
    strcpy(u_fs_file.fext, extension);
    u_fs_file.nStartBlock = disk_block_location;
    u_fs_file.flag = 1;
    write_u_fs_file_directory(file_directory_location, &u_fs_file);

    struct u_fs_Disk_block u_fs_disk_block;
    write_u_fs_disk_block(disk_block_location, &u_fs_disk_block);
    return 0;
}

//写文件
static int u_fs_write(const char *path, const char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    (void) fi;

//    处理对应路径
    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    printf("It's write.\n");

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

//    判断对应路径名是否合法
    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }

    long location_file = u_fs_find_file(location_directory, filename, extension);
    if (location_file == 0) {
        return -ENOENT;
    }

//    找到对应的u_fs_File_directory
    struct u_fs_File_directory u_fs_file_directory;
    get_u_fs_file_directory(location_file, &u_fs_file_directory);

//    处理偏移
    if (offset > u_fs_file_directory.fsize) {
        return -EFBIG;
    }
    u_fs_file_directory.fsize = offset + size;
    write_u_fs_file_directory(location_file, &u_fs_file_directory);

//    找到储存内容的u_fs_Disk_block
    struct u_fs_Disk_block u_fs_disk_block;
    get_u_fs_disk_block(u_fs_file_directory.nStartBlock, &u_fs_disk_block);

    long location_disk_block;
    location_disk_block = u_fs_file_directory.nStartBlock;
    int offset_remainder = (int) offset;
    while (offset_remainder >= MAX_DATA_IN_BLOCK) {
        offset_remainder -= MAX_DATA_IN_BLOCK;
        location_disk_block = u_fs_disk_block.nNextBlock;
        if (get_u_fs_disk_block(u_fs_disk_block.nNextBlock, &u_fs_disk_block)) {
            continue;
        } else {
            printf("The offset is out of range.\n");
            return -1;
        }
    }

//    开始写文件
    int i;
    for (i = 0; i < size; i++) {
        if ((i + offset_remainder) % MAX_DATA_IN_BLOCK == 0 && i != 0) {
            if (u_fs_disk_block.nNextBlock == 0) {
                long disk_block_new_block_location = find_free_block();
                if (disk_block_new_block_location == -1) {
                    printf("There is no free blocks.\n");
                    return -1;
                }
                u_fs_disk_block.nNextBlock = disk_block_new_block_location;
                write_u_fs_disk_block(location_disk_block, &u_fs_disk_block);
                location_disk_block = disk_block_new_block_location;
                struct u_fs_Disk_block u_fs_disk_block_new;
                u_fs_disk_block = u_fs_disk_block_new;
            } else {
                location_disk_block = u_fs_disk_block.nNextBlock;
                get_u_fs_disk_block(location_disk_block, &u_fs_disk_block);
            }
        }
        if (i + offset > u_fs_file_directory.fsize) {
            break;
        }
        u_fs_disk_block.data[(i + offset_remainder) % (int) (MAX_DATA_IN_BLOCK)] = buf[i];
    }

    write_u_fs_disk_block(location_disk_block, &u_fs_disk_block);
    while (u_fs_disk_block.nNextBlock != 0) {
        location_disk_block = u_fs_disk_block.nNextBlock;
        get_u_fs_disk_block(location_disk_block, &u_fs_disk_block);
        mark_block_free(location_disk_block);
    }
    return (int) size;
}

//读文件
static int u_fs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    (void) fi;

//    处理对应路径
    printf("It's read.\n");
    char directoryname[MAX_FILENAME + 1];
    char filename[MAX_FILENAME + 1];
    char extension[MAX_EXTENSION + 1];
    memset(directoryname, 0, MAX_FILENAME + 1);
    memset(filename, 0, MAX_FILENAME + 1);
    memset(extension, 0, MAX_EXTENSION + 1);

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

//    找到对应的文件
    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }

    long location_file = u_fs_find_file(location_directory, filename, extension);
    if (location_file == 0) {
        return -ENOENT;
    }

    struct u_fs_File_directory u_fs_file_directory;
    get_u_fs_file_directory(location_file, &u_fs_file_directory);

//    处理偏移
    if (offset > u_fs_file_directory.fsize) {
        return -EFBIG;
    }

//    找到储存内容的u_fs_Disk_block
    struct u_fs_Disk_block u_fs_disk_block;
    get_u_fs_disk_block(u_fs_file_directory.nStartBlock, &u_fs_disk_block);

    int offset_remainder = (int) offset;
    while (offset_remainder >= MAX_DATA_IN_BLOCK) {
        offset_remainder -= MAX_DATA_IN_BLOCK;
        if (get_u_fs_disk_block(u_fs_disk_block.nNextBlock, &u_fs_disk_block)) {
            continue;
        } else {
            printf("The offset is out of range.\n");
            return -1;
        }
    }

//    开始读文件
    int i;
    size = u_fs_file_directory.fsize - offset;
    for (i = 0; i < size; i++) {
        if ((i + offset_remainder) % MAX_DATA_IN_BLOCK == 0 && i != 0) {
            get_u_fs_disk_block(u_fs_disk_block.nNextBlock, &u_fs_disk_block);
        }
        if (i + offset > u_fs_file_directory.fsize) {
            break;
        }
        buf[i] = u_fs_disk_block.data[(i + offset_remainder) % (int) (MAX_DATA_IN_BLOCK)];
    }

    return i;
}

//删除文件
static int u_fs_unlink(const char *path) {
    (void) path;

    printf("It's unlink.\n");
//    处理对应路径
    char directoryname[2 * (MAX_FILENAME + 1)];
    char filename[2 * (MAX_FILENAME + 1)];
    char extension[2 * (MAX_EXTENSION + 1)];
    memset(directoryname, 0, 2 * (MAX_FILENAME + 1));
    memset(filename, 0, 2 * (MAX_FILENAME + 1));
    memset(extension, 0, 2 * (MAX_EXTENSION + 1));

    sscanf(path, "/%[^/]/%[^.].%s", directoryname, filename, extension);

//    检查文件名和路径名是否合格？
    if (strcmp(filename, "") == 0) {
        return -EISDIR;
    }

//    找到对应的文件夹和文件
    long location_directory = u_fs_find_directory(directoryname);
    if (location_directory == 0) {
        return -ENOENT;
    }

    long location_file = u_fs_find_file(location_directory, filename, extension);
    if (location_file == 0) {
        return -ENOENT;
    }

//    先删除目录下的文件记录
    int delete_flag = 0;
    struct u_fs_File_directory u_fs_file_directory;
    get_u_fs_file_directory(location_directory, &u_fs_file_directory);
    long location_directory_entry = u_fs_file_directory.nStartBlock;
    struct Directory_entry directory_entry;
    get_directory_entry(location_directory_entry, &directory_entry);

    for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
        if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, filename) == 0 &&
            strcmp(directory_entry.u_fs_file_directory_list[i].fext, extension) == 0) {
            struct u_fs_File_directory u_fs_file_directory_new;
            directory_entry.u_fs_file_directory_list[i] = u_fs_file_directory_new;
            delete_flag = 1;
            write_directory_entry(location_directory_entry, &directory_entry);
        }
    }

    while (directory_entry.nNextBlock != 0 && delete_flag == 0) {
        get_directory_entry(directory_entry.nNextBlock, &directory_entry);
        for (int i = 0; i < MAX_FILES_IN_DIRECTORY; ++i) {
            if (strcmp(directory_entry.u_fs_file_directory_list[i].fname, filename) == 0 &&
                strcmp(directory_entry.u_fs_file_directory_list[i].fext, extension) == 0) {
                struct u_fs_File_directory u_fs_file_directory_new;
                directory_entry.u_fs_file_directory_list[i] = u_fs_file_directory_new;
                delete_flag = 1;
                write_directory_entry(location_directory_entry, &directory_entry);
            }
        }
    }

    if (delete_flag == 0) {
        return -ENOENT;
    }

//    删除文件对应的硬盘块
    mark_block_free(location_file);
    get_u_fs_file_directory(location_file, &u_fs_file_directory);
    mark_block_free(u_fs_file_directory.nStartBlock);
    struct u_fs_Disk_block u_fs_disk_block;
    get_u_fs_disk_block(u_fs_file_directory.nStartBlock, &u_fs_disk_block);

    while (u_fs_disk_block.nNextBlock != 0) {
        mark_block_free(u_fs_disk_block.nNextBlock);
        get_u_fs_disk_block(u_fs_disk_block.nNextBlock, &u_fs_disk_block);
    }
}

static struct fuse_operations u_fs_operations = {
        .getattr=u_fs_getattr,
        .readdir=u_fs_readdir,
        .mkdir=u_fs_mkdir,
        .rmdir=u_fs_rmdir,
        .mknod=u_fs_mknod,
        .write=u_fs_write,
        .read=u_fs_read,
        .unlink=u_fs_unlink,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &u_fs_operations, NULL);
}