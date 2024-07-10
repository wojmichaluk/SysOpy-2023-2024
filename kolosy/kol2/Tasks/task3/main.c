#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>

void* hello(void* arg){
        // ustawienia do przerwania watku
        int status = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
        if (status != 0) {
                perror("Blad pthread_setcanceltype\n");
                return NULL;
        }
        
        sleep(1);
        while(1){
                printf("Hello world from thread number %d\n", *(int*)arg);
		/****************************************************
			przerwij dzialanie watku jesli bylo takie zadanie
		*****************************************************/                
                printf("Hello again world from thread number %d\n", *(int*)arg);
                sleep(2);
        }
        return NULL;
}


int main(int argc, char** args){

       if(argc !=3){
	    printf("Not a suitable number of program parameters\n");
    	return(1);
   	}

        int n = atoi(args[1]);

	/**************************************************
	    Utworz n watkow realizujacych funkcje hello
	**************************************************/
        pthread_t threads[n];
        int thread_nos[n];

        for (int i = 0; i < n; i++) {
                thread_nos[i] = i + 1;
                pthread_create(&threads[i], NULL, &hello, (void*)(thread_nos + i));
        }

        int i = 0;
        while(i++ < atoi(args[2])) {
                printf("Hello from main %d\n",i);
                sleep(2);
        }
        
        i = 0;

	/*******************************************
	    "Skasuj" wszystke uruchomione watki i poczekaj na ich zakonczenie
	*******************************************/
        while (i++ < n) {
                pthread_cancel(threads[i - 1]);
                pthread_join(threads[i - 1], NULL);
        }
        printf("DONE");
        
        
        return 0;
}

