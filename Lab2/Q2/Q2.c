#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  pid_t pid;
  for(int i = 0; i < 3; i++){
    pid = fork();
  }
  while(wait(NULL) > 0);
  if(pid > 0)
  {
    printf("I'm parent\n");
  }
  if(pid == 0){
    if(execlp("./programA","programA.c",NULL) != 0)
      {
        perror("execlp failed\n");
        exit(1);
      }
    }
  if(pid < 0){
    perror("fork failed");
    exit(1);
  }
  return 0;
}
