#include "operations.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Given a path, fills pointers with strings for the parent path and child
 * file name
 * Input:
 *  - path: the path to split. ATENTION: the function may alter this parameter
 *  - parent: reference to a char*, to store parent path
 *  - child: reference to a char*, to store child file name
 * Returns : number of slashes in the path used for getting path's depth
 */
int split_parent_child_from_path(char *path, char **parent, char **child)
{
	int n_slashes = 0, last_slash_location = 0;
	int len = strlen(path);

	// deal with trailing slash ( a/x vs a/x/ )
	if (path[len - 1] == '/')
	{
		path[len - 1] = '\0';
	}

	for (int i = 0; i < len; ++i)
	{
		if (path[i] == '/' && path[i + 1] != '\0')
		{
			last_slash_location = i;
			n_slashes++;
		}
	}

	if (n_slashes == 0)
	{ // root directory
		*parent = "";
		*child = path;
		return n_slashes;
	}

	path[last_slash_location] = '\0';
	*parent = path;
	*child = path + last_slash_location + 1;
	return n_slashes;
}

/*
 * Initializes tecnicofs and creates root node.
 */
void init_fs()
{
	inode_table_init();

	/* create root inode */
	int root = inode_create(T_DIRECTORY, -1);

	if (root != FS_ROOT)
	{
		printf("failed to create node for tecnicofs root\n");
		exit(EXIT_FAILURE);
	}
}

/*
 * Destroy tecnicofs and inode table.
 */
void destroy_fs()
{
	inode_table_destroy();
}

/*
 * Checks if content of directory is not empty.
 * Input:
 *  - entries: entries of directory
 * Returns: SUCCESS or FAIL
 */
int is_dir_empty(DirEntry *dirEntries)
{
	if (dirEntries == NULL)
	{
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		if (dirEntries[i].inumber != FREE_INODE)
		{
			return FAIL;
		}
	}
	return SUCCESS;
}

/*
 * Looks for node in directory entry from name.
 * Input:
 *  - name: path of node
 *  - entries: entries of directory
 * Returns:
 *  - inumber: found node's inumber
 *  - FAIL: if not found
 */
int lookup_sub_node(char *name, DirEntry *entries)
{
	if (entries == NULL)
	{
		return FAIL;
	}
	for (int i = 0; i < MAX_DIR_ENTRIES; i++)
	{
		if (entries[i].inumber != FREE_INODE && strcmp(entries[i].name, name) == 0)
		{
			return entries[i].inumber;
		}
	}
	return FAIL;
}

/*
 * Creates a new node given a path.
 * Input:
 *  - name: path of node
 *  - nodeType: type of node
 * Returns: SUCCESS or FAIL
 */
