#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <libgen.h>

/* Read permissions info of file */
void getPermissions(struct stat fs)
{
  printf("\tOwner:\n");
  if (fs.st_mode & S_IRUSR)
    printf("\t  read - yes | ");
  else
    printf("\tread - no | ");
  if (fs.st_mode & S_IWUSR)
    printf("write - yes | ");
  else
    printf("write - no | ");
  if (fs.st_mode & S_IXUSR)
    printf("execute - yes ");
  else
    printf("execute - no");
  putchar('\n');

  printf("\tGroup:\n");
  if (fs.st_mode & S_IRGRP)
    printf("\t  read - yes | ");
  else
    printf("\tread - no | ");
  if (fs.st_mode & S_IWGRP)
    printf("write - yes | ");
  else
    printf("write - no | ");
  if (fs.st_mode & S_IXGRP)
    printf("execute - yes ");
  else
    printf("execute - no");
  putchar('\n');

  printf("\tOthers:\n");
  if (fs.st_mode & S_IROTH)
    printf("\t  read - yes | ");
  else
    printf("\tread - no | ");
  if (fs.st_mode & S_IWOTH)
    printf("write - yes | ");
  else
    printf("write - no | ");
  if (fs.st_mode & S_IXOTH)
    printf("execute - yes ");
  else
    printf("execute - no");
  putchar('\n');
}

/* Compute score for C files */
int computeScore(int errors, int warnings)
{
  if (errors >= 1)
  {
    return 1;
  }
  if (errors == 0 && warnings > 10)
  {
    return 2;
  }
  if (errors == 0 && warnings < 10)
  {
    return 2 + 8 * (10 - warnings) / 10;
  }
  return 10;
}

/* Extract file/dir name from path */
char *getName(char path[])
{
  if (strrchr(path, '/') == NULL)
  {
    return path;
  }
  else
  {
    return strrchr(path, '/') + 1;
  }
}

