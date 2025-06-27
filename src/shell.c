#include "shell.h"
#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void shell() {
  char buf[64];
  char cmd[64];
  char arg[2][64];

  byte cwd = FS_NODE_P_ROOT;

  while (true) {
    printString("MengOS:");
    printCWD(cwd);
    printString("$ ");
    readString(buf);
    parseCommand(buf, cmd, arg);

    if (strcmp(cmd, "cd") == 0) cd(&cwd, arg[0]);
    else if (strcmp(cmd, "ls") == 0) ls(cwd, arg[0]);
    else if (strcmp(cmd, "mv")) mv(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cp")) cp(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cat")) cat(cwd, arg[0]);
    else if (strcmp(cmd, "mkdir")) mkdir(cwd, arg[0]);
    else if (strcmp(cmd, "clear")) clearScreen();
    else printString("Invalid command\n");
  }
}

// TODO: 4. Implement printCWD function
void printCWD(byte cwd) {
    struct node_fs fs;
    byte stack[FS_MAX_NODE];
    int count = 0;
    byte cur;
    int i;

    readSector((byte*)&fs.nodes[0], FS_NODE_SECTOR_NUMBER);
    readSector((byte*)&fs.nodes[32], FS_NODE_SECTOR_NUMBER + 1);


    if (cwd == FS_NODE_P_ROOT) {
        printString("/");
        return;
    }
    cur = cwd;
    while (cur != FS_NODE_P_ROOT) {
        stack[count++] = cur;
        cur = fs.nodes[cur].parent_index;
    }
    for (i = count - 1; i >= 0; i--) {
        printString("/");
        printString(fs.nodes[ stack[i] ].node_name);
    }
}


// TODO: 5. Implement parseCommand function
void parseCommand(char* buf, char* cmd, char arg[2][64]) {
    int i;
    int j;
    int len;
    int stage;
    char* target;
    len = strlen(buf);
    stage = 0; 

    cmd[0]     = '\0';
    arg[0][0]  = '\0';
    arg[1][0]  = '\0';

    while (i < len && stage <= 2) {
        while (i < len && buf[i] == ' ') i++;
        if (i >= len) break;

        if (stage == 0) target = cmd;
        else if (stage == 1) target = arg[0];
        else target = arg[1];

        j = 0;
        while (i < len && buf[i] != ' ' && j < 63) {
            target[j++] = buf[i++];
        }
        target[j] = '\0';

        stage++;
    }
}


// TODO: 6. Implement cd function
void cd(byte* cwd, char* dirname) {
    struct node_fs fs;
    int i;

    // Baca seluruh node table
    readSector((byte*)&fs.nodes[0], FS_NODE_SECTOR_NUMBER);
    readSector((byte*)&fs.nodes[32], FS_NODE_SECTOR_NUMBER + 1);

    if (strcmp(dirname, "/") == 0) {
        *cwd = FS_NODE_P_ROOT;
        return;
    }

    if (strcmp(dirname, "..") == 0) {
        if (*cwd != FS_NODE_P_ROOT) {
            *cwd = fs.nodes[*cwd].parent_index;
        }
        return;
    }
    for (i = 0; i < FS_MAX_NODE; i++) {
        struct node_item *node = &fs.nodes[i];
        if (node->parent_index   == *cwd &&
            node->data_index     == FS_NODE_S_IDX_FOLDER &&
            strcmp(node->node_name, dirname) == 0) {
            *cwd = i;
            return;
        }
    }

}

// TODO: 7. Implement ls function
void ls(byte cwd, char* dirname) {
    struct node_fs node_fs_buf;
    byte target = cwd;
    int i;

    readSector(&(node_fs_buf.nodes[0]), FS_NODE_SECTOR_NUMBER);
    readSector(&(node_fs_buf.nodes[32]), FS_NODE_SECTOR_NUMBER+1);

    if (dirname[0] != '\0' && strcmp(dirname, ".") != 0) {
        for (i = 0; i < FS_MAX_NODE; i++) {
            if (node_fs_buf.nodes[i].parent_index == cwd &&
                node_fs_buf.nodes[i].data_index  == FS_NODE_D_DIR &&
                strcmp(node_fs_buf.nodes[i].node_name, dirname) == 0) {
                target = i;
                break;
            }
        }
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (node_fs_buf.nodes[i].parent_index== target) {
            printString(node_fs_buf.nodes[i].node_name);
            printString("\n");
        }
    }
}




// TODO: 8. Implement mv function
void mv(byte cwd, char* src, char* dst) {}

// TODO: 9. Implement cp function
void cp(byte cwd, char* src, char* dst) {}

// TODO: 10. Implement cat function
void cat(byte cwd, char* filename) {
  struct file_metadata meta;
  enum fs_return status;
  int i;
  meta.parent_index = cwd;

  strcpy(meta.node_name, filename);
  fsRead(&meta, &status);
  if (status == FS_SUCCESS) {
    for (i = 0; i < meta.filesize; i++) {
      printString(meta.buffer[i]);  
    }
  }
}

// TODO: 11. Implement mkdir function
void mkdir(byte cwd, char* dirname) {
    struct node_fs fs;
    int i;
    int empty;
    empty = -1;

    readSector((byte*)&fs.nodes[0], FS_NODE_SECTOR_NUMBER);
    readSector((byte*)&fs.nodes[32], FS_NODE_SECTOR_NUMBER + 1);


    if (dirname[0]=='\0' || strlen(dirname) >= MAX_FILENAME) {
        printString("mkdir tidak valdi\n");
        return;
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (fs.nodes[i].parent_index == cwd && fs.nodes[i].data_index   == FS_NODE_S_IDX_FOLDER &&
            strcmp(fs.nodes[i].node_name, dirname) == 0) {
            printString("mkdir: direktori sudah ada\n");
            return;
        }
    }

    for (i = 0; i < FS_MAX_NODE; i++) {
        if (fs.nodes[i].node_name[0] == '\0') {
            empty = i;
            break;
        }
    }

    fs.nodes[empty].parent_index = cwd;
    fs.nodes[empty].data_index   = FS_NODE_S_IDX_FOLDER;
    strcpy(fs.nodes[empty].node_name, dirname);

    writeSector((byte*)&fs, FS_NODE_SECTOR_NUMBER);
}