int create(char *name, type nodeType)
{
	int parent_inumber, child_inumber;
	int locked[INODE_TABLE_SIZE] = {0}, locked_index;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType;
	union Data pdata;

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = aux_lookup(parent_name, locked, &locked_index, NULL, 0);

	if (parent_inumber == FAIL)
	{
		printf("failed to create %s, invalid parent dir %s\n",
			   name, parent_name);
		unlockAll(locked, locked_index);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if (pType != T_DIRECTORY)
	{
		printf("failed to create %s, parent %s is not a dir\n",
			   name, parent_name);
		unlockAll(locked, locked_index);
		return FAIL;
	}

	if (lookup_sub_node(child_name, pdata.dirEntries) != FAIL)
	{
		printf("failed to create %s, already exists in dir %s\n",
			   child_name, parent_name);
		unlockAll(locked, locked_index);
		return FAIL;
	}

	/* create node and add entry to folder that contains new node */
	child_inumber = inode_create(nodeType, parent_inumber);

	if (child_inumber == FAIL)
	{
		printf("failed to create %s in  %s, couldn't allocate inode\n",
			   child_name, parent_name);
		unlockAll(locked, locked_index);
		return FAIL;
	}

	if (dir_add_entry(parent_inumber, child_inumber, child_name) == FAIL)
	{
		printf("could not add entry %s in dir %s\n",
			   child_name, parent_name);
		unlockAll(locked, locked_index);
		return FAIL;
	}
	unlockAll(locked, locked_index);
	return SUCCESS;
}

/*
 * Deletes a node given a path.
 * Input:
 *  - name: path of node
 * Returns: SUCCESS or FAIL
 */
int delete (char *name)
{

	int parent_inumber, child_inumber;
	int locked[INODE_TABLE_SIZE] = {0}, locked_index;
	char *parent_name, *child_name, name_copy[MAX_FILE_NAME];
	/* use for copy */
	type pType, cType;
	union Data pdata, cdata;

	strcpy(name_copy, name);
	split_parent_child_from_path(name_copy, &parent_name, &child_name);

	parent_inumber = aux_lookup(parent_name, locked, &locked_index, NULL, 0);

	if (parent_inumber == FAIL)
	{
		printf("failed to delete %s, invalid parent dir %s\n",
			   child_name, parent_name);
		unlockAll(locked, locked_index);
		return FAIL;
	}

	inode_get(parent_inumber, &pType, &pdata);

	if (pType != T_DIRECTORY)
	{
		printf("failed to delete %s, parent %s is not a dir\n",
			   child_name, parent_name);

		unlockAll(locked, locked_index);
		return FAIL;
	}

	child_inumber = lookup_sub_node(child_name, pdata.dirEntries);

	if (child_inumber == FAIL)
	{
		printf("could not delete %s, does not exist in dir %s\n",
			   name, parent_name);
		unlockAll(locked, locked_index);
		return FAIL;
	}

	inodeLock('w', child_inumber);
	inode_get(child_inumber, &cType, &cdata);

	if (cType == T_DIRECTORY && is_dir_empty(cdata.dirEntries) == FAIL)
	{
		printf("could not delete %s: is a directory and not empty\n",
			   name);
		unlockAll(locked, locked_index);
		inodeUnlock(child_inumber);
		return FAIL;
	}

	/* remove entry from folder that contained deleted node */
	if (dir_reset_entry(parent_inumber, child_inumber) == FAIL)
	{
		printf("failed to delete %s from dir %s\n",
			   child_name, parent_name);
		unlockAll(locked, locked_index);
		inodeUnlock(child_inumber);
		return FAIL;
	}

	if (inode_delete(child_inumber) == FAIL)
	{
		printf("could not delete inode number %d from dir %s\n",
			   child_inumber, parent_name);
		unlockAll(locked, locked_index);
		inodeUnlock(child_inumber);
		return FAIL;
	}

	unlockAll(locked, locked_index);
	inodeUnlock(child_inumber);
	return SUCCESS;
}

/*
 * Moves a node from a given to another one.
 * Input:
 *  - src: path of the node
 *  - dest: destination of the node
 * Returns: SUCCESS or FAIL
 */
int move(char *src, char *dest)
{
	int sparent_inumber, dparent_inumber, moved_inumber;		   /* src parent inumber, destination parent inumber, moved file inumber */
	int slocked[INODE_TABLE_SIZE] = {0};						   /* Locked inumbers to the to the source parent directory */
	int dlocked[INODE_TABLE_SIZE] = {0};						   /* Locked inumbers to the to the destination parent directory */
	int sindex, dindex;											   /* represent the ammount of locks needed to get to source (sindex) or to destination (source) */
	char *sparent_name, *schild_name, *dparent_name, *dchild_name; /* Used for name splitting */
	type sType, dType;											   /* used for getting the type of the source, destination and moved inodes*/
	union Data sdata, ddata;									   /* used for getting the date of the source, detination and moved inodes */

	int ddepth = split_parent_child_from_path(dest, &dparent_name, &dchild_name); /* Splitting paths and getting depth of each path */
	int sdepth = split_parent_child_from_path(src, &sparent_name, &schild_name);

	/* Checking for loop cases m /a /a/a */
	if (strcmp(schild_name, dparent_name + 1) == 0 && *sparent_name == '\0')
		return FAIL;

	/* Establishing an order for locking, the shallowest inode first */
	if (sdepth < ddepth)
	{
		sparent_inumber = aux_lookup(sparent_name, slocked, &sindex, NULL, 0);
		dparent_inumber = aux_lookup(dparent_name, dlocked, &dindex, slocked, sindex);
	}
	else if (sdepth > ddepth)
	{
		dparent_inumber = aux_lookup(dparent_name, dlocked, &dindex, NULL, 0);
		sparent_inumber = aux_lookup(sparent_name, slocked, &sindex, dlocked, dindex);
	}
	else
	{
		/* If both paths have the same depth, use supposed inumbers */
		sparent_inumber = lookup(sparent_name);
		dparent_inumber = lookup(dparent_name);

		/* First case, also handles moving within the same directory (when the inumbers are the same) */
		if (sparent_inumber >= dparent_inumber)
		{
			sparent_inumber = aux_lookup(sparent_name, slocked, &sindex, NULL, 0);
			dparent_inumber = aux_lookup(dparent_name, dlocked, &dindex, slocked, sindex);
		}
		else
		{
			dparent_inumber = aux_lookup(dparent_name, dlocked, &dindex, NULL, 0);
			sparent_inumber = aux_lookup(sparent_name, slocked, &sindex, dlocked, dindex);
		}
	}

	// Now that we locked everything, verifying src and dest parent actually exist
	if (sparent_inumber < 0 || dparent_inumber < 0)
	{
		unlockAll(slocked, sindex);
		unlockAll(dlocked, dindex);
		return FAIL;
	}

	// Veryfying the destination is a folder and doesnt contain another file with the same name
	inode_get(dparent_inumber, &dType, &ddata);
	if (dType != T_DIRECTORY || lookup_sub_node(dchild_name, ddata.dirEntries) != FAIL)
	{
		unlockAll(slocked, sindex);
		unlockAll(dlocked, dindex);
		return FAIL;
	}

	// Veryfying the inode we want to move exists
	inode_get(sparent_inumber, &sType, &sdata);
	moved_inumber = lookup_sub_node(schild_name, sdata.dirEntries);
	if (sType != T_DIRECTORY || moved_inumber == FAIL)
	{
		unlockAll(slocked, sindex);
		unlockAll(dlocked, dindex);
		return FAIL;
	}

	/* Actual move operation happens here */
	dir_remove_entry(sparent_inumber, moved_inumber);
	dir_add_entry(dparent_inumber, moved_inumber, dchild_name);

	unlockAll(slocked, sindex);
	unlockAll(dlocked, dindex);
	return SUCCESS;
}

/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */
int lookup(char *name)
{
	int locked[INODE_TABLE_SIZE] = {0};
	int index = 0;
	char *saveptr;
	char full_path[MAX_FILE_NAME];
	char delim[] = "/";
	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	/* Critical Zone */
	inodeLock('r', current_inumber); /* Locking the root folder */
	locked[index++] = current_inumber;

	/* get root inode data */
	inode_get(current_inumber, &nType, &data);
	char *path = strtok_r(full_path, delim, &saveptr);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL)
	{
		inodeLock('r', current_inumber); /*  Locking all the nodes along the lookup path */
		locked[index++] = current_inumber;

		inode_get(current_inumber, &nType, &data);
		path = strtok_r(NULL, delim, &saveptr);
	}
	/* Unlocking in reverse order */
	unlockAll(locked, index);
	return current_inumber;
}

