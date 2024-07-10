#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int fd[2];
int flag = 1;
int number = 0;
/*
1) program tworzy dwa procesy potomne. Nastepnie proces macierzysty co sekund�
wysy�a SIGUSR1 do procesu potomnego 1. Proces potomny 1 po otrzymaniu sygna�u
przesy�a kolejn� liczb� (rozpoczynajac od zera) przez potok nienazwany do
procesu potomnego 2, kt�ry wyswietla te liczbe.

2) Po wcisnieciu ctrl-c (SIGINT) powinno nastapic przerwanie wysy�ania sygnalow.
Powtorne wcisniecie ctrl-c powinno wznowic wysylanie sygnalow*/

void sigusr1_handler(){
  int size = sizeof(int);
  write(fd[1], &size, sizeof(int));
  write(fd[1], &number, sizeof(int));
  number++;
}

void sigint_handler(){
  if (flag == 0){
    flag = 1;
  } else{
    flag = 0;
  }
}

int main (int lpar, char *tab[]){
  pid_t pid1, pid2;
  int d,i;
  pipe(fd);
  pid1 = fork();
  if (pid1<0){
    perror("fork");
    return 0;
  }else if (pid1==0){//proces 1
    close(fd[0]);
    while(1){
      signal(SIGUSR1, sigusr1_handler);
    }
    close(fd[1]);
    return 0;
  }else{
    pid2 = fork();
    if (pid2<0){
      perror("fork");
      return 0;
    }else if (pid2==0){//proces 2
      close(fd[1]);
      while(1){
        read(fd[0], &d, sizeof(int));
        read(fd[0], &i, sizeof(int));
        printf("przyjeto %d bajtow, wartosc:%d\n",d,i);
      }
      close(fd[0]);
      return 0;
    }
  }
  close(fd[0]);
  close(fd[1]);
  while(1) {
    signal(SIGINT, sigint_handler);
    if (flag == 1){
      kill(pid1, SIGUSR1);
      printf("wyslano SIGUSR1\n");
    } else{
      printf("wylaczono wysylanie sygnalow\n");
    } 
    sleep(1);
  }
}