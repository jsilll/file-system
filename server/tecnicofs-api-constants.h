/* tecnicofs-api-constants.h */
#ifndef TECNICOFS_API_CONSTANTS_H
#define TECNICOFS_API_CONSTANTS_H

#define MAX_FILE_NAME 100
#define MAX_INPUT_SIZE 100

typedef enum permission
{
    NONE,
    WRITE,
    READ,
    RW
} permission;

typedef enum type
{
    T_FILE,
    T_DIRECTORY,
    T_NONE
} type;

#define SUCCESS 0

/* Client already has an open session with a TecnicoFS server */
#define TECNICOFS_ERROR_OPEN_SESSION -1
/* Doesn't exist an open session */
#define TECNICOFS_ERROR_NO_OPEN_SESSION -2
/* Communication failed */
#define TECNICOFS_ERROR_CONNECTION_ERROR -3
/* Already exists a file with the given name */
#define TECNICOFS_ERROR_FILE_ALREADY_EXISTS -4
/* No file found with the given name */
#define TECNICOFS_ERROR_FILE_NOT_FOUND -5
/* Client doesn't have permissions for the operation */
#define TECNICOFS_ERROR_PERMISSION_DENIED -6
/* Number of open files that can be open has been reached */
#define TECNICOFS_ERROR_MAXED_OPEN_FILES -7
/* File is not open */
#define TECNICOFS_ERROR_FILE_NOT_OPEN -8
/* File is open */
#define TECNICOFS_ERROR_FILE_IS_OPEN -9
/* File is open in the a mode that allows the operation */
#define TECNICOFS_ERROR_INVALID_MODE -10
/* Generic error */
#define TECNICOFS_ERROR_OTHER -11

/* Create */
/* Parent directory is invalid */
#define TECNICOFS_ERROR_INVALID_PARENT_DIR -12
/* Parent directory is not a directory */
#define TECNICOFS_ERROR_PARENT_NOT_DIR -13
/* Unable to allocate inode */
#define TECNICOFS_ERROR_COULDNT_ALLOCATE_INODE -14
/* Unable to add entry in dir */
#define TECNICOFS_ERROR_COULDNT_ADD_ENTRY -15
/* Invalid node type */
#define TECNICOFS_ERROR_INVALID_NODE_TYPE -16

/* Lookup only returns SUCCESS OR FAIL */

/* Delete */
/* File doesnt exist in dir */
#define TECNICOFS_ERROR_DOESNT_EXIST_IN_DIR -17
/* Directory isnt empty */
#define TECNICOFS_ERROR_DIR_NOT_EMPTY -18
/* Failed to remove from dir */
#define TECNICOFS_ERROR_FAILED_REMOVE_FROM_DIR -19
/* Failed to remove inode */
#define TECNICOFS_ERROR_FAILED_DELETE_INODE -20

#define TECNICOFS_ERROR_MOUNT_ERROR -21

#endif /* TECNICOFS_API_CONSTANTS_H */