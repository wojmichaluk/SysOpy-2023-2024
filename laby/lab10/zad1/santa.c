#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

typedef struct arguments {
	pthread_mutex_t *mut;
	pthread_cond_t  *con1;
	pthread_cond_t  *con2;
	int *ret;
	int *additional_data;
} args;

#define REINDEERS 9
#define MAX_RIDES 4

void* santa_routine(void*);
void* reindeer_routine(void*);

int main(int argc, char *argv[]) {
	pthread_t threads[REINDEERS + 1];
	args arguments[REINDEERS + 1];
	int ids [REINDEERS];

	int returned = 0;
	int rides = 0;
	srand(time(NULL));

	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond1  = PTHREAD_COND_INITIALIZER;
	pthread_cond_t cond2  = PTHREAD_COND_INITIALIZER;
	
	// santa thread
	arguments[0].mut = &mutex;
	arguments[0].con1 = &cond1;
	arguments[0].con2 = &cond2;
	arguments[0].ret = &returned;
	arguments[0].additional_data = &rides;

	if (pthread_create(&threads[0], NULL, &santa_routine, (void*)&arguments[0]) != 0) {
		perror("Blad przy tworzeniu watku Mikolaja!");
		pthread_cond_destroy(&cond1);
		pthread_cond_destroy(&cond2);
		return 1;
	}

	// reindeer threads
	for (int i = 1; i < REINDEERS + 1; i++) {
		arguments[i].mut = &mutex;
		arguments[i].con1 = &cond1;
		arguments[i].con2 = &cond2;
		arguments[i].ret = &returned;

		ids[i - 1] = i;
		arguments[i].additional_data = &ids[i - 1];

		if (pthread_create(&threads[i], NULL, &reindeer_routine, (void*)&arguments[i]) != 0) {
			perror("Blad przy tworzeniu watku renifera!");
			pthread_cond_destroy(&cond1);
			pthread_cond_destroy(&cond2);
			return 1;
		}
	}

	printf("\nWatek glowny: Na poczatku Mikolaj spi, a renifery sa na wakacjach w cieplych krajach!\n\n");
	while (rides < MAX_RIDES);

	for (int i = 0; i < REINDEERS + 1; i++) {
		pthread_cancel(threads[i]);
	}

	pthread_cond_destroy(&cond1);
	pthread_cond_destroy(&cond2);
	printf("Watek glowny: Mikolaj rozwiozl zabawki juz %d razy, pora na urlop!\n\n", rides);
	
	return 0;
}

void* santa_routine(void* arg) {
	int scs = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	int sct = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	if (scs != 0 || sct != 0) {
		perror("Blad w ustawianiu reakcji na przerwanie watku Mikolaja!");
		pthread_exit(NULL);
	}

	args arguments = *(args*)arg;
	pthread_mutex_t *mutex = arguments.mut;
	pthread_cond_t  *cond1 = arguments.con1;
	pthread_cond_t  *cond2 = arguments.con2;
	int *returned = arguments.ret;
	int *rides = arguments.additional_data;

	while (true) {
		pthread_mutex_lock(mutex);
		while (*returned < REINDEERS) {
			pthread_cond_wait(cond1, mutex);
		}

		printf("Mikolaj: budze sie\n");
		printf("Mikolaj: dostarczam zabawki\n");
		sleep(rand() % 3 + 2); // sleep 2 to 4 seconds
		printf("Mikolaj: zasypiam\n\n");

		*returned = 0;
		*rides += 1;
		pthread_cond_broadcast(cond2);
		pthread_mutex_unlock(mutex);
	}

	return NULL;
}

void* reindeer_routine(void* arg) {
	int scs = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	int sct = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	if (scs != 0 || sct != 0) {
		perror("Blad w ustawianiu reakcji na przerwanie watku renifera!");
		pthread_exit(NULL);
	}

	args arguments = *(args*)arg;
	pthread_mutex_t *mutex = arguments.mut;
	pthread_cond_t  *cond1 = arguments.con1;
	pthread_cond_t  *cond2 = arguments.con2;
	int *returned = arguments.ret;
	int id = *arguments.additional_data;

	while (true) {
		sleep(rand() % 6 + 5); // sleep 5 to 10 seconds
		pthread_mutex_lock(mutex);
		*returned += 1;
		printf("Renifer: czeka %d reniferow na Mikolaja, %d\n", *returned, id);

		if (*returned == REINDEERS) {
			printf("Renifer: wybudzam Mikolaja, %d\n", id);
			pthread_cond_broadcast(cond1);
		}
		
		while (*returned > 0) {
			pthread_cond_wait(cond2, mutex);
		}
		pthread_mutex_unlock(mutex);
	}

	return NULL;
}
