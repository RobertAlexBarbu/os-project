#include <stdio.h>
#include <fcntl.h> // for open()
#include <unistd.h> // for close()
#include <sys/stat.h> // for fstat()
#include <string.h>

int main(int argn, char* argv[]) {
  struct stat buffer;
  for(int i=1; i<argn; i++) {
    printf("We work on: %s\n", argv[1]);
    lstat(argv[i], &buffer);
    if(S_ISREG(buffer.st_mode) > 0) {
      printf("Regular File\n");
      printf("-n (file name)\n-d (file size)\n-a (access rights)\n-");
      char options[10];
      scanf("%s", options);
      printf("We read the following options: %s\n", options);
      for(int i=0; i<strlen(options); i++) {
        switch(options[i]) {
          case 'n':
          printf("file name: \n");
          break;
          case 'd':
          printf("file size: \n");
          break;
          case 'a':
          printf("access rights: \n");
          break;
          default:
          printf("%c is an invalid option\n", options[i]);
        }
      }
    } else if(S_ISLNK(buffer.st_mode) > 0) {
      printf("Symbolic Link\n");
      printf("-n (link name)\n-d (link size)\n-a (access rights)\n-");
      char options[10];
      scanf("%s", options);
      printf("We read the following options: %s\n", options);
    }
  }
  
  // int fileDescriptor = open(argv[1], O_RDONLY); // we don't need this yet
  // close(fileDescriptor);
  return 0;
}