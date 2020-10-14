#ifndef INODES_H
#define INODES_H

#include <stdio.h>
#include <stdlib.h>
#include "../tecnicofs-api-constants.h"

/* FS root inode number */
#define FS_ROOT 0

#define FREE_INODE -1
#define INODE_TABLE_SIZE 50
#define MAX_DIR_ENTRIES 20

#define SUCCESS 0
#define FAIL -1

#define DELAY 5000

/*
 * Contains the name of the entry and respective i-number
 */
typedef struct dirEntry
{
	char name[MAX_FILE_NAME];
	int inumber;
} DirEntry;

/*
 * Data is either text (file) or entries (DirEntry)
 */
union Data
{
	char *fileContents;	  /* for files */
	DirEntry *dirEntries; /* for directories */
};

/*
 * I-node definition
 */
typedef struct inode_t
{
	type nodeType;
	union Data data;
	/* more i-node attributes will be added in future exercises */
} inode_t;

void fsLock(char *syncstrat, char type);

void fsUnlock(char *syncstrat);

void insert_delay(int cycles);

void inode_table_init();

void inode_table_destroy();

int inode_create(type nType, char *syncstrat);

int inode_delete(int inumber, char *syncstrat);

int inode_get(int inumber, type *nType, union Data *data, char *syncstrat);

int inode_set_file(int inumber, char *fileContents, int len);

int dir_reset_entry(int inumber, int sub_inumber, char *syncstrat);

int dir_add_entry(int inumber, int sub_inumber, char *sub_name, char *syncstrat);

void inode_print_tree(FILE *fp, int inumber, char *name);

#endif /* INODES_H */
