#ifndef FS_H
#define FS_H

#include "state.h"

void init_fs();

void destroy_fs();

int is_dir_empty(DirEntry *dirEntries);

int create(char *name, type nodeType);

int delete (char *name);

int move(char *src, char *dest);

int lookup(char *name);

int aux_lookup(char *name, int *locked, int *index, int *already_locked, int already_locked_amount);

void print_tecnicofs_tree(FILE *fp);

#endif /* FS_H */
