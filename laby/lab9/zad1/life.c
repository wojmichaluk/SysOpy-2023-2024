#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

// Values copied from grid.c
#define GRID_WIDTH 30
#define GRID_HEIGHT 30

typedef struct arguments {
	int begin;
	int end;
	char *background;
	char *foreground;
} args;

void handle_signal() {}

int invalid_parameter(char*);
void* thread_routine(void*);

int main(int argc, char **argv) {
	if (argc != 2 || invalid_parameter(argv[1])) {
		printf("Nieprawidlowe argumenty uruchomienia programu!\n");
        printf("Prawidlowa forma: `./life <liczba_watkow>`, gdzie\n");
        printf("1 <= <liczba_watkow> <= <szerokosc_planszy> x <wysokosc_planszy>,\n");
		printf("obecnie gorne ograniczenie wynosi 30 x 30 = 900\n");
        return 1;
	}

	int n = (int)strtol(argv[1], NULL, 10);
	pthread_t threads[n];
	args arguments[n];

	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

	char *foreground = create_grid();
	char *background = create_grid();
	char *tmp;

	if (foreground == NULL || background == NULL) {
		perror("Funkcja `malloc` nie zwrocila poprawnego adresu!");
		return 1;
	}

	init_grid(foreground);
	signal(SIGUSR1, handle_signal);

	for (int i = 0; i < n; i++) {
		int begin = (int)(GRID_WIDTH * GRID_HEIGHT * i / n);
		int end = (int)(GRID_WIDTH * GRID_HEIGHT * (i + 1) / n);

		arguments[i].begin = begin;
		arguments[i].end = end;
		arguments[i].foreground = foreground;
		arguments[i].background = background;

		if (pthread_create(&threads[i], NULL, &thread_routine, (void*)&arguments[i]) != 0) {
			perror("Blad przy tworzeniu watku!");
			return 1;
		}
	}

	while (true) {
		draw_grid(foreground);
		usleep(500 * 1000);

		// Step simulation
		/*update_grid(foreground, background); - old version*/
		for (int i = 0; i < n; i++) {
			if (pthread_kill(threads[i], SIGUSR1) != 0) {
				perror("Niepoprawne wyslanie sygnalu do watku!");
			}
		}

		tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}

int invalid_parameter(char* param) {
	char *temp;
    int no_threads = (int)strtol(param, &temp, 10);

    if (!strcmp(temp, "\0") && no_threads >= 1 && no_threads <= GRID_WIDTH * GRID_HEIGHT) {
        return 0;
    }
    return 1;
}

void* thread_routine(void* arg) {
	int scs = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	int sct = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	if (scs != 0 || sct != 0) {
		perror("Blad w ustawianiu reakcji na przerwanie watku!");
		pthread_exit(NULL);
	}

	args arguments = *(args*)arg;
	int begin = arguments.begin;
	int end = arguments.end;
	char *fore = arguments.foreground;
	char *back = arguments.background;
	char *tmp;

	while (true) {
		update_grid(fore, back, begin, end);
		pause();

		tmp = fore;
		fore = back;
		back = tmp;
	}

	return NULL;
}
