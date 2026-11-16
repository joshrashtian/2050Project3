#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define MAX 1024

struct inode {
  uint32_t inode;
  uint32_t parent;
  char type;
  char name[32];
};

/*
 * PROJECT REQUIREMENTS
 *  exit (done)
 *  cd <name> (done)
 *  ls (done)
 *  mkdir <name>
 *  touch <name>
 */

int get_cmd(char *string) {
  if (string == NULL)
    exit(1);
  char command[64];
  int i = 0;
  while (*string != ' ' && *string != '\0' && *string != '\n' && i < 63) {
    command[i] = *string;
    string++;
    i++;
  }
  command[i] = '\0';

  if (strcmp(command, "exit") == 0)
    return 0;
  if (strcmp(command, "cd") == 0)
    return 1;
  if (strcmp(command, "ls") == 0)
    return 2;
  if (strcmp(command, "mkdir") == 0)
    return 3;
  if (strcmp(command, "touch") == 0)
    return 4;

  return -1;
}

char *get_argument(char *ns) {
  if (ns == NULL)
    exit(1);
  // since we will only ever need one argument, we can outright just start
  // aftert the first space.
  while (*ns == ' ')
    ns++;
  while (*ns != '\0' && *ns != ' ')
    ns++; // skip the cmd
  while (*ns == ' ')
    ns++; // spaces before arg

  if (*ns == '\0' || *ns == '\n')
    return NULL;

  char *start = ns;
  while (*ns != '\0' && *ns != ' ' && *ns != '\n')
    ns++;
  *ns = '\0';
  return start;
}

int cd(struct inode *fs, uint32_t *cwd, char *name) {
  if (name == NULL) {
    perror("Missing name");
    return -1;
  }
  if (!strcmp(name, ".")) {
    return 0;
  } else if (!strcmp(name, "..")) {
    *cwd = fs[*cwd].parent;
    return 0;
  }

  // look for a directory named `name` inside the current one
  for (int i = 0; i < MAX; i++) {
    if (fs[i].type == 'd' && fs[i].parent == *cwd &&
        fs[i].inode != fs[i].parent && !strcmp(fs[i].name, name)) {
      *cwd = fs[i].inode;
      return 0;
    }
  }

  // if no dir found
  fprintf(stderr, "cd: %s: no such directory\n", name);
  return -1;
}

int makeNode(struct inode *fs, uint32_t cwd, char *name, char type) {
  if (name == NULL) {
    fprintf(stderr, "%s: missing name\n", type == 'd' ? "mkdir" : "touch");
    return -1;
  }
  if (strlen(name) >= sizeof fs[0].name) {
    fprintf(stderr, "%s: name too long\n", name);
    return -1;
  }

  // making sure the node doesn't already exist
  for (int i = 0; i < MAX; i++) {
    if ((fs[i].type == 'd' || fs[i].type == 'f') && fs[i].parent == cwd &&
        fs[i].inode != fs[i].parent && !strcmp(fs[i].name, name)) {
      fprintf(stderr, "%s: already exists\n", name);
      return -1;
    }
  }

  int n = -1;
  for (int i = 0; i < MAX; i++) {
    if (fs[i].type != 'd' && fs[i].type != 'f') {
      n = i;
      break;
    }
  }
  if (n == -1) {
    fprintf(stderr, "no free inodes\n");
    return -1;
  }

  char path[16];
  sprintf(path, "%d", n);
  FILE *fp = fopen(path, "w");

  // check if directory or file; directory needs something files do not
  if (!fp) {
    perror(path);
    return -1;
  }
  if (type == 'd')
    fprintf(fp, "%d .\n%u ..\n", n, cwd); // directory: its . and .. inodes
  else
    fprintf(fp, "%s", name);
  fclose(fp);

  struct inode newNode;
  memset(&newNode, 0, sizeof newNode);
  newNode.inode = n;
  newNode.parent = cwd;
  newNode.type = type;
  strcpy(newNode.name, name);
  fs[n] = newNode;
  return 0;
}

void ls(struct inode *fs, uint32_t cwd) {
  for (int i = 0; i < MAX; i++) {
    if ((fs[i].type == 'd' || fs[i].type == 'f') && // skip empty slots
        fs[i].parent == cwd &&                      // only this directory
        fs[i].inode != fs[i].parent) {              // skip root's self-entry
      printf("inode: %u, type: %c, filename: %s\n", fs[i].inode, fs[i].type,
             fs[i].name);
    }
  }
}

void loadNodes(struct inode *fs, FILE *f) {
  struct inode e;
  while (fread(&e, sizeof e, 1, f) == 1) {
    if (e.inode >= MAX)
      continue;
    if (e.type != 'd' && e.type != 'f')
      continue;
    fs[e.inode] = e;
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: <dir>");
    exit(EXIT_FAILURE);
  }
  int running = 1;
  // for now, just make the buffer 64
  char buf[64];
  char *dir = argv[1];

  if (chdir(dir) != 0) {
    perror(dir);
    exit(1);
  }

  struct inode fs[MAX] = {0};
  FILE *f = fopen("inodes_list", "rb");
  if (f == NULL) {
    perror("inodes_list");
    exit(EXIT_FAILURE);
  }
  loadNodes(fs, f);
  fclose(f);

  if (fs[0].type != 'd') {
    fprintf(stderr, "inode 0 is not a directory\n");
    exit(EXIT_FAILURE);
  }

  uint32_t cwd = 0;

  while (running) {
    printf("> ");
    if (!fgets(buf, sizeof buf, stdin))
      break;
    int outcome = get_cmd(buf);
    char *args = get_argument(buf);

    switch (outcome) {
    case 0:
      running = 0;
      break;
    case 1:
      cd(fs, &cwd, args);
      break;
    case 2:
      ls(fs, cwd);
      break;
      break;
    case 3:
      makeNode(fs, cwd, args, 'd');
      break;
    case 4:
      makeNode(fs, cwd, args, 'f');
      break;
    default:
      fprintf(stderr, "Command doesnt exist;\n");
      break;
    }
  }

  FILE *fp = fopen("inodes_list", "wb");

  if (!fp) {
    perror("Error saving file");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < MAX; i++) {
    if (fs[i].type == 'd' || fs[i].type == 'f') {
      if (fwrite(&fs[i], sizeof fs[i], 1, fp) != 1) {
        perror("FWrite Error");
        fclose(fp);
        return -1;
      }
    }
  }

  fclose(fp);

  return 0;
}
