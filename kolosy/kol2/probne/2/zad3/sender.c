#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define KEY "./queuekey"

int main(int argc, char* argv[])
{
 
 if(argc !=2){
   printf("Not a suitable number of program parameters\n");
   return(1);
 }

 /******************************************************
 Utworz kolejke komunikatow systemu V "reprezentowana" przez KEY
 Wyslij do niej wartosc przekazana jako parametr wywolania programu 
 Obowiazuja funkcje systemu V
 ******************************************************/
  key_t token = ftok(KEY, 1);
  if (token == (key_t)-1) {
      perror("Blad ftok\n");
      return 1;
  }

  int mq = msgget(token, IPC_CREAT | 0666);
  if (mq == -1) {
      perror("Blad msgget\n");
      return 1;
  }

  if (msgsnd(mq, argv[1], strlen(argv[1]) + 1, 0) < 0) {
    perror("Blad msgsnd\n");
		return 1;
  }

  return 0;
}