/*
 * Searches for a number in a array
 * Input:
 *  - array: array of numbers
 *  - length: max index to look for 
 *  - el: number to look for
 * Returns:
 *  SUCESS : found the number in the array
 *  FAIL: otherwise
 */
int linear_search(int *array, int length, int el)
{
	for (int i = 0; i < length; i++)
	{
		if (array[i] == el)
			return SUCCESS;
	}
	return FAIL;
}

/*
 * Lookup for a given path.
 * Input:
 *  - name: path of node
 * Returns:
 *  inumber: identifier of the i-node, if found
 *     FAIL: otherwise
 */
int aux_lookup(char *name, int *locked, int *index, int *already_locked, int already_locked_index)
{
	int locked_index = 0;
	char *saveptr;
	char full_path[MAX_FILE_NAME];
	char delim[] = "/";
	strcpy(full_path, name);

	/* start at root node */
	int current_inumber = FS_ROOT;

	/* use for copy */
	type nType;
	union Data data;

	/* First iteration outside the loop, for the root folder */
	char *path = strtok_r(full_path, delim, &saveptr);
	if (already_locked_index == 0 || linear_search(already_locked, already_locked_index, current_inumber) == FAIL)
	{
		if (path == NULL)
			inodeLock('w', current_inumber); /* Locking the root folder */
		else
			inodeLock('r', current_inumber); /* Locking the root folder */
		locked[locked_index++] = current_inumber;
	}
	inode_get(current_inumber, &nType, &data);

	/* search for all sub nodes */
	while (path != NULL && (current_inumber = lookup_sub_node(path, data.dirEntries)) != FAIL)
	{
		path = strtok_r(NULL, delim, &saveptr);
		/* Check if the node has been locked previously */
		if (already_locked_index == 0 || linear_search(already_locked, already_locked_index, current_inumber) == 0)
		{
			if (path == NULL)
				inodeLock('w', current_inumber);
			else
				inodeLock('r', current_inumber); /*  Locking all the nodes along the lookup path */
			locked[locked_index++] = current_inumber;
		}
		inode_get(current_inumber, &nType, &data);
	}
	*index = locked_index;
	return current_inumber;
}

/*
 * Prints tecnicofs tree.
 * Input:
 *  - fp: pointer to output file
 */
void print_tecnicofs_tree(FILE *fp)
{
	inode_print_tree(fp, FS_ROOT, "");
}
