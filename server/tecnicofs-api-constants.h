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

/* Generic error */
#define TECNICOFS_ERROR_OTHER -1

/* General Errors */
#define TECNICOFS_ERROR_CONNECTION_ERROR -2
#define TECNICOFS_ERROR_FILE_ALREADY_EXISTS -3
#define TECNICOFS_ERROR_FILE_NOT_FOUND -4

/* Create Specific */
#define TECNICOFS_ERROR_INVALID_PARENT_DIR -5
#define TECNICOFS_ERROR_PARENT_NOT_DIR -6
#define TECNICOFS_ERROR_COULDNT_ALLOCATE_INODE -7
#define TECNICOFS_ERROR_COULDNT_ADD_ENTRY -8
#define TECNICOFS_ERROR_INVALID_NODE_TYPE -9

/* Delete Specific */
#define TECNICOFS_ERROR_DOESNT_EXIST_IN_DIR -10
#define TECNICOFS_ERROR_DIR_NOT_EMPTY -11
#define TECNICOFS_ERROR_FAILED_REMOVE_FROM_DIR -12
#define TECNICOFS_ERROR_FAILED_DELETE_INODE -13

/* Move Specific */
#define TECNICOFS_ERROR_MOVE_TO_ITSELF -14
#define TECNICOFS_ERROR_MOUNT_ERROR -15

/* Print Specific */
#define TECNICOFS_ERROR_FILE_NOT_OPEN -16

#endif /* TECNICOFS_API_CONSTANTS_H */