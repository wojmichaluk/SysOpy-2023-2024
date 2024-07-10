#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define KEY  "./queuekey"


int main() {
        sleep(1);
        int val = 0;


	/**********************************
	Otworz kolejke systemu V "reprezentowana" przez KEY
	**********************************/
	key_t token = ftok(KEY, 1);
	if (token == (key_t)-1) {
		perror("Blad ftok\n");
		return 1;
	}

	int mq = msgget(token, 0666);
	if (mq == -1) {
		perror("Blad msgget\n");
		return 1;
	}
	
	while(1)
 	{	
	    /**********************************
	    Odczytaj zapisane w kolejce wartosci i przypisz je do zmiennej val
	    obowiazuja funkcje systemu V
	    ************************************/
	   if (msgrcv(mq, (void*)&val, sizeof(int), 0, 0) <= 0) {
			perror("Blad msgrcv\n");
			return 1;
	   }
        	 printf("%d square is: %d\n", val, val*val);
 
	}

	/*******************************
	posprzataj
	********************************/
	msgctl(mq, IPC_RMID, NULL);

     return 0;
   }
