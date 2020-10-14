#ifndef FS_H
#define FS_H

#include "state.h"

void init_fs();

void destroy_fs();

int is_dir_empty(DirEntry *dirEntries, char *syncstrat);

int create(char *name, type nodeType, char *syncstrat);

int delete (char *name, char *syncstrat);

int lookup(char *name, char *syncstrat);

void print_tecnicofs_tree(FILE *fp);

#endif /* FS_H */
