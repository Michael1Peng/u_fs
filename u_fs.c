#define FUSE_USE_VERSION 31

#include "fuse.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "u_fs_structs.h"

u_fs_getattr