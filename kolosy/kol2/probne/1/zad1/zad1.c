#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#define L_SLOW 8

pthread_mutex_t mutex01 = PTHREAD_MUTEX_INITIALIZER;

char slownik[L_SLOW][10]={"alfa","bravo","charlie","delta","echo","foxtrot","golf","hotel"};
int NR=0;

void* fun_watka(void* parametr) {
  //zajmij mutex 'mutex01'
  //...
  pthread_mutex_lock(&mutex01);

  printf("%s ", slownik[NR++]);    fflush(stdout);
  if (NR>=L_SLOW) NR=0;
  //zwolnij mutex 'mutex01'
  //...
  pthread_mutex_unlock(&mutex01);

  sleep(1);
}

int main(void){
  int i;
  //Utworz 20 watkow realizujacych funkcje 'fun_watka'
  //...
  const int liczba_watkow = 20;

  pthread_t watki[liczba_watkow];
  for (int i = 0; i < liczba_watkow; i++) {
    pthread_create(&watki[i], NULL, &fun_watka, NULL);
  }

  //poczekaj na zakonczenie wszystkich watkow
  //...
  for (int i = 0; i < liczba_watkow; i++) {
    pthread_join(watki[i], NULL);
  }

  printf("\n");

}