/* Handle path received as argument */
void handleArgument(char *file)
{
  printf("\nWe work on: %s ", file);
  struct stat buffer;
  lstat(file, &buffer);

  /* Handling REGULAR FILES */
  if (S_ISREG(buffer.st_mode) > 0)
  {
    printf("Regular File\n");
    printf("-n (file name)\n-d (file size)\n-h (number of hard links)\n-m (time of last modification)\n-a (access rights)\n-l (create symlink)\n");
    char options[10];
    scanf("%s", options);
    int processes = 0;
    processes++;
    // Process 1
    pid_t pid1 = fork();
    if (pid1 < 0)
    {
      perror("Child process creation error\n");
      exit(1);
    }
    if (pid1 == 0)
    {
      printf("\nWe read the following options: %s\n", options);
      if (options[0] != '-')
      {
        printf("ERROR: please use the following format: -option1option2\n");
        exit(2);
      }
      for (int j = 1; j < strlen(options); j++)
      {
        char linkName[64];
        switch (options[j])
        {
        case 'n':
          printf(" - File name: %s\n", getName(file));
          break;
        case 'd':
          printf(" - File size: %lld bytes\n", buffer.st_size);
          break;
        case 'h':
          printf(" - Number of hardlinks: %d\n", buffer.st_nlink);
          break;
        case 'm':
          printf(" - Time of last modification: %s", ctime(&buffer.st_mtime));
          break;
        case 'a':
          printf(" - Access rights:\n");
          getPermissions(buffer);
          break;
        case 'l':
          printf("!! TYPE the name of the symlink you want to create: ");
          scanf("%s", linkName);
          symlink(file, linkName);
          break;
        default:
          printf("%c is an invalid option\n", options[j]);
          exit(3);
        }
      }
      exit(0);
    }
    // Create pipe
    int pfd[2];
    if (pipe(pfd) < 0)
    {
      perror("Pipe creation error\n");
      exit(1);
    }
    // Process 2
    processes++;
    pid_t pid2 = fork();
    if (pid2 < 0)
    {
      perror("Child process creation error\n");
      exit(1);
    }
    if (pid2 == 0)
    {
      printf("Starting 2nd child proccess\n");
      char *end = strrchr(file, '.');
      if (end && !strcmp(end, ".c"))
      {
        // Handling C regular files
        close(pfd[0]);
        if (dup2(pfd[1], 1) < 0)
        {
          perror("Error when calling dup2\n");
          exit(1);
        }
        execlp("/bin/sh", "sh", "checkErrors.sh", file, NULL);
        printf("Something went wrong with execlp\n");
        exit(2);
      }
      else
      {
        // Handling normal regular files
        FILE *fp = fopen(file, "r");
        char c = getc(fp);
        int lines = 0;
        while (c != EOF)
        {
          if (c == '\n')
          {
            lines++;
          }
          c = getc(fp);
        }
        printf(" - Number of lines: %d\n", lines);
        exit(0);
      }
    }
    // Read from pipe and compute score if C file
    char *end = strrchr(file, '.');
    if (end && !strcmp(end, ".c"))
    {
      close(pfd[1]);
      FILE *stream = fdopen(pfd[0], "r");
      char strAux[500];
      int works = 1;
      int store = 0;
      char errorsWarnings[3];
      while (fscanf(stream, "%s", strAux) && works == 1) // a more "formatted" way of reading so we read exactly what we need in one go
      {
        if (strcmp(strAux, "((stop))") == 0)
        {
          store = 0;
          works = 0;
        }
        else if (strcmp(strAux, "((start))") == 0)
        {
          store = 1;
        }
        else if (store == 1)
        {
          strcat(errorsWarnings, strAux);
          fscanf(stream, "%s", strAux);
          fscanf(stream, "%s", strAux);
          strcat(errorsWarnings, strAux);
          fscanf(stream, "%s", strAux);
        }
      }
      close(pfd[0]);

      printf(" - We have %c errors, %c warnings\n", errorsWarnings[0], errorsWarnings[1]);
      int score = computeScore(errorsWarnings[0] - '0', (int)errorsWarnings[1] - '0');
      printf("   SCORE: %d\n", score);
      FILE *gradesFile = fopen("grades.txt", "w");
      if (gradesFile == NULL)
      {
        printf("Could not open grades file");
      }
      fprintf(gradesFile, "%s : %d\n", strrchr(file, '/') == NULL ? file : strrchr(file, '/') + 1, score);
      fclose(gradesFile);
    }
    // Receive and print return states of child processes & wait for them to finish
    for (int i = 0; i < processes; i++)
    {
      int stat;
      int pd = wait(&stat);
      int exit = WEXITSTATUS(stat);
      printf(" * The process with PID %d has ended with exit code %d", pd, exit);
      if (exit == 2 && pd == pid1)
      {
        handleArgument(file);
      }
      if (exit == 3 && pd == pid1)
      {
        handleArgument(file);
      }
    }
  }

  /* Handling SYMBOLIC LINKS */
  else if (S_ISLNK(buffer.st_mode) > 0)
  {
    printf("Symbolic Link\n");
    printf("-n (link name)\n-d (link size)\n-t (target size)\n-a (access rights)\n-l (delete link)\n");
    char options[10];
    scanf("%s", options);
    int processes = 0;
    processes++;
    // Process 1
    pid_t pid1 = fork();
    if (pid1 < 0)
    {
      perror("Child process creation error\n");
      exit(1);
    }
    if (pid1 == 0)
    {
      printf("We read the following options: %s\n", options);
      if (options[0] != '-')
      {
        printf("ERROR: please use the following format: -option1option2\n");
        exit(2);
      }
      if (strchr(options, 'l') != NULL)
      {
        remove(file);
        printf(" - Link deleted!\n");
      }
      else
      {
        for (int j = 1; j < strlen(options); j++)
        {
          struct stat targetStat;

          switch (options[j])
          {
          case 'n':
            printf(" - File name: %s\n", getName(file));
            break;
          case 'd':
            printf(" - File size: %lld bytes\n", buffer.st_size);
            break;
          case 't':
            stat(file, &targetStat);
            printf(" - Target size: %lld bytes\n", targetStat.st_size);
            break;
          case 'a':
            printf(" - Access rights:\n");
            getPermissions(buffer);
            break;
          case 'l':
            remove(file);
            printf(" - Link deleted!\n");
            break;
          default:
            printf("%c is an invalid option\n", options[j]);
            exit(3);
          }
        }
      }
      exit(0);
    }
    // Process 2
    processes++;
    pid_t pid2 = fork();
    if (pid2 < 0)
    {
      perror("Child process creation error\n");
      exit(1);
    }
    if (pid2 == 0)
    {
      printf("Second child process starts\n");
      execlp("chmod", "chmod", "-R", "760", file, NULL);
      printf("Something went wrong with execlp\n");
      exit(1);
    }
    // Receive and print return states of child processes & wait for them to finish
    for (int i = 0; i < processes; i++)
    {
      int stat;
      int pd = wait(&stat);
      int exit = WEXITSTATUS(stat);
      printf(" * The process with PID %d has ended with exit code %d", pd, exit);
      if (exit == 2 && pd == pid1)
      {
        handleArgument(file);
      }
      if (exit == 3 && pd == pid1)
      {
        handleArgument(file);
      }
    }
  }

  /* Handling DIRECTORIES */
  else if (S_ISDIR(buffer.st_mode) > 0)
  {
    printf("Directory\n");
    printf("-n (directory name)\n-d (directory size)\n-a (access rights)\n-c (total number of .c files)\n");
    char options[10];
    scanf("%s", options);
    int processes = 0;
    processes++;
    // Process 1
    pid_t pid1 = fork();
    if (pid1 < 0)
    {
      perror("Child process creation error\n");
      exit(1);
    }
    if (pid1 == 0)
    {
      printf("We read the following options: %s\n", options);
      if (options[0] != '-')
      {
        printf("ERROR: please use the following format: -option1option2\n");
        exit(2);
      }
      for (int j = 1; j < strlen(options); j++)
      {
        switch (options[j])
        {
        case 'n':
          printf(" - Directory name: %s\n", getName(file));
          break;
        case 'd':
          printf(" - Directory size: %lld bytes\n", buffer.st_size);
          break;
        case 'a':
          printf(" - Access rights:\n");
          getPermissions(buffer);
          break;
        case 'c':
        {
          DIR *dir = opendir(file);
          int nrOfFiles = 0;
          struct dirent *entry = readdir(dir);
          while (entry != NULL)
          {
            char *end = strrchr(entry->d_name, '.');
            if (end && !strcmp(end, ".c"))
            {
              nrOfFiles++;
            }
            entry = readdir(dir);
          }
          printf(" - Number of .c files: %d\n", nrOfFiles);
          closedir(dir);
          break;
        }
        default:
          printf("%c is an invalid option\n", options[j]);
          exit(3);
        }
      }
      exit(0);
    }
    // Process 2
    processes++;
    pid_t pid2 = fork();
    if (pid2 < 0)
    {
      perror("Child process creation error\n");
      exit(1);
    }
    if (pid2 == 0)
    {
      execlp("touch", "touch", strcat(getName(file), "_file.txt"), NULL);
      printf("Something went wrong with execlp\n");
      exit(1);
    }
    // Receive and print return states of child processes & wait for them to finish
    for (int i = 0; i < processes; i++)
    {
      int stat;
      int pd = wait(&stat);
      int exit = WEXITSTATUS(stat);
      printf(" * The process with PID %d has ended with exit code %d\n", pd, exit);
      if (exit == 2 && pd == pid1)
      {
        handleArgument(file);
      }
      if (exit == 3 && pd == pid1)
      {
        handleArgument(file);
      }
    }
  }
}

int main(int argn, char *argv[])
{
  for (int i = 1; i < argn; i++)
  {
    handleArgument(argv[i]);
  }
  printf("\n");

  return 0;
}