#include <stdio.h>

#include <stdlib.h>

#include <fcntl.h> // for open()

#include <unistd.h> // for close()

#include <sys/stat.h> // for fstat()

#include <string.h>

#include <time.h>

#include <unistd.h> // for readlink()

#include <dirent.h> // for dir stuff

#include <libgen.h>

void getPermissions(struct stat fs) {
  printf("Owner: ");
  if (fs.st_mode & S_IRUSR)
    printf("read ");
  if (fs.st_mode & S_IWUSR)
    printf("write ");
  if (fs.st_mode & S_IXUSR)
    printf("execute");
  putchar(' ');

  printf("Group: ");
  if (fs.st_mode & S_IRGRP)
    printf("read ");
  if (fs.st_mode & S_IWGRP)
    printf("write ");
  if (fs.st_mode & S_IXGRP)
    printf("execute");
  putchar(' ');

  printf("Others: ");
  if (fs.st_mode & S_IROTH)
    printf("read ");
  if (fs.st_mode & S_IWOTH)
    printf("write ");
  if (fs.st_mode & S_IXOTH)
    printf("execute");
  putchar('\n');
}

void handleArgument(char * file) {
  printf("\nWe work on: %s ", file);
  struct stat buffer;
  lstat(file, & buffer);
  if (S_ISREG(buffer.st_mode) > 0) {
    printf("Regular File\n");
    printf("-n (file name)\n-d (file size)\n-h (number of hard links)\n-m (time of last modification)\n-a (access rights)\n-l (create symlink)\n-");
    char options[10];
    scanf("%s", options);
    int processes = 0;
    processes++;
    pid_t pid = fork();
    if (pid == 0) {
      printf("We read the following options: %s\n", options);
      for (int j = 0; j < strlen(options); j++) {
        char linkName[64];
        switch (options[j]) {
        case 'n':
          printf("file name: %s\n", file);
          break;
        case 'd':
          printf("file size: %lld bytes\n", buffer.st_size);
          break;
        case 'h':
          printf("number of hardlinks: %d\n", buffer.st_nlink);
          break;
        case 'm':
          printf("time of last modification: %s", ctime( & buffer.st_mtime));
          break;
        case 'a':
          printf("access rights: ");
          getPermissions(buffer);
          break;
        case 'l':
          printf("Name of the symlink you want to create: ");
          scanf("%s", linkName);
          symlink(file, linkName);
          break;
        default:
          printf("%c is an invalid option\n", options[j]);
          handleArgument(file);
        }
      }

      exit(0);
    }
    
    char * end = strrchr(file, '.');
    if (end && !strcmp(end, ".c")) {
      // new process
      processes++;
      pid = fork();
      // int status;
      if (pid == 0) {
        // printf("---New process!\n");
        execlp("/bin/zsh", "zsh", "checkErrors.sh", file, NULL);
        printf("error");
        exit(0);

      }
      
      //printf("---Process is done\n");
    }
    for(int i=0; i<processes; i++) {
      wait(0);
    }

  } else if (S_ISLNK(buffer.st_mode) > 0) {
    printf("Symbolic Link\n");
    printf("-n (link name)\n-d (link size)\n-t (target size)\n-a (access rights)\n-l (delete link)\n-");
    char options[10];
    scanf("%s", options);

    printf("We read the following options: %s\n", options);
    for (int j = 0; j < strlen(options); j++) {
      struct stat targetStat;
      switch (options[j]) {
      case 'n':
        printf("file name: %s\n", file);
        break;
      case 'd':
        printf("file size: %lld bytes\n", buffer.st_size);
        break;
      case 't':
        stat(file, & targetStat);
        printf("target size: %lld bytes\n", targetStat.st_size);
        break;
      case 'a':
        printf("access rights: ");
        getPermissions(buffer);
        break;
      case 'l':
        remove(file);
        printf("link deleted\n");
        break;
      default:
        printf("%c is an invalid option\n", options[j]);
        handleArgument(file);
      }
    }

  } else if (S_ISDIR(buffer.st_mode) > 0) {
    printf("(Directory)\n");
    printf("-n (directory name)\n-d (directory size)\n-a (access rights)\n-c (total number of .c files)\n-");
    char options[10];
    scanf("%s", options);
    // if all options are invalid -> recall the function
    pid_t pid = fork();
    if (pid == 0) {
      int n = 0, d = 0, a = 0, c = 0, err = 0; // used to make sure we don't display the same thing more than once
      printf("We read the following options: %s\n", options);
      for (int j = 0; j < strlen(options); j++) {
        if (options[j] == 'n' && n == 0) {
          printf("directory name: %s\n", file);
          n++;
        } else if (options[j] == 'd' && d == 0) {
          printf("directory size: %lld bytes\n", buffer.st_size);
          d++;
        } else if (options[j] == 'a' && a == 0) {
          printf("access rights: ");
          getPermissions(buffer);
          a++;
        } else if (options[j] == 'c' && c == 0) {
          DIR * dir = opendir(file);
          int nrOfFiles = 0;
          struct dirent * entry = readdir(dir);
          while (entry != NULL) {
            char * end = strrchr(entry -> d_name, '.');
            if (end && !strcmp(end, ".c")) {
              nrOfFiles++;
            }
            entry = readdir(dir);
          }
          printf("Number of .c files: %d\n", nrOfFiles);
          closedir(dir);
          c++;
        } else {
          printf("%c is an invalid option\n", options[j]);
          err++;
        }
      }
      exit(0);
    }
    wait(0);
    pid = fork();
    if(pid == 0) {
        execlp("/bin/zsh", "zsh", "handleDirectory.sh", file, NULL);
        printf("error");
        exit(0);
    }
    wait(0);
  }
}

int main(int argn, char * argv[]) {

  for (int i = 1; i < argn; i++) {
    handleArgument(argv[i]);
  }
  printf("\n");
  // int fileDescriptor = open(argv[1], O_RDONLY); // we don't need this yet
  // close(fileDescriptor);
  return 0;
}